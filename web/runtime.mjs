import { validateManifest, sha256, writeVirtualFile, normalizeSavePath, listVirtualFiles, createPakBuilder, LIMITS } from './resource-utils.mjs';
import { loadResourceBundle } from './resource-loader.mjs';
import { syncFileSystem, fitCanvas } from './loading-utils.mjs';

const $ = id => document.getElementById(id);
const status = $('status');
const progress = $('progress');
const start = $('start');
let phase = 'loading';
let saveMounted = false;
let persistentSaves = false;
let syncPromise = null;
let lastSaved = 0;
let saveTimer;
let readyTimeout;
const waitFrame = () => new Promise(resolve => setTimeout(resolve, 0));

function setStatus(message, value) {
  if (phase === 'error') return;
  status.textContent = message;
  if (value !== undefined) progress.value = value;
}
function reportError(error) {
  if (phase === 'playing') Module.pauseMainLoop?.();
  phase = 'error';
  clearTimeout(readyTimeout);
  clearInterval(saveTimer);
  $('loader').hidden = false;
  $('canvas-container').hidden = true;
  $('tools').hidden = true;
  start.hidden = true;
  start.disabled = true;
  $('reload').hidden = false;
  $('diagnostics').hidden = false;
  status.textContent = String(error?.message || '未能启动，请重新加载。');
  $('error-detail').textContent = String(error?.message || error) + '\n网站版本：' + (document.documentElement?.dataset?.version || 'dev') + '\n\n' + window.pvzEngineLog.join('\n');
  console.error(error);
}

async function fetchBundle(manifest) {
  return loadResourceBundle(manifest, setStatus);
}

async function setupSaves() {
  const FS = Module.FS;
  if (!FS.analyzePath('/saves').exists) FS.mkdir('/saves');
  try {
    FS.mount(FS.filesystems.IDBFS, {}, '/saves');
    saveMounted = true;
    await syncFileSystem(FS, true);
    persistentSaves = true;
    lastSaved = Date.now();
    $('save-status').textContent = '自动保存已开启';
  } catch (error) {
    persistentSaves = false;
    if (error.name === 'TimeoutError') throw error;
    $('save-status').textContent = '浏览器未允许存档，请用“备份存档”保存。';
    console.warn('Local save persistence unavailable', error);
  }
}

async function syncSaves() {
  if (!saveMounted || !persistentSaves) return false;
  if (syncPromise) return syncPromise;
  syncPromise = syncFileSystem(Module.FS, false).then(() => {
    lastSaved = Date.now(); $('save-status').textContent = '已自动保存'; return true;
  }, error => {
    if (error.name === 'TimeoutError') persistentSaves = false;
    $('save-status').textContent = '自动保存失败，请手动备份'; console.warn(error); return false;
  });
  try { return await syncPromise; } finally { syncPromise = null; }
}

