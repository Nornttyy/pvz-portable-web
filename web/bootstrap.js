// Must be defined before the integrated Emscripten engine.
'use strict';
window.pvzEngineLog = [];
window.pvzEngineReady = new Promise((resolve, reject) => {
  window.pvzResolveEngine = resolve;
  window.pvzRejectEngine = reject;
});
// Keep early download/compile failures handled until the module UI attaches.
window.pvzEngineReady.catch(() => {});
function recordEngineMessage(message) {
  window.pvzEngineLog.push(String(message));
  if (window.pvzEngineLog.length > 80) window.pvzEngineLog.shift();
}
function failStartup(message) {
  window.pvzEarlyFailure = String(message);
  if (!window.pvzLoaderAttached) {
    document.getElementById('status').textContent = String(message);
    document.getElementById('start').hidden = true;
    document.getElementById('reload').hidden = false;
    document.getElementById('resource-picker').hidden = true;
  }
  window.dispatchEvent(new CustomEvent('pvz-fatal', { detail: String(message) }));
}
window.pvzBootWatchdog = setTimeout(() => {
  if (!window.pvzLoaderAttached) failStartup('网页组件加载超时，请检查网络后重新加载。');
}, 20000);
document.getElementById('reload').addEventListener('click', () => {
  if (!window.pvzLoaderAttached) window.location.reload();
});
function connectionStatus() {
  document.getElementById('connection-status').hidden = navigator.onLine !== false;
}
window.addEventListener('online', connectionStatus);
window.addEventListener('offline', connectionStatus);
connectionStatus();
document.getElementById('share-site').addEventListener('click', async () => {
  const url = new URL('./', window.location.href).href;
  const status = document.getElementById('share-status');
  status.hidden = false;
  try {
    await navigator.clipboard.writeText(url);
    status.textContent = '网址已复制；对方仍需自行导入资源包。';
  } catch {
    const input = document.getElementById('share-link');
    input.value = url; input.hidden = false; input.focus(); input.select();
    status.textContent = '长按上方网址复制；分享不会包含资源和存档。';
  }
});
var Module = {
  canvas: document.getElementById('canvas'),
  noInitialRun: true,
  locateFile: name => {
    const url = new URL('sandbox-engine/' + name, window.location.href);
    const version = document.documentElement?.dataset?.version;
    if (version) url.searchParams.set('v', version);
    return url.href;
  },
  onRuntimeInitialized: () => window.pvzResolveEngine(),
  onAbort: message => {
    recordEngineMessage(message);
    window.pvzRejectEngine(new Error(String(message)));
    failStartup(message);
  },
  print: message => { recordEngineMessage(message); console.log(message); },
  printErr: message => { recordEngineMessage(message); console.warn(message); },
};
Module.canvas.addEventListener('contextmenu', event => event.preventDefault());
Module.canvas.addEventListener('webglcontextlost', event => {
  event.preventDefault();
  failStartup('浏览器图形上下文丢失。请重新加载游戏，必要时关闭其他高负载标签页。');
});
window.addEventListener('error', event => {
  if (event.target?.tagName === 'SCRIPT') {
    const message = '游戏组件下载失败，请检查网络后重新加载。';
    window.pvzRejectEngine(new Error(message));
    failStartup(message);
  } else if (event.error && event.filename?.includes('pvz-portable.js')) {
    recordEngineMessage(event.error.stack || event.message);
    failStartup(event.message);
  }
}, true);
