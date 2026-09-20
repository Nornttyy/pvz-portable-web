// State-machine unit tests only. These do not replace real browser/gameplay QA.
import test from 'node:test';
import assert from 'node:assert/strict';
import vm from 'node:vm';
import { readFile } from 'node:fs/promises';
import { createRequire } from 'node:module';
import { webcrypto } from 'node:crypto';
import * as resourceUtils from '../web/resource-utils.mjs';

const JSZip = createRequire(import.meta.url)('../site/vendor/jszip-3.10.1.min.js');

// Minimal synthetic resource bytes for loader state tests, not game art.
const zipFixture = new JSZip();
const fixturePaths = ['properties/resources.xml', 'properties/LawnStrings.txt', 'images/background1.jpg', 'reanim/PeaShooter.reanim', 'reanim/Zombie.reanim'];
const files = [];
for (const path of fixturePaths) {
  const bytes = new TextEncoder().encode('synthetic-loader-fixture');
  zipFixture.file(path, bytes);
  files.push({path, size: bytes.length, sha256: await resourceUtils.sha256(bytes)});
}
const bundle = await zipFixture.generateAsync({type: 'uint8array'});
const manifest = {schema: 1, localOnly: true, files, totalFiles: files.length, totalBytes: files.reduce((n, f) => n + f.size, 0), bundle: {url: '/local-resources.zip', size: bundle.length, sha256: await resourceUtils.sha256(bundle)}};
const source = (await readFile(new URL('../web/runtime.mjs', import.meta.url), 'utf8')).replace(/^import [^;]+;/gm, '');

function memoryFS(failSync) {
  const directories = new Set(['/']);
  const data = new Map();
  return {
    filesystems: { IDBFS: {} },
    analyzePath: path => ({ exists: directories.has(path) || data.has(path) }),
    mkdir: path => directories.add(path), mount() {},
    syncfs: (_populate, done) => queueMicrotask(() => done(failSync ? Error('Storage blocked') : null)),
    writeFile: (path, bytes) => data.set(path, new Uint8Array(bytes)),
    readFile: path => { if (!data.has(path)) throw Error('Missing file'); return data.get(path).slice(); },
    unlink: path => data.delete(path),
    stat: path => ({ mode: directories.has(path) ? 1 : 0 }),
    isDir: mode => mode === 1,
    readdir: dir => ['.', '..', ...[...directories, ...data.keys()].filter(p => p !== dir && p.startsWith(dir + '/') && !p.slice(dir.length + 1).includes('/')).map(p => p.slice(dir.length + 1))],
  };
}

function harness(t, { corruptBundle = false, failSync = false, engineFailure = false } = {}) {
  const timers = new Set();
  const elements = new Map();
  const makeTarget = () => {
    const handlers = new Map();
    return {
      hidden: false, disabled: false, style: {}, width: 800, height: 600, clientWidth: 1024, clientHeight: 768,
      textContent: '', value: '', focus() {}, click() {},
      addEventListener: (type, fn) => handlers.set(type, fn),
      dispatch: async (type, event = {}) => handlers.get(type)?.(event),
    };
  };
  const element = id => { if (!elements.has(id)) elements.set(id, makeTarget()); return elements.get(id); };
  element('start').disabled = true;
  element('canvas-container').hidden = true;
  element('tools').hidden = true;
  element('reload').hidden = true;
  const calls = [];
  const Module = { FS: memoryFS(failSync), canvas: element('canvas'), callMain: args => calls.push([...args]), pauseMainLoop() {}, resumeMainLoop() {} };
  const document = { ...makeTarget(), getElementById: element, createElement: makeTarget };
  const ready = engineFailure ? Promise.reject(Error('WASM test failure')) : Promise.resolve();
  ready.catch(() => {});
  const window = { ...makeTarget(), JSZip, pvzEngineReady: ready, pvzEngineLog: [] };
  const location = { hostname: 'nornttyy.github.io', reload: () => calls.push('reload') };
  const context = vm.createContext({
    ...resourceUtils, importResourceBundle: async () => { if (corruptBundle) throw Error('资源包校验失败'); return bundle; }, window, document, Module, JSZip, location, crypto: webcrypto,
    AbortSignal, Blob, URL, WebAssembly, Uint8Array, console: { error() {}, warn() {} },
    confirm: () => false,
    setTimeout: (fn, ms) => { const timer = setTimeout(fn, ms); timers.add(timer); return timer; },
    clearTimeout,
    setInterval: () => 1, clearInterval() {},
    fetch: async path => path === 'resource-manifest.json'
      ? new Response(JSON.stringify(manifest), { headers: { 'Content-Type': 'application/json' } })
      : Promise.reject(Error('Unexpected network request: ' + path)),
  });
  t.after(() => { for (const timer of timers) clearTimeout(timer); });
  vm.runInContext(source, context, { filename: 'runtime.mjs' });
  return { element, Module, calls, window };
}

