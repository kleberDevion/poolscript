// cliente gerado por swagger (typescript / react_router)
const BASE = "";

async function _req(method, path, body) {
  const r = await fetch(BASE + path, {
    method,
    headers: body !== undefined ? { "Content-Type": "application/json" } : {},
    body: body !== undefined ? JSON.stringify(body) : undefined,
  });
  return r.json();
}


export const routes = {
};
