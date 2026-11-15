import "./scripts/offline-export.mjs"
import { createSupplierReplay } from "./scripts/supplier-material.mjs"
import path from "node:path"
import { fileURLToPath } from "node:url"

const pcb = path.dirname(fileURLToPath(import.meta.url))
export default { platformConfig: { partsEngine: createSupplierReplay(pcb), localCacheEngine: undefined } }