async function prepareGame() {
  if (typeof WebAssembly === 'undefined' || !globalThis.crypto?.subtle) throw Error('此浏览器不支持所需的 WebAssembly 或安全校验功能，请使用较新版本的浏览器。');
  if (!window.JSZip) throw Error('本地解压组件没有加载成功');
  readyTimeout = setTimeout(() => reportError(new Error('引擎准备超时。请确认浏览器支持 WebAssembly，并重新加载。')), 60000);
  setStatus('读取资源清单…', 4);
  let response;
  try { response = await fetch('resource-manifest.json', { cache: 'no-store', signal: AbortSignal.timeout(10000) }); }
  catch { throw Error('资源清单下载失败，请检查网络后重新加载。'); }
  if (!response.ok) throw Error('资源清单暂时不可用，请稍后重新加载。');
  const manifest = validateManifest(await response.json());
  setStatus('正在下载并准备游戏引擎…', 6);
  await window.pvzEngineReady;
  if (phase === 'error') return;
  clearTimeout(readyTimeout);
  // The streamed resource download has its own timeout and progress reporting.
  const bytes = await fetchBundle(manifest);
  if (phase === 'error') return;
  readyTimeout = setTimeout(() => reportError(new Error('资源或存档载入超时，请刷新重试；已有存档不会删除。')), 120000);
  const archive = await JSZip.loadAsync(bytes, { checkCRC32: true });
  const entries = Object.values(archive.files).filter(entry => !entry.dir);
  if (entries.length !== manifest.files.length) throw Error('资源包内的文件数与清单不符');
  const nativePak = createPakBuilder(manifest.files);
  for (let i = 0; i < manifest.files.length; i++) {
    if (phase === 'error') return;
    const expected = manifest.files[i];
    const entry = archive.file(expected.path);
    if (!entry || (entry.unsafeOriginalName && entry.unsafeOriginalName !== expected.path)) throw Error('资源包路径校验失败');
    const data = await entry.async('uint8array');
    if (data.length !== expected.size || await sha256(data) !== expected.sha256) throw Error(`资源校验失败：${expected.path}`);
    writeVirtualFile(Module.FS, '/resources', expected.path, data);
    nativePak.append(expected.path, data);
    if (i % 24 === 0) {
      setStatus(`载入分件、动画与场景 ${i + 1} / ${manifest.files.length}`, 58 + 35 * (i + 1) / manifest.files.length);
      await waitFrame();
    }
  }
  setStatus('准备中文资源与透明图层…', 94);
  Module.FS.writeFile('/resources/main.pak', nativePak.finish());
  setStatus('恢复本地存档…', 96);
  await setupSaves();
  if (phase === 'error') return;
  clearTimeout(readyTimeout);
  phase = 'ready';
  setStatus('准备好了', 100);
  start.disabled = false;
  start.textContent = '开始游戏';
  start.focus();
}

function resizeCanvas() {
  const canvas = Module.canvas;
  const container = $('canvas-container');
  const style = window.getComputedStyle?.(container);
  const padding = name => parseFloat(style?.[name]) || 0;
  const size = fitCanvas(canvas.width, canvas.height,
    container.clientWidth - padding('paddingLeft') - padding('paddingRight'),
    container.clientHeight - padding('paddingTop') - padding('paddingBottom'));
  if (!size) return;
  canvas.style.width = size.width + 'px';
  canvas.style.height = size.height + 'px';
}

start.addEventListener('click', () => {
  if (phase !== 'ready') return;
  phase = 'playing';
  $('loader').hidden = true;
  $('canvas-container').hidden = false;
  $('tools').hidden = false;
  resizeCanvas();
  Module.canvas.focus();
  try {
    Module.callMain(['-resdir', '/resources/']);
    saveTimer = setInterval(() => { void syncSaves(); }, 5000);
  } catch (error) { reportError(error); }
});

function download(bytes, filename) {
  const url = URL.createObjectURL(new Blob([bytes], { type: 'application/zip' }));
  const anchor = document.createElement('a');
  anchor.href = url;
  anchor.download = filename;
  anchor.click();
  setTimeout(() => URL.revokeObjectURL(url), 10000);
}

async function exportSaves() {
  await syncSaves();
  const archive = new JSZip();
  let count = 0;
  for (const full of listVirtualFiles(Module.FS, '/saves')) {
    const relative = full.slice('/saves/'.length);
    try { normalizeSavePath(relative); } catch { continue; }
    archive.file(relative, Module.FS.readFile(full));
    count++;
  }
  if (!count) { $('save-status').textContent = '先在游戏中创建玩家并开始游玩'; return false; }
  download(await archive.generateAsync({ type: 'uint8array', compression: 'DEFLATE' }), 'pvz-local-save-' + new Date().toISOString().slice(0, 10) + '.zip');
  $('save-status').textContent = '存档备份已下载';
  return true;
}