async function waitFor(check) {
  const deadline = Date.now() + 15000;
  while (!check()) {
    if (Date.now() > deadline) throw Error('Loader state did not settle');
    await new Promise(resolve => setTimeout(resolve, 5));
  }
}

test('Pages loader mounts imported resources and calls the engine entry only after user click', { timeout: 20000 }, async t => {
  const app = harness(t);
  await waitFor(() => !app.element('start').disabled);
  assert.equal(app.element('status').textContent, '准备好了');
  assert.equal(app.calls.length, 0, 'game starts only from user gesture');
  assert.ok(app.Module.FS.analyzePath('/resources/reanim/Zombie.reanim').exists);
  assert.ok(app.Module.FS.analyzePath('/resources/main.pak').exists, 'native PAK must exist before engine starts');
  await app.element('start').dispatch('click');
  assert.deepEqual(app.calls, [['-resdir', '/resources/']]);
  assert.equal(app.element('canvas-container').hidden, false);
  assert.equal(app.element('loader').hidden, true);
  assert.equal(app.Module.canvas.style.width, '1024px');
  assert.equal(app.Module.canvas.style.height, '768px');
  await app.element('start').dispatch('click');
  assert.equal(app.calls.length, 1, 'double clicking must not initialize two games');
});

test('corrupt assets never start the game and show a recoverable error', async t => {
  const app = harness(t, { corruptBundle: true });
  await waitFor(() => !app.element('reload').hidden);
  assert.match(app.element('error-detail').textContent, /资源包校验失败/);
  assert.equal(app.element('start').hidden, true);
  assert.equal(app.calls.length, 0);
});

test('integrated game retains persistent adventure saves and starts normally without a sandbox CLI flag', async t => {
  const app=harness(t);
  let mounts=0,syncs=0;
  app.Module.FS.mount=()=>{mounts++;};
  app.Module.FS.syncfs=(_populate,done)=>{syncs++;done(null);};
  await waitFor(()=>!app.element('start').disabled);
  assert.equal(mounts,1);assert.equal(syncs,1);
  assert.match(app.element('save-status').textContent,/自动保存已开启/);
  await app.element('start').dispatch('click');
  assert.deepEqual(app.calls,[['-resdir','/resources/']]);
});

test('WASM initialization failure is visible, not an endless progress screen', async t => {
  const app = harness(t, { engineFailure: true });
  await waitFor(() => !app.element('reload').hidden);
  assert.match(app.element('error-detail').textContent, /WASM test failure/);
  assert.equal(app.calls.length, 0);
});

test('blocked persistent storage still allows playing and clearly offers manual backups', { timeout: 20000 }, async t => {
  const app = harness(t, { failSync: true });
  await waitFor(() => !app.element('start').disabled);
  assert.match(app.element('save-status').textContent, /浏览器未允许存档/);
  await app.element('start').dispatch('click');
  assert.equal(app.calls.length, 1);
});

test('save restore rejects traversal before changing the virtual filesystem', { timeout: 20000 }, async t => {
  const app = harness(t);
  await waitFor(() => !app.element('start').disabled);
  const zip = new JSZip();
  zip.file('../userdata/user1.dat', 'invalid');
  const bytes = await zip.generateAsync({ type: 'uint8array' });
  bytes.size = bytes.length;
  await app.element('save-file').dispatch('change', { target: { files: [bytes], value: 'save.zip' } });
  assert.match(app.element('save-status').textContent, /恢复失败/);
  assert.equal(app.Module.FS.analyzePath('/saves/userdata/user1.dat').exists, false);
  assert.equal(app.calls.length, 0);
});
