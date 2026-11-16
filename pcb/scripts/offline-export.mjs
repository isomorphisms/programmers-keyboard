// Imported before the replay configuration is constructed, including in the
// CLI worker. A caught configuration error must never enable live fallback.
globalThis.fetch = async function() {
  throw new Error("supplier-material: online fetch forbidden during offline export")
}
