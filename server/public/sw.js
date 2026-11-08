const CACHE = "alarmbot-v1";

self.addEventListener("install", (e) => e.waitUntil(caches.open(CACHE).then((c) => c.addAll(["/", "/icon-192.png"]))));

self.addEventListener("fetch", (e) => {
    if (new URL(e.request.url).pathname.startsWith("/api")) return;
    e.respondWith(fetch(e.request).catch(() => caches.match(e.request)));
});
