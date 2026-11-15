import fs from "node:fs"
import path from "node:path"

const root = path.resolve(import.meta.dirname, "../..")
const circuitJsonPath = path.join(
  root,
  "fabrication",
  "movement-prototype.circuit.json",
)
const gerberDirectory = path.join(
  root,
  "fabrication",
  "movement-prototype-gerbers",
)

const fail = (message) => {
  console.error(`fabrication validation failed: ${message}`)
  process.exit(1)
}

if (!fs.existsSync(circuitJsonPath)) {
  fail(`missing ${circuitJsonPath}`)
}

const circuit = JSON.parse(fs.readFileSync(circuitJsonPath, "utf8"))
if (!Array.isArray(circuit)) fail("circuit JSON is not an array")

const errorElements = circuit.filter((element) =>
  String(element?.type ?? "").includes("error"),
)
if (errorElements.length > 0) {
  console.error(JSON.stringify(errorElements, null, 2))
  fail(`${errorElements.length} circuit error element(s) were emitted`)
}

const sourceComponents = circuit.filter(
  (element) => element?.type === "source_component",
)
const componentNames = sourceComponents.map((element) => element.name)

for (let index = 1; index <= 12; index += 1) {
  if (!componentNames.includes(`K${index}`)) fail(`K${index} is missing`)
  if (!componentNames.includes(`D${index}`)) fail(`D${index} is missing`)
}

if (!componentNames.includes("J_USB")) {
  fail("the USB-C connector J_USB is missing")
}
if (!componentNames.includes("U1")) {
  fail("the RP2040 U1 is missing")
}

const sourceTraces = circuit.filter((element) => element?.type === "source_trace")
const pcbTraces = circuit.filter((element) => element?.type === "pcb_trace")
if (sourceTraces.length < 30) {
  fail(`only ${sourceTraces.length} source traces were emitted`)
}
if (pcbTraces.length < 30) {
  fail(`only ${pcbTraces.length} routed PCB traces were emitted`)
}

if (!fs.existsSync(gerberDirectory)) {
  fail(`missing ${gerberDirectory}`)
}

const files = fs.readdirSync(gerberDirectory)
const gerbers = files.filter((file) => file.endsWith(".gbr"))
const drills = files.filter((file) => file.endsWith(".drl"))

if (gerbers.length < 5) {
  fail(`only ${gerbers.length} Gerber layers were emitted`)
}
if (drills.length < 1) {
  fail("no Excellon drill file was emitted")
}

for (const file of [...gerbers, ...drills]) {
  const fullPath = path.join(gerberDirectory, file)
  if (fs.statSync(fullPath).size < 100) fail(`${file} is implausibly small`)
}

console.log(
  `validated ${sourceComponents.length} components, ${sourceTraces.length} source traces, ` +
    `${pcbTraces.length} PCB traces, ${gerbers.length} Gerber layers, and ${drills.length} drill file(s)`,
)
