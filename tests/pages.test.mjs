import test from 'node:test';
import './runtime-pages.test.mjs';
import './sandbox-pages.test.mjs';
import './website-loading.test.mjs';
import './automatic-resources.test.mjs';
import assert from 'node:assert/strict';
import { createHash } from 'node:crypto';
import { readFile, readdir, stat } from 'node:fs/promises';
import { Script, runInNewContext } from 'node:vm';
import { resolve, dirname, relative } from 'node:path';
import { fileURLToPath } from 'node:url';

const site = fileURLToPath(new URL('../site/', import.meta.url));
const entry = await readFile(resolve(site, 'classic.html'), 'utf8');
const provenance = JSON.parse(await readFile(resolve(site, 'upstream-release.json'), 'utf8'));
const hash = (data) => createHash('sha256').update(data).digest('hex');

for (const [path, digest] of Object.entries(provenance.files)) {
  test(`pinned release integrity: ${path}`, async () => {
    assert.equal(hash(await readFile(resolve(site, path))), digest);
  });
}

test('WebAssembly validates and compiles without game resources', async () => {
  const bytes = await readFile(resolve(site, 'pvz-portable.wasm'));
  assert.ok(WebAssembly.validate(bytes));
  const module = await WebAssembly.compile(bytes);
  assert.ok(WebAssembly.Module.exports(module).some(({ kind }) => kind === 'function'));
});

test('entry is localized and does not start the game without resources', () => {
  assert.match(entry, /<html lang="zh-CN">/);
  assert.match(entry, /返回自动加载的中文沙盒/);
  assert.match(entry, /生成的分件样张不能代替这套资源/);
  assert.match(entry, /noInitialRun:\s*true/);
  assert.match(entry, /const allReady = hasPak && hasProperties/);
  assert.ok(entry.includes("if (!dropZone.classList.contains('ready')) return;"), 'click handler must guard against missing resources');
  assert.match(entry, /window\._rejectModuleReady\(new Error\('引擎文件下载失败/);
  assert.doesNotMatch(entry, /\{\{\{|https:\/\/cdn\.jsdelivr/);
});

test('inline JavaScript and local runtime scripts parse', async () => {
  for (const [, script] of entry.matchAll(/<script\b[^>]*>([\s\S]*?)<\/script>/g)) {
    if (script.trim()) new Script(script);
  }
  for (const filename of ['pvz-portable.js', 'runtime-status.js', 'vendor/jszip-3.10.1.min.js']) {
    new Script(await readFile(resolve(site, filename), 'utf8'), { filename });
  }
});

for (const filename of ['index.html', 'classic.html', 'art-preview.html', 'credits.html']) {
  test(`local links resolve under the project subpath: ${filename}`, async () => {
    const html = await readFile(resolve(site, filename), 'utf8');
    for (const [, url] of html.matchAll(/(?:src|href)="([^"<>]+)"/g)) {
      if (/^(?:https?:|mailto:|#|data:)/.test(url)) continue;
      assert.ok(!url.startsWith('/'), `root-relative URL breaks project Pages: ${url}`);
      const path = resolve(site, dirname(filename), url.split(/[?#]/)[0]);
      assert.ok(!relative(site, path).startsWith('..'), `escaped site: ${url}`);
      assert.ok(await stat(path));
    }
    for (const [, src] of html.matchAll(/<script\b[^>]*\bsrc="([^"]+)"/g)) {
      assert.doesNotMatch(src, /^https?:/, 'entry should not need external scripts');
    }
    assert.match(html, /name="robots" content="noindex, nofollow"/);
  });
}

test('generated art is a disclosed static preview with PNG alpha', async () => {
  const art = await readFile(resolve(site, 'art-preview.html'), 'utf8');
  assert.match(art, /静态样张，尚未绑定骨骼或接入游戏/);
  assert.match(art, /不是完整游戏资源包，也不是原版素材/);
  const png = await readFile(resolve(site, 'assets/peashooter-parts-preview-v1.png'));
  assert.equal(png.subarray(0, 8).toString('hex'), '89504e470d0a1a0a');
  assert.equal(png.readUInt32BE(16), 1774);
  assert.equal(png.readUInt32BE(20), 887);
  assert.equal(png[25], 6, 'RGBA color type');
});

test('engine licenses and source links are retained', async () => {
  const credits = await readFile(resolve(site, 'credits.html'), 'utf8');
  assert.match(credits, /Copyright \(C\) 2026 Zhou Qiankang/);
  assert.match(credits, /This product includes portions of the PopCap Games Framework/);
  assert.match(credits, /Nornttyy\/pvz-portable-web\/tree\/main\/src/);
  for (const name of ['LGPL-3.0.txt', 'GPL-3.0.txt', 'PopCap-Framework.txt', 'SDL-Mixer-X.txt', 'JSZip-LICENSE.txt']) {
    assert.ok((await stat(resolve(site, 'licenses', name))).size > 500);
  }
});

test('published directory allows only the audited game ZIP, never player saves or secrets', async () => {
  const manifest = JSON.parse(await readFile(resolve(site, 'resource-manifest.json')));
  async function walk(path) {
    for (const file of await readdir(path, { withFileTypes: true })) {
      assert.ok(!file.isSymbolicLink());
      assert.doesNotMatch(file.name, /^(main\.pak|properties|userdata|\.env|\.git)$/i);
      if (file.name.endsWith('.zip')) assert.equal(relative(site, resolve(path,file.name)), manifest.bundle.url);
      else assert.doesNotMatch(file.name, /\.(?:pak|dat|v4)$/i);
      if (file.isDirectory()) await walk(resolve(path, file.name));
    }
  }
  await walk(site);
});

for (const outcome of ['ready', 'error']) {
  test(`runtime status handles ${outcome} without starting the game`, async () => {
    const status = { textContent: '', dataset: {} };
    let callback;
    let cleared = false;
    const code = await readFile(resolve(site, 'runtime-status.js'), 'utf8');
    const ready = outcome === 'ready' ? Promise.resolve() : Promise.reject(new Error('test failure'));
    runInNewContext(code, {
      window: { moduleReadyPromise: ready },
      document: { getElementById: (id) => { assert.equal(id, 'engine-status'); return status; } },
      setTimeout: (fn) => { callback = fn; return 1; },
      clearTimeout: () => { cleared = true; },
    });
    await new Promise(setImmediate);
    assert.equal(status.dataset.state, outcome);
    assert.ok(cleared);
    assert.match(status.textContent, outcome === 'ready' ? /请导入游戏资源/ : /加载失败/);
    callback();
    assert.match(status.textContent, /仍在加载/);
  });
}
