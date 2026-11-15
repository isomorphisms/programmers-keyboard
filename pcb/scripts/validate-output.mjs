import fs from "node:fs"
import path from "node:path"
import { spawnSync } from "node:child_process"

const repository = path.resolve(import.meta.dirname, "../..")
const required = ["B_Cu.gbr", "B_Mask.gbr", "B_Paste.gbr", "B_SilkScreen.gbr",
  "Edge_Cuts.gbr", "F_Cu.gbr", "F_Fab.gbr", "F_Mask.gbr", "F_Paste.gbr",
  "F_SilkScreen.gbr", "bom.csv", "drill.drl", "drill_npth.drl", "pick_and_place.csv"]
const check = (condition, stage, message) => { if (!condition) throw new Error(stage + ": " + message) }
const regular = filename => {
  check(fs.existsSync(filename), "inventory", "missing " + filename)
  check(fs.lstatSync(filename).isFile(), "inventory", "not a regular file: " + filename)
}
const exactMembers = (members, stage) => {
  check(new Set(members).size === members.length, stage, "duplicate package member")
  for (const member of members) check(required.includes(member), stage, "unexpected member " + member)
  for (const member of required) check(members.includes(member), stage, "missing member " + member)
}
const unzip = (archive, flags) => {
  const result = spawnSync("unzip", [...flags, archive], { timeout: 10000, maxBuffer: 16 * 1024 * 1024 })
  check(!result.error && result.status === 0, "archive", "unzip failed: " + (result.error?.message ?? result.stderr?.toString()))
  return result.stdout
}

// Inspect names and Unix modes before permitting extraction.
function validateArchive(root, compareExtracted) {
  const archive = path.join(root, "fabrication/movement-prototype-gerbers.zip")
  regular(archive)
  check(fs.statSync(archive).size <= 32 * 1024 * 1024, "archive", "archive exceeds 32 MiB")
  const members = unzip(archive, ["-Z1"]).toString("utf8").trimEnd().split("\n")
  exactMembers(members, "archive")
  const listing = unzip(archive, ["-Z", "-l"]).toString("utf8").split("\n")
  const entries = listing.filter(line => /^[-dl?bcps]/.test(line))
  check(entries.length === required.length, "archive", "unrecognized member attributes")
  for (const entry of entries) {
    check(entry.startsWith("-"), "archive", "symlink or special archive member")
    const fields = entry.trim().split(/\s+/)
    check(Number.isSafeInteger(Number(fields[3])) && Number(fields[3]) <= 16 * 1024 * 1024,
      "archive", "invalid or oversized uncompressed member")
  }
  unzip(archive, ["-tqq"])
  if (compareExtracted) for (const member of required) {
    const result = spawnSync("unzip", ["-p", archive, member], { timeout: 10000, maxBuffer: 16 * 1024 * 1024 })
    check(!result.error && result.status === 0, "archive", "cannot read " + member)
    check(result.stdout.equals(fs.readFileSync(path.join(root, "fabrication/movement-prototype-gerbers", member))),
      "archive-content", "ZIP differs from extracted " + member)
  }
}

function validateCircuit(root) {
  const filename = path.join(root, "fabrication/movement-prototype.circuit.json")
  regular(filename)
  let circuit
  try { circuit = JSON.parse(fs.readFileSync(filename, "utf8")) } catch { check(false, "circuit", "invalid JSON") }
  check(Array.isArray(circuit) && circuit.every(item => item && typeof item.type === "string"), "circuit", "invalid element array")
  check(!circuit.some(item => item.type.includes("error")), "circuit", "circuit error element was emitted")
  const components = circuit.filter(item => item.type === "source_component")
  check(components.every(item => typeof item.source_component_id === "string" && item.source_component_id.length > 0), "components", "missing source component identity")
  check(new Set(components.map(item => item.source_component_id)).size === components.length, "components", "duplicate source component identity")
  for (let index = 1; index <= 12; ++index) {
    const switches = components.filter(item => item.name === "K" + index)
    check(switches.length === 1, "components", "K" + index + " is missing or ambiguous")
    check(typeof switches[0].source_group_id === "string" && circuit.filter(item => item.type === "source_group" && item.source_group_id === switches[0].source_group_id).length === 1,
      "components", "K" + index + " has missing or ambiguous scope")
    const diodes = components.filter(item => item.name === "D" + index && item.source_group_id === switches[0].source_group_id)
    check(diodes.length === 1 && diodes[0].ftype === "simple_diode", "components", "matrix D" + index + " is missing or ambiguous in K" + index + " scope")
  }
  for (const name of ["U1", "J_USB"]) check(components.filter(item => item.name === name).length === 1, "components", name + " is missing or ambiguous")
  for (const kind of ["source_trace", "pcb_trace"]) check(circuit.filter(item => item.type === kind).length >= 30, "circuit", "too few " + kind + " elements")
  return { circuit, warnings: circuit.filter(item => item.type.includes("warning")), components: components.length }
}

