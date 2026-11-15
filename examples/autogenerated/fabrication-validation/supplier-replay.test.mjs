import test from "node:test"
import assert from "node:assert/strict"
import fs from "node:fs"
import os from "node:os"
import path from "node:path"
import { spawnSync } from "node:child_process"
import { createSupplierReplay, digest, canonical } from "../../../pcb/scripts/supplier-material.mjs"

const pcb = path.resolve(import.meta.dirname, "../../../pcb")
const directory = path.join(pcb, "supplier-material")
const lookups = JSON.parse(fs.readFileSync(path.join(directory, "lookups.json"), "utf8"))
const parts = JSON.parse(fs.readFileSync(path.join(directory, "parts.json"), "utf8"))
const scratch = fs.mkdtempSync(path.join(os.tmpdir(), "pkb-supplier-"))
let sequence = 0, networkCalls = 0
globalThis.fetch = async () => { ++networkCalls; throw new Error("online fallback attempted") }
test.after(() => { assert.equal(networkCalls, 0); fs.rmSync(scratch, { recursive: true, force: true }) })
const fixture = () => {
  const root = path.join(scratch, String(++sequence)); fs.mkdirSync(root)
  for (const name of ["movement-prototype.tsx", "package-lock.json"]) fs.copyFileSync(path.join(pcb, name), path.join(root, name))
  fs.cpSync(directory, path.join(root, "supplier-material"), { recursive: true })
  return root
}
const editRecords = (root, name, edit, updateHash = false) => {
  const location = path.join(root, "supplier-material", name)
  const rows = JSON.parse(fs.readFileSync(location, "utf8")); edit(rows)
  fs.writeFileSync(location, JSON.stringify(rows))
  if (updateHash) {
    const filename = path.join(root, "supplier-material/sha256.tsv")
    const lines = fs.readFileSync(filename, "utf8").trim().split("\n").map(line => line.endsWith("\t" + name) ? digest(fs.readFileSync(location)) + "\t" + name : line)
    fs.writeFileSync(filename, lines.join("\n") + "\n")
  }
}
test("the actual export preload refuses network access; missing preload is detected", () => {
  const code = 'try { await fetch("http://127.0.0.1:9"); process.exit(3) } catch(error) { if(error.message!=="supplier-material: online fetch forbidden during offline export") { console.error("guard missing"); process.exit(2) } console.log("PASS offline guard") }'
  const good = spawnSync(process.execPath, ["--import", path.join(pcb, "scripts/offline-export.mjs"), "--input-type=module", "-e", code], { encoding: "utf8", timeout: 5000 })
  assert.ifError(good.error); assert.equal(good.status, 0, good.stderr); assert.match(good.stdout, /PASS offline guard/)
  const bad = spawnSync(process.execPath, ["--input-type=module", "-e", code], { encoding: "utf8", timeout: 5000 })
  assert.ifError(bad.error); assert.equal(bad.status, 2); assert.match(bad.stderr, /guard missing/)
})
test("all recorded queries and supplier responses replay without network", () => {
  const engine = createSupplierReplay(pcb)
  assert.equal(lookups.length, 52); assert.equal(parts.length, 18)
  for (const record of lookups) assert.equal(canonical(engine.findPart(record.request)), canonical(record.response.value))
  for (const record of parts) {
    if (record.response.kind === "error") assert.throws(() => engine.fetchPartCircuitJson(record.request), error => error.message === record.response.message)
    else assert.equal(canonical(engine.fetchPartCircuitJson(record.request)), canonical(record.response.value ?? undefined))
  }
})
test("original resistor alternatives retain order and return fresh objects", () => {
  const engine = createSupplierReplay(pcb)
  const request = lookups.find(row => row.request.sourceComponent.name === "R_3V3_EN").request
  const result = engine.findPart(request)
  assert.deepEqual(result.jlcpcb, ["C25741", "C60491", "C2906859"])
  result.jlcpcb[1] = "MUTATED"
  assert.equal(engine.findPart(request).jlcpcb[1], "C60491")
})
test("same-name MCU D1 cannot alias matrix D1", () => {
  const engine = createSupplierReplay(pcb)
  const request = structuredClone(lookups.find(row => row.request.sourceComponent.name === "D1").request)
  engine.findPart(request)
  request.sourceComponent.source_component_id = "source_component_19"
  request.sourceComponent.source_group_id = "source_group_0"
  assert.throws(() => engine.findPart(request), /unknown lookup/)
})
for (const field of ["name", "resistance", "ftype", "source_group_id"]) test("changed component descriptor " + field, () => {
  const engine = createSupplierReplay(pcb)
  const request = structuredClone(lookups.find(row => row.request.sourceComponent.name === "R_3V3_EN").request)
  request.sourceComponent[field] = "changed"
  assert.throws(() => engine.findPart(request), /unknown lookup/)
})
test("changed literal footprint cannot inherit a lookup", () => {
  const request = structuredClone(lookups[0].request); request.footprinterString = "different-footprint"
  assert.throws(() => createSupplierReplay(pcb).findPart(request), /unknown lookup/)
})
test("unknown supplier part cannot fall back online", () => {
  assert.throws(() => createSupplierReplay(pcb).fetchPartCircuitJson({ supplierPartNumber: "C0000000000" }), /unknown supplier part/)
})
test("manufacturer search cannot fall back online", () => {
  assert.throws(() => createSupplierReplay(pcb).fetchPartCircuitJson({ manufacturerPartNumber: "unrecorded" }), /unknown supplier part/)
})
for (const name of ["lookups.json", "parts.json"]) test("missing material " + name, () => {
  const root = fixture(); fs.unlinkSync(path.join(root, "supplier-material", name))
  assert.throws(() => createSupplierReplay(root), /ENOENT/)
})
test("modified payload fails its recorded digest", () => {
  const root = fixture()
  editRecords(root, "lookups.json", rows => { rows[0].response.value = {} })
  assert.throws(() => createSupplierReplay(root), /digest mismatch/)
})
test("duplicate request is rejected even with a matching payload digest", () => {
  const root = fixture()
  editRecords(root, "lookups.json", rows => rows.push(rows[0]), true)
  assert.throws(() => createSupplierReplay(root), /duplicate request/)
})
test("missing request remains a refusal after a manifest update", () => {
  const root = fixture(), request = lookups[0].request
  editRecords(root, "lookups.json", rows => rows.shift(), true)
  assert.throws(() => createSupplierReplay(root).findPart(request), /unknown lookup/)
})
test("malformed manifest fails before lookup", () => {
  const root = fixture(); fs.writeFileSync(path.join(root, "supplier-material/sha256.tsv"), "garbage")
  assert.throws(() => createSupplierReplay(root), /invalid material digest manifest/)
})
test("invalid candidate shape cannot masquerade as supplier data", () => {
  const root = fixture()
  editRecords(root, "lookups.json", rows => { rows[0].response.value = "not-an-object" }, true)
  assert.throws(() => createSupplierReplay(root).findPart(lookups[0].request), /invalid candidate response/)
})
test("invalid supplier footprint shape is rejected", () => {
  const root = fixture()
  editRecords(root, "parts.json", rows => { rows[0].response = {kind:"part",value:[42]} }, true)
  assert.throws(() => createSupplierReplay(root).fetchPartCircuitJson(parts[0].request), /invalid part response/)
})
test("supplier footprint responses are fresh copies", () => {
  const engine = createSupplierReplay(pcb)
  const record = parts.find(row => row.response.kind === "part" && row.response.value?.length)
  const first = engine.fetchPartCircuitJson(record.request)
  first[0].type = "changed-by-caller"
  assert.equal(canonical(engine.fetchPartCircuitJson(record.request)), canonical(record.response.value))
})
for (const name of ["movement-prototype.tsx", "package-lock.json"]) test("changed bound input " + name, () => {
  const root = fixture(); fs.appendFileSync(path.join(root, name), "\nchanged\n")
  assert.throws(() => createSupplierReplay(root), /changed source input/)
})
