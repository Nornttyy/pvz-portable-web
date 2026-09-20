import { sha256 } from './resource-utils.mjs';
import { cacheResource } from './resource-cache.mjs';

export async function verifyResourceBytes(value, manifest) {
  const bytes = value instanceof Uint8Array ? value : new Uint8Array(value);
  if (bytes.byteLength !== manifest.bundle.size) throw Error('资源包版本不匹配，请选择修复加载问题后的 local-resources.zip。');
  if (await sha256(bytes) !== manifest.bundle.sha256) throw Error('资源包校验失败，请选择修复后的资源包，勿解压或修改。');
  return bytes;
}

// This flow only reads a local file; no upload, fetch or remote storage operation.
export async function importResourceBundle(manifest, setStatus, { skipCache = false } = {}) {
  const $ = id => document.getElementById(id);
  setStatus('检查此浏览器中已保存的资源…', 8);
  const cached = skipCache ? null : await cacheResource('get', manifest.bundle.sha256);
  if (cached) {
    try {
      const bytes = await verifyResourceBytes(cached, manifest);
      setStatus('已读取本机资源', 56);
      return bytes;
    } catch { /* A stale or incomplete resource cache never affects saves. */ }
  }
  $('resource-picker').hidden = false;
  $('start').hidden = true;
  setStatus(skipCache ? '请选择资源包，已有存档会保留' : '请选择本机资源包', 8);
  const fileInput = $('resource-file'), choose = $('choose-resource');
  return new Promise(resolve => {
    const open = () => fileInput.click();
    choose.addEventListener('click', open);
    const select = async () => {
      const file = fileInput.files?.[0];
      fileInput.value = '';
      if (!file || choose.disabled) return;
      choose.disabled = true;
      try {
        if (file.size !== manifest.bundle.size) throw Error('请选修复后的 local-resources.zip（约 50.5 MB），不是存档 ZIP 或原版 main.pak。');
        setStatus('正在校验本机资源…', 30);
        const bytes = await verifyResourceBytes(await file.arrayBuffer(), manifest);
        setStatus('在此浏览器中记住资源…', 45);
        const saved = await cacheResource('put', manifest.bundle.sha256, bytes);
        $('loader-note').firstChild.textContent = saved
          ? '资源已记住，下次可直接加载。资源和存档都只在此浏览器中。'
          : '本次可以游玩；浏览器未允许缓存，下次需重新选择资源包。';
        choose.removeEventListener('click', open);
        fileInput.removeEventListener('change', select);
        $('resource-picker').hidden = true;
        $('start').hidden = false;
        setStatus('资源校验通过', 56);
        resolve(bytes);
      } catch (error) {
        $('resource-hint').textContent = error.message;
        setStatus('资源未导入，请重新选择', 8);
      } finally { choose.disabled = false; }
    };
    fileInput.addEventListener('change', select);
  });
}
