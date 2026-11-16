import fs from "node:fs"
import path from "node:path"
import assert from "node:assert/strict"
import configuration from "../tscircuit.config.js"
import { canonical } from "./supplier-material.mjs"

const directory = path.resolve(import.meta.dirname, "../supplier-material")
const engine = configuration.platformConfig.partsEngine
const lookups = JSON.parse(fs.readFileSync(path.join(directory, "lookups.json"), "utf8"))
const parts = JSON.parse(fs.readFileSync(path.join(directory, "parts.json"), "utf8"))
for (const record of lookups) assert.equal(canonical(engine.findPart(record.request)), canonical(record.response.value))
for (const record of parts) {
  if (record.response.kind === "error") assert.throws(() => engine.fetchPartCircuitJson(record.request), error => error.message === record.response.message)
  else assert.equal(canonical(engine.fetchPartCircuitJson(record.request)), canonical(record.response.value ?? undefined))
}
console.log("supplier-material\tPASS\t" + lookups.length + " lookup requests; " + parts.length + " supplier responses; offline")
