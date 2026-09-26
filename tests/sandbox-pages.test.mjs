// Runtime / state-machine checks; these are not browser gameplay tests.
import test from 'node:test';
import assert from 'node:assert/strict';
import { readFile } from 'node:fs/promises';
import { createHash } from 'node:crypto';
import { createRequire } from 'node:module';
import { fileURLToPath } from 'node:url';
import vm from 'node:vm';
import { cacheResource } from '../web/resource-cache.mjs';
import { verifyResourceBytes } from '../web/resource-import.mjs';
import { validateManifest, sha256 } from '../web/resource-utils.mjs';
import { TECH_PLANTS, validateLayout } from '../web/sandbox-data.mjs';
const read = name => readFile(new URL('../' + name, import.meta.url));

test('default page uses integrated sandbox and project-relative paths, not loopback resources', async () => {
  const html = (await read('site/index.html')).toString();
  assert.match(html, /sandbox-engine\/pvz-portable.js/);
  assert.doesNotMatch(html, /id="(?:choose-resource|resource-file|resource-picker)"/);
  assert.match(html, /游戏自动加载/);
  const bootstrap = (await read('site/bootstrap.js')).toString();
  assert.match(bootstrap, /noInitialRun: true/);
  assert.match(bootstrap, /new URL\('sandbox-engine\//);
  const runtime = (await read('site/runtime.mjs')).toString();
  assert.match(runtime, /fetch\('resource-manifest.json'/);
  assert.match(runtime, /loadResourceBundle\(manifest, setStatus\)/);
  assert.doesNotMatch(runtime, /importResourceBundle|choose-resources/);
  for (const name of ['bootstrap.js', 'runtime.mjs', 'resource-import.mjs', 'resource-cache.mjs', 'game.css']) {
    const code = (await read('site/' + name)).toString();
    assert.doesNotMatch(code, /https?:\/\/127\.0\.0\.1|https?:\/\/localhost|url\('title.jpg'\)/);
  }
  assert.doesNotMatch((await read('site/resource-import.mjs')).toString(), /\bfetch\s*\(|XMLHttpRequest|sendBeacon/);
  const manifest = validateManifest(JSON.parse(await read('site/resource-manifest.json')));
  assert.equal(manifest.totalFiles, manifest.files.length);
});

test('published sandbox engine matches the recorded build and really initializes its exported API', async () => {
  const info = JSON.parse(await read('site/sandbox-engine/build.json'));
  for (const f of info.files) {
    const bytes = await read('site/sandbox-engine/' + f.name);
    assert.equal(bytes.length, f.size);
    assert.equal(createHash('sha256').update(bytes).digest('hex'), f.sha256);
  }
  const url = new URL('../site/sandbox-engine/pvz-portable.js', import.meta.url);
  let resolve, reject;
  const ready = new Promise((yes, no) => { resolve = yes; reject = no; });
  const Module = { noInitialRun: true, onRuntimeInitialized: resolve, onAbort: reject };
  const ctx = vm.createContext({ Module, require: createRequire(url), __dirname: fileURLToPath(new URL('./', url)), __filename: fileURLToPath(url), process, Buffer, TextEncoder, TextDecoder, URL, console, performance, setTimeout, clearTimeout, setInterval, clearInterval });
  vm.runInContext(await readFile(url, 'utf8'), ctx);
  await ready;
  assert.equal(typeof Module.callMain, 'function');
  assert.ok(Module.FS.filesystems.IDBFS);
  for (let command = 0; command <= 21; command++) assert.equal(Module._pvz_sandbox_command(command, 0, 0, 0), -1);
  assert.equal(Module._pvz_sandbox_plant_data(0, 0), -1);
});

test('sandbox source and all ten active technology plant IDs are included', async () => {
  assert.deepEqual(TECH_PLANTS.map(p => p.id), Array.from({length:10},(_,i)=>100+i));
  const layout = {schema: 1, map: 0, plants: [{type: 100, col: 2, row: 2}]};
  assert.deepEqual(validateLayout(layout), layout);
  for (const file of ['Sandbox.cpp', 'SandboxUI.cpp', 'SandboxPlants.cpp', 'SandboxButton.cpp']) assert.ok((await read('src/' + file)).length > 1000);
  assert.match((await read('src/Sandbox.cpp')).toString(), /case 21:/);
  assert.match((await read('src/SandboxUI.cpp')).toString(), /根网共振/);
  assert.match((await read('src/Lawn/Widget/GameSelector.cpp')).toString(), /SandboxEnter\(\)/);
  assert.match((await read('src/Lawn/System/SaveGame.cpp')).toString(), /if \(gSandboxEnabled\) return false/);
});

test('legacy import verification and bundled entry retain the exact complete resource pack', async () => {
  const bytes = new Uint8Array([1, 2, 3]);
  const manifest = {bundle: {size: 3, sha256: await sha256(bytes)}};
  assert.deepEqual(await verifyResourceBytes(bytes.buffer, manifest), bytes);
  await assert.rejects(verifyResourceBytes(new Uint8Array([1]), manifest), /版本不匹配/);
  await assert.rejects(verifyResourceBytes(new Uint8Array([3, 2, 1]), manifest), /校验失败/);
  const deployed = JSON.parse(await read('site/resource-manifest.json'));
  const baseline=JSON.parse(await read('tests/baseline-assets.json'));
  const original=deployed.files.filter(f=>!f.path.startsWith('images/sandbox/'));
  assert.equal(original.length,baseline.fileCount);
  assert.equal(createHash('sha256').update(original.map(f=>`${f.path}:${f.size}:${f.sha256}`).join('\n')).digest('hex'),baseline.sha256);
});

test('cache blocked, denied and hanging cases fail open without touching save databases', async () => {
  assert.equal(await cacheResource('get', 'x', null, null), null);
  assert.equal(await cacheResource('get', 'x', null, {open() {throw Error('denied');}}), null);
  assert.equal(await cacheResource('get', 'x', null, {open(name) {
    assert.equal(name, 'pvz.pages.resource-cache.v1');
    const req = {}; queueMicrotask(() => req.onblocked()); return req;
  }}), null);
  assert.equal(await cacheResource('get', 'x', null, {open() {return {}; }}, 10), null);
});

test('cache writes resolve on transaction commit, and cached bytes can be read', async () => {
  const bytes = new Uint8Array([1, 2]), stored = new Map();
  const factory = {open(name) {
    assert.equal(name, 'pvz.pages.resource-cache.v1');
    const db = {close() {}, transaction(storeName) {
      assert.equal(storeName, 'bundles');
      const transaction = {objectStore() {return {
        clear() {stored.clear();},
        put(value, key) {stored.set(key, value); queueMicrotask(() => transaction.oncomplete());},
        get(key) {const result = {result: stored.get(key)}; queueMicrotask(() => { result.onsuccess(); transaction.oncomplete(); }); return result;},
      };}};
      return transaction;
    }};
    const request = {result: db}; queueMicrotask(() => request.onsuccess()); return request;
  }};
  assert.equal(await cacheResource('put', 'hash', bytes, factory), true);
  assert.deepEqual(await cacheResource('get', 'hash', null, factory), bytes);
});

async function importHarness(cached = null) {
  const elements = new Map(), writes = [];
  const getElementById = id => {
    if (!elements.has(id)) {
      const handlers = new Map();
      elements.set(id, {hidden: false, disabled: false, firstChild: {}, files: [], value: '', click() {}, addEventListener(k, fn) {handlers.set(k, fn);}, removeEventListener(k) {handlers.delete(k);}, dispatch(k) {return handlers.get(k)?.();}});
    }
    return elements.get(id);
  };
  const ctx = vm.createContext({document: {getElementById}, Uint8Array, sha256, cacheResource: async (mode, key, value) => {if (mode === 'get') return cached; writes.push(value); return true;}});
  const code = (await read('web/resource-import.mjs')).toString().replace(/^import .*;\n/gm, '').replaceAll('export async function', 'async function');
  vm.runInContext(code, ctx);
  const bytes = new Uint8Array([4, 5, 6]);
  const manifest = {bundle: {size: 3, sha256: await sha256(bytes)}};
  const status = [];
  const result = ctx.importResourceBundle(manifest, text => status.push(text));
  await new Promise(setImmediate);
  return {element: getElementById, result, bytes, status, writes};
}

test('first import: cancellation stays ready, wrong file can retry, valid file finishes without upload', async () => {
  const app = await importHarness();
  assert.equal(app.element('resource-picker').hidden, false);
  assert.equal(app.element('start').hidden, true);
  const input = app.element('resource-file');
  await input.dispatch('change');
  input.files = [{size: 9}];
  await input.dispatch('change');
  assert.match(app.element('resource-hint').textContent, /请选修复后的/);
  assert.equal(app.element('choose-resource').disabled, false);
  input.files = [{size: 3, arrayBuffer: async () => app.bytes.buffer}];
  await input.dispatch('change');
  assert.deepEqual(await app.result, app.bytes);
  assert.equal(app.element('resource-picker').hidden, true);
  assert.equal(app.element('start').hidden, false);
  assert.equal(app.writes.length, 1);
});

test('returning visit reuses a verified resource cache without prompting again', async () => {
  const app = await importHarness(new Uint8Array([4, 5, 6]));
  assert.deepEqual(await app.result, app.bytes);
  assert.ok(app.status.includes('已读取本机资源'));
  assert.equal(app.writes.length, 0);
});
