// One-time material capture adapter; never imported by the offline producer.
import fs from "node:fs"
import path from "node:path"
import { canonical, digest, lookupRequest, partRequest, verifySource } from "../../../pcb/scripts/supplier-material.mjs"

const pcb = path.resolve(import.meta.dirname, "../../../pcb")
verifySource(pcb)
const baseline = JSON.parse(fs.readFileSync(path.join(pcb, "../fabrication/movement-prototype.circuit.json"), "utf8"))
if (digest(fs.readFileSync(path.join(pcb, "../fabrication/movement-prototype.circuit.json"))) !== "f31d49c3e518faee75348e208e916ce11eac66178bde01932395fdb658a2a5cb")
  throw new Error("capture requires unchanged recorded baseline")
const components = new Map(baseline.filter(row => row.type === "source_component").map(row => [row.source_component_id, row]))
let upstream
const lookups = new Map(), parts = new Map()
const directory = path.join(pcb, "supplier-material")
fs.mkdirSync(directory, { recursive: true })
const save = () => {
  for (const [name, records] of [["lookups.json", lookups], ["parts.json", parts]]) {
    fs.writeFileSync(path.join(directory, name), JSON.stringify([...records.values()].sort((a,b) => canonical(a.request).localeCompare(canonical(b.request))), null, 2) + "\n")
  }
  const lines = ["lookups.json", "parts.json"].map(name => digest(fs.readFileSync(path.join(directory, name))) + "\t" + name)
  fs.writeFileSync(path.join(directory, "sha256.tsv"), lines.join("\n") + "\n")
}
export const captureEngine = {
  findPart(params) {
    const recorded = components.get(params.sourceComponent.source_component_id)
    if (!recorded || canonical(lookupRequest({ sourceComponent: recorded }).sourceComponent) !== canonical(lookupRequest(params).sourceComponent))
      throw new Error("capture source descriptor mismatch: " + canonical(params))
    const response = { kind: "candidates", value: recorded.supplier_part_numbers ?? {} }
    const request = lookupRequest(params)
    lookups.set(canonical(request), { request, response })
    save()
    return structuredClone(response.value)
  },
  async fetchPartCircuitJson(params) {
    const request = partRequest(params)
    if (!/^C[0-9]+$/.test(request.supplierPartNumber ?? "") || request.manufacturerPartNumber !== undefined)
      throw new Error("capture permits only a public supplier product number")
    let response
    try {
      // Send only a public catalogue number, never component/project details.
      const value = await upstream.fetchPartCircuitJson({ supplierPartNumber: request.supplierPartNumber })
      response = { kind: "part", value: value ?? null }
    } catch (error) {
      response = { kind: "error", message: error.message }
    }
    const prior = parts.get(canonical(request))
    if (prior && canonical(prior.response) !== canonical(response)) throw new Error("supplier response changed during capture")
    parts.set(canonical(request), { request, response })
    save()
    if (response.kind === "error") throw new Error(response.message)
    return response.value === null ? undefined : structuredClone(response.value)
  },
}
export function createCaptureEngine(engine) {
  upstream = engine
  return captureEngine
}
