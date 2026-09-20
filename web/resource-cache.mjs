// Dedicated resource cache. Never opens, clears or migrates the save database.
const DB_NAME = 'pvz.pages.resource-cache.v1';
const STORE = 'bundles';
export function cacheResource(mode, key, bytes, factory = globalThis.indexedDB, timeoutMs = 10000) {
  if (!factory) return Promise.resolve(null);
  return new Promise(resolve => {
    let db, transaction, done = false, value = null;
    const finish = result => {
      if (done) return;
      done = true;
      clearTimeout(timer);
      db?.close();
      resolve(result);
    };
    const timer = setTimeout(() => {
      try { transaction?.abort(); } catch {}
      finish(null);
    }, timeoutMs);
    try {
      const request = factory.open(DB_NAME, 1);
      request.onupgradeneeded = () => request.result.createObjectStore(STORE);
      request.onerror = request.onblocked = () => finish(null);
      request.onsuccess = () => {
        db = request.result;
        if (done) { db.close(); return; }
        db.onversionchange = () => { db.close(); finish(null); };
        try {
          transaction = db.transaction(STORE, mode === 'put' ? 'readwrite' : 'readonly');
          transaction.oncomplete = () => finish(mode === 'put' ? true : value);
          transaction.onerror = transaction.onabort = () => finish(null);
          const store = transaction.objectStore(STORE);
          if (mode === 'put') {
            // Only the validated current resource pack is retained. Atomic with put.
            store.clear();
            store.put(bytes, key);
          } else {
            const read = store.get(key);
            read.onsuccess = () => { value = read.result ?? null; };
          }
        } catch { finish(null); }
      };
    } catch { finish(null); }
  });
}
