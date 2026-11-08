import assert from "node:assert/strict";
import { tmpdir } from "node:os";

process.env.STATE_FILE = `${tmpdir()}/alarmbot-test-${process.pid}.json`;
const { server } = await import("./server.js");
await new Promise((r) => server.listen(0, r));
const base = `http://127.0.0.1:${server.address().port}`;
const get = (p) => fetch(`${base}${p}`).then(async (r) => [r.status, await r.text()]);
const post = (body) => fetch(`${base}/api/cmd?key=dev`, { method: "POST", body }).then((r) => r.status);

assert.equal((await get("/poll"))[0], 401);
for (const [path, type] of [["/", "text/html"], ["/manifest.webmanifest", "application/manifest+json"], ["/sw.js", "text/javascript"], ["/icon-512.png", "image/png"]]) {
    const r = await fetch(base + path);
    assert.equal(r.status, 200, path);
    assert.equal(r.headers.get("content-type"), type, path);
}
assert.equal((await fetch(base + "/api/state")).status, 401);
assert.equal(await post("set 7:05"), 200);
assert.equal(await post("bogus"), 400);
assert.equal(await post("set 25:00"), 400);
assert.deepEqual(await get("/poll?key=dev"), [200, "set 7:05\n"]);
assert.deepEqual(await get("/poll?key=dev"), [200, ""]);

await post("ring");
assert.deepEqual(await get("/poll?key=dev&say=ringing"), [200, "ring\n"]);
let s = JSON.parse((await get("/api/state?key=dev"))[1]);
assert.equal(s.ringing, true);
assert.equal(s.alarm, "07:05");
assert.equal(s.online, true);
await get("/poll?key=dev&say=stopped,alarm_set");
s = JSON.parse((await get("/api/state?key=dev"))[1]);
assert.equal(s.ringing, false);

assert.deepEqual(await get("/poll?key=dev&boot=1"), [200, "set 07:05\n"]);
await post("off");
assert.deepEqual(await get("/poll?key=dev&boot=1"), [200, "off\n"]);

console.log("ok");
server.close();