$('export-save').addEventListener('click', async () => {
  $('export-save').disabled = true;
  try { await exportSaves(); } catch (error) { $('save-status').textContent = '备份失败：' + error.message; }
  finally { $('export-save').disabled = false; }
});
$('import-save').addEventListener('click', () => $('save-file').click());
$('save-file').addEventListener('change', async event => {
  const file = event.target.files[0];
  event.target.value = '';
  if (!file) return;
  let paused = false;
  let previousFiles;
  try {
    if (!persistentSaves) throw Error('浏览器存档不可用，无法恢复后重启。');
    if (file.size > LIMITS.saves) throw Error('备份文件过大');
    const zip = await JSZip.loadAsync(file, { checkCRC32: true });
    const entries = Object.values(zip.files).filter(entry => !entry.dir);
    if (!entries.length || entries.length > 200) throw Error('备份内的文件数不正确');
    const pending = [];
    const seen = new Set();
    let total = 0;
    for (const entry of entries) {
      const path = normalizeSavePath(entry.unsafeOriginalName ?? entry.name);
      if (seen.has(path.toLowerCase())) throw Error('备份存在重复文件');
      seen.add(path.toLowerCase());
      if (entry._data?.uncompressedSize > LIMITS.saves) throw Error('备份展开后过大');
      const data = await entry.async('uint8array');
      total += data.length;
      if (total > LIMITS.saves) throw Error('备份展开后过大');
      pending.push({ path, data });
    }
    if (!confirm('将恢复选中的备份并重启游戏。会先下载当前存档的备份，继续吗？')) return;
    Module.pauseMainLoop();
    paused = true;
    await exportSaves();
    clearInterval(saveTimer);
    previousFiles = new Map(pending.map(entry => {
      const full = '/saves/' + entry.path;
      return [full, Module.FS.analyzePath(full).exists ? Module.FS.readFile(full) : null];
    }));
    for (const entry of pending) writeVirtualFile(Module.FS, '/saves', entry.path, entry.data);
    if (!await syncSaves()) throw Error('恢复存档无法保存到浏览器，请勿刷新。');
    phase = 'exiting';
    location.reload();
  } catch (error) {
    if (previousFiles) {
      for (const [full, bytes] of previousFiles) {
        if (bytes !== null) Module.FS.writeFile(full, bytes);
        else if (Module.FS.analyzePath(full).exists) Module.FS.unlink(full);
      }
      await syncSaves();
    }
    $('save-status').textContent = '恢复失败：' + error.message;
    if (paused) {
      Module.resumeMainLoop();
      clearInterval(saveTimer);
      saveTimer = setInterval(() => { void syncSaves(); }, 5000);
    }
  }
});
$('fullscreen').addEventListener('click', async () => {
  try {
    if (document.fullscreenElement) await document.exitFullscreen();
    else await document.documentElement.requestFullscreen();
  } catch { $('save-status').textContent = '此浏览器不支持全屏'; }
});
$('return').addEventListener('click', async () => {
  if (!confirm('返回启动页？建议先在游戏菜单中保存并退出。')) return;
  const synced = await syncSaves();
  if (!synced) await exportSaves();
  phase = 'exiting';
  location.reload();
});
$('reload').addEventListener('click', () => location.reload());
window.addEventListener('resize', resizeCanvas);
window.visualViewport?.addEventListener('resize', resizeCanvas);
window.addEventListener('orientationchange', () => setTimeout(resizeCanvas, 100));
document.addEventListener('fullscreenchange', resizeCanvas);
document.addEventListener('visibilitychange', () => { if (document.hidden) void syncSaves(); });
window.addEventListener('pagehide', () => { void syncSaves(); });
window.addEventListener('beforeunload', event => {
  if (phase !== 'playing') return;
  void syncSaves();
  if (!persistentSaves || Date.now() - lastSaved > 7000) { event.preventDefault(); event.returnValue = ''; }
});
window.addEventListener('pvz-fatal', event => reportError(new Error(event.detail)));
window.onGameExit = async () => {
  phase = 'exiting';
  clearInterval(saveTimer);
  const synced = await syncSaves();
  if (!synced) await exportSaves();
  location.reload();
};

window.pvzLoaderAttached = true;
clearTimeout(window.pvzBootWatchdog);
if (window.pvzEarlyFailure) reportError(new Error(window.pvzEarlyFailure));
else prepareGame().catch(reportError);
