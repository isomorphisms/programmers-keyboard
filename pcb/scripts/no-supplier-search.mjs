// Capture may read public catalogue IDs. It may never search using a design.
const originalFetch = globalThis.fetch
globalThis.fetch = function(input, ...rest) {
  const url = new URL(typeof input === "string" || input instanceof URL ? input : input.url)
  if (url.hostname === "jlcsearch.tscircuit.com" || url.hostname === "digikeysearch.tscircuit.com")
    throw new Error("supplier-material: live component search is disabled")
  return originalFetch.call(this, input, ...rest)
}
