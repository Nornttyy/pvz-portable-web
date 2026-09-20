import {sha256, LIMITS} from './resource-utils.mjs';
import {cacheResource} from './resource-cache.mjs';

export async function verifyDownloadedBundle(value, manifest) {
  const bytes = value instanceof Uint8Array ? value : new Uint8Array(value);
  if (bytes.length !== manifest.bundle.size || await sha256(bytes) !== manifest.bundle.sha256)
    throw Error('游戏文件下载不完整，请重新加载。');
  return bytes;
}

async function download(manifest, setStatus, fetcher, timeoutMs) {
  const controller = new AbortController();
  const timer = setTimeout(() => controller.abort(), timeoutMs);
  let reader;
  try {
    const response = await fetcher(manifest.bundle.url, {signal:controller.signal, cache:'no-cache'});
    if (!response.ok) throw Error(`游戏文件暂时无法下载（${response.status}），请稍后重试。`);
    const total = manifest.bundle.size;
    const declared = Number(response.headers.get('content-length'));
    if (declared && declared !== total) throw Error('游戏文件大小异常，请重新加载。');
    if (!response.body?.getReader) return await verifyDownloadedBundle(await response.arrayBuffer(), manifest);
    const bytes = new Uint8Array(total);
    reader = response.body.getReader();
    let offset = 0;
    while (true) {
      const {done, value} = await reader.read();
      if (done) break;
      if (offset + value.length > total) throw Error('游戏文件大小异常，请重新加载。');
      bytes.set(value, offset); offset += value.length;
      setStatus(`正在下载游戏 ${(offset / 1048576).toFixed(1)} / ${(total / 1048576).toFixed(1)} MB`, 8 + 46 * offset / total);
    }
    if (offset !== total) throw Error('游戏文件下载中断，请重新加载。');
    setStatus('校验游戏文件…', 55);
    return await verifyDownloadedBundle(bytes, manifest);
  } catch (error) {
    controller.abort();
    try { await reader?.cancel(); } catch {}
    if (error.name === 'AbortError') throw Error('游戏下载超时，请检查网络后重新加载。');
    throw error;
  } finally { clearTimeout(timer); reader?.releaseLock(); }
}

// No file picker. Only a versioned, same-site package may be downloaded.
export async function loadResourceBundle(manifest, setStatus, {
  fetcher = globalThis.fetch, cache = cacheResource, timeoutMs = 300000,
} = {}) {
  const {bundle} = manifest;
  if (manifest.delivery !== 'bundled' || !/^[a-f0-9]{64}$/.test(bundle?.sha256 || '') ||
      bundle.url !== `resources/game-${bundle.sha256.slice(0,12)}.zip` ||
      !Number.isSafeInteger(bundle.size) || bundle.size < 1 || bundle.size > LIMITS.totalBytes)
    throw Error('游戏资源地址不正确。');
  setStatus('读取游戏缓存…', 8);
  const cached = await cache('get', bundle.sha256);
  if (cached) {
    try { const bytes = await verifyDownloadedBundle(cached, manifest); setStatus('已读取游戏缓存', 56); return bytes; }
    catch { /* Replace only the resource cache; player saves are separate. */ }
  }
  setStatus('正在下载游戏，请稍候…', 8);
  let bytes;
  for (let attempt = 0; attempt < 2; attempt++) {
    try { bytes = await download(manifest, setStatus, fetcher, timeoutMs); break; }
    catch (error) {
      if (attempt) throw error;
      setStatus('下载中断，正在重试…', 8);
    }
  }
  setStatus('保存游戏缓存…', 56);
  await cache('put', bundle.sha256, bytes);
  return bytes;
}
