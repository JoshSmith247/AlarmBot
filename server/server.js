import { createServer } from "node:http";
import { readFileSync, writeFileSync, existsSync, readdirSync } from "node:fs";

const KEY = process.env.KEY || "dev";
const FILE = process.env.STATE_FILE || new URL("./state.json", import.meta.url).pathname;
const TYPES = { html: "text/html", webmanifest: "application/manifest+json", js: "text/javascript", png: "image/png" };
const files = new Map(
    readdirSync(new URL("./public/", import.meta.url)).map((f) => [f === "index.html" ? "/" : "/" + f, [readFileSync(new URL("./public/" + f, import.meta.url)), TYPES[f.split(".").pop()]]]),
);

const state = { alarm: "07:30", enabled: false, ringing: false, events: [], seen: 0, ...(existsSync(FILE) && JSON.parse(readFileSync(FILE))) };
let queue = [];

const save = () => writeFileSync(FILE, JSON.stringify(state));
const stamp = () => new Date().toISOString();

function command(line) {
    const set = /^set (\d{1,2}):(\d{2})$/.exec(line);
    if (set && +set[1] < 24 && +set[2] < 60) {
        state.alarm = `${set[1].padStart(2, "0")}:${set[2]}`;
        state.enabled = true;
    } else if (line === "off") {
        state.enabled = false;
    } else if (line !== "ring" && line !== "stop") {
        return false;
    }
    queue.push(line);
    save();
    return true;
}

function body(req) {
    return new Promise((resolve) => {
        let s = "";
        req.on("data", (c) => (s += c));
        req.on("end", () => resolve(s.trim()));
    });
}

const server = createServer(async (req, res) => {
    const url = new URL(req.url, "http://x");
    const send = (code, text, type = "text/plain") => {
        res.writeHead(code, { "Content-Type": type, "Content-Length": Buffer.byteLength(text) });
        res.end(text);
    };

    if (files.has(url.pathname)) return send(200, ...files.get(url.pathname));
    if ((url.searchParams.get("key") || req.headers["x-key"]) !== KEY) return send(401, "bad key\n");

    if (url.pathname === "/poll") {
        if (url.searchParams.get("boot") === "1") {
            queue = queue.filter((c) => !c.startsWith("set") && c !== "off");
            queue.unshift(state.enabled ? `set ${state.alarm}` : "off");
        }
        for (const e of (url.searchParams.get("say") || "").split(",").filter(Boolean)) {
            const text = e.replaceAll("_", " ");
            state.events.push({ t: stamp(), text });
            if (text === "ringing") state.ringing = true;
            if (text === "stopped" || text === "caught") state.ringing = false;
        }
        state.events = state.events.slice(-20);
        state.seen = Date.now();
        save();
        const out = queue.map((c) => c + "\n").join("");
        queue = [];
        return send(200, out);
    }
    if (url.pathname === "/api/state") return send(200, JSON.stringify({ ...state, online: Date.now() - state.seen < 10000 }), "application/json");
    if (url.pathname === "/api/cmd" && req.method === "POST")
        return command(await body(req)) ? send(200, "ok\n") : send(400, "unknown command\n");
    send(404, "not found\n");
});

if (process.argv[1] === new URL(import.meta.url).pathname) {
    const port = process.env.PORT || 3000;
    server.listen(port, () => console.log(`alarmbot server on :${port}, key "${KEY}"`));
}
export { server };
