import fs from "node:fs"
import path from "node:path"
import { createHash } from "node:crypto"

export const boardHash = "2a59a798a87028f27429991902ba0601f7f8f60993536cec29af8884a5e8eaf5"
export const lockHash = "82815f4feb468e3fbb35ad67ca7bbfe7c275a5402bce77291d1fcb5953bce114"
const stable = value => Array.isArray(value) ? value.map(stable)
  : value && typeof value === "object" ? Object.fromEntries(Object.keys(value).sort().map(key => [key, stable(value[key])])) : value
export const canonical = value => JSON.stringify(stable(value))
const requireThat = (condition, message) => { if (!condition) throw new Error("supplier-material: " + message) }
export const digest = bytes => createHash("sha256").update(bytes).digest("hex")
export function lookupRequest({ sourceComponent, footprinterString }) {
  const descriptor = { ...sourceComponent }
  delete descriptor.supplier_part_numbers
  return { sourceComponent: descriptor, ...(footprinterString === undefined ? {} : { footprinterString }) }
}
export function partRequest({ supplierPartNumber, manufacturerPartNumber }) {
  return { ...(supplierPartNumber === undefined ? {} : { supplierPartNumber }),
    ...(manufacturerPartNumber === undefined ? {} : { manufacturerPartNumber }) }
}
export function verifySource(pcb) {
  for (const [name, expected] of [["movement-prototype.tsx", boardHash], ["package-lock.json", lockHash]]) {
    requireThat(digest(fs.readFileSync(path.join(pcb, name))) === expected, "changed source input " + name)
  }
}
function loadRecords(directory, name, expectedDigest) {
  const filename = path.join(directory, name)
  requireThat(fs.lstatSync(filename).isFile(), "not a regular material file " + name)
  const bytes = fs.readFileSync(filename)
  requireThat(bytes.length <= 8 * 1024 * 1024, "oversized material " + name)
  requireThat(digest(bytes) === expectedDigest, "digest mismatch " + name)
  const records = JSON.parse(bytes)
  requireThat(Array.isArray(records), "invalid record array " + name)
  const map = new Map()
  for (const record of records) {
    requireThat(record && record.request && record.response, "invalid record " + name)
    const key = canonical(record.request)
    requireThat(!map.has(key), "duplicate request " + name)
    map.set(key, record.response)
  }
  return map
}
export function createSupplierReplay(pcb) {
  verifySource(pcb)
  const directory = path.join(pcb, "supplier-material")
  const lines = fs.readFileSync(path.join(directory, "sha256.tsv"), "utf8").trim().split("\n").map(line => line.split("\t"))
  requireThat(lines.length === 2 && lines.every(row => row.length === 2 && /^[a-f0-9]{64}$/.test(row[0])), "invalid material digest manifest")
  const manifest = new Map(lines.map(([hash, name]) => [name, hash]))
  requireThat(manifest.size === 2 && manifest.has("lookups.json") && manifest.has("parts.json"), "invalid material members")
  const lookups = loadRecords(directory, "lookups.json", manifest.get("lookups.json"))
  const parts = loadRecords(directory, "parts.json", manifest.get("parts.json"))
  return {
    findPart(params) {
      const response = lookups.get(canonical(lookupRequest(params)))
      requireThat(response, "unknown lookup for " + params.sourceComponent?.name)
      requireThat(response.kind === "candidates" && response.value && typeof response.value === "object" && !Array.isArray(response.value), "invalid candidate response")
      requireThat(Object.entries(response.value).every(([supplier, candidates]) =>
        ["jlcpcb", "macrofab", "pcbway", "digikey", "mouser", "lcsc"].includes(supplier) &&
        Array.isArray(candidates) && candidates.every(candidate => typeof candidate === "string" && candidate.length > 0)), "invalid supplier candidates")
      return structuredClone(response.value)
    },
    fetchPartCircuitJson(params) {
      const response = parts.get(canonical(partRequest(params)))
      requireThat(response, "unknown supplier part " + canonical(partRequest(params)))
      if (response.kind === "error") {
        requireThat(typeof response.message === "string", "invalid recorded error")
        throw new Error(response.message)
      }
      requireThat(response.kind === "part" && (response.value === null || (Array.isArray(response.value) && response.value.every(row => row && typeof row.type === "string"))), "invalid part response")
      return response.value === null ? undefined : structuredClone(response.value)
    },
  }
}