// Envelope checks only. PKB-A2 owns full Gerber/Excellon syntax/geometry.
function validateExtracted(root) {
  const fabrication = path.join(root, "fabrication")
  check(fs.existsSync(fabrication) && fs.lstatSync(fabrication).isDirectory(), "inventory", "missing or symlinked fabrication directory")
  const directory = path.join(root, "fabrication/movement-prototype-gerbers")
  check(fs.existsSync(directory) && fs.lstatSync(directory).isDirectory(), "inventory", "missing or symlinked package directory")
  exactMembers(fs.readdirSync(directory), "inventory")
  for (const member of required) {
    const filename = path.join(directory, member)
    regular(filename)
    check(fs.statSync(filename).size >= 100, "framing", "implausibly small " + member)
    const contents = fs.readFileSync(filename, "utf8")
    if (member.endsWith(".gbr")) check(contents.includes("%MOMM*%") && contents.trimEnd().endsWith("M02*"), "framing", "invalid Gerber envelope: " + member)
    if (member.endsWith(".drl")) check(contents.startsWith("M48\n") && /(?:^|\n)METRIC(?:,|\n)/.test(contents) && contents.trimEnd().endsWith("M30"), "framing", "invalid drill envelope: " + member)
  }
}
const normalize = (member, bytes) => /\.(gbr|drl)$/.test(member)
  ? bytes.toString("utf8").split("\n").filter(line => !/^(%TF\.CreationDate,|G04 Created by tscircuit \(builder\) date |; DRILL file \{tscircuit\} date |; #@! TF\.CreationDate,)/.test(line)).join("\n")
  : bytes
const stableJson = value => Array.isArray(value) ? value.map(stableJson)
  : value && typeof value === "object" ? Object.fromEntries(Object.keys(value).sort().map(key => [key, stableJson(value[key])])) : value
function compareRoots(root, previous) {
  validateExtracted(previous); validateArchive(previous, true)
  // No Circuit JSON fields are ignored; only object-key ordering is immaterial.
  check(JSON.stringify(stableJson(validateCircuit(previous).circuit)) === JSON.stringify(stableJson(validateCircuit(root).circuit)), "comparison", "Circuit JSON changed")
  for (const member of required) {
    const relative = path.join("fabrication/movement-prototype-gerbers", member)
    check(Buffer.from(normalize(member, fs.readFileSync(path.join(previous, relative)))).equals(Buffer.from(normalize(member, fs.readFileSync(path.join(root, relative))))), "comparison", "changed " + member)
  }
  for (const name of ["movement-prototype-pcb.svg", "movement-prototype-schematic.svg", "movement-prototype-readable.netlist"]) {
    const relative = path.join("fabrication", name)
    regular(path.join(root, relative)); regular(path.join(previous, relative))
    check(fs.readFileSync(path.join(root, relative)).equals(fs.readFileSync(path.join(previous, relative))), "comparison", "changed " + name)
  }
}
try {
  let root = repository, previous, archiveOnly = false
  const args = process.argv.slice(2)
  while (args.length) {
    const option = args.shift()
    if (option === "--root" || option === "--compare-root") {
      check(args.length > 0 && !args[0].startsWith("--"), "arguments", "missing value for " + option)
      if (option === "--root") root = path.resolve(args.shift())
      else previous = path.resolve(args.shift())
    } else if (option === "--archive-only") archiveOnly = true
    else check(false, "arguments", "unknown option " + option)
  }
  if (archiveOnly) {
    check(!previous, "arguments", "--archive-only cannot compare roots")
    validateArchive(root, false)
  } else {
    validateExtracted(root); validateArchive(root, true)
    const result = validateCircuit(root)
    if (previous) compareRoots(root, previous)
    console.log("package-structure\tPASS\t14 members; " + result.components + " components")
    console.log("circuit-warnings\t" + (result.warnings.length ? "NOT_VERIFIED" : "NONE_EMITTED") + "\t" + result.warnings.length)
    console.log("syntax-and-geometry\tNOT_VERIFIED\tPKB-A2/A4 required")
    console.log("assembly-identities\tNOT_VERIFIED\tPKB-A7 required")
    console.log("electrical-and-manufacturing\tNOT_VERIFIED\tindependent review required")
  }
} catch (error) {
  console.error("fabrication validation failed: " + error.message)
  process.exitCode = 1
}
