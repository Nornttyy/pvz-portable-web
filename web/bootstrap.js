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
var Module = {
  canvas: document.getElementById('canvas'),
  noInitialRun: true,
  locateFile: name => new URL('sandbox-engine/' + name, window.location.href).href,
  onRuntimeInitialized: () => window.pvzResolveEngine(),
  onAbort: message => {
    recordEngineMessage(message);
    window.pvzRejectEngine(new Error(String(message)));
    window.dispatchEvent(new CustomEvent('pvz-fatal', { detail: String(message) }));
  },
  print: message => { recordEngineMessage(message); console.log(message); },
  printErr: message => { recordEngineMessage(message); console.warn(message); },
};
Module.canvas.addEventListener('contextmenu', event => event.preventDefault());
Module.canvas.addEventListener('webglcontextlost', event => {
  event.preventDefault();
  window.dispatchEvent(new CustomEvent('pvz-fatal', { detail: '浏览器图形上下文丢失。请重新加载游戏，必要时关闭其他高负载标签页。' }));
});
window.addEventListener('error', event => {
  if (event.target?.tagName === 'SCRIPT') {
    const message = '引擎文件下载失败，请检查网络后刷新重试。';
    window.pvzRejectEngine(new Error(message));
    window.dispatchEvent(new CustomEvent('pvz-fatal', { detail: message }));
  } else if (event.error && event.filename?.includes('pvz-portable.js')) {
    recordEngineMessage(event.error.stack || event.message);
    window.dispatchEvent(new CustomEvent('pvz-fatal', { detail: event.message }));
  }
}, true);
