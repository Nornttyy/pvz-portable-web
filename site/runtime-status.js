// Status is separate from the unmodified engine; importing is still handled upstream.
(() => {
  const status = document.getElementById('engine-status');
  const waiting = setTimeout(() => {
    status.textContent = '引擎仍在加载，网络较慢时请稍候；也可刷新重试。';
  }, 20000);
  Promise.resolve(window.moduleReadyPromise).then(() => {
    clearTimeout(waiting);
    status.dataset.state = 'ready';
    status.textContent = '引擎已就绪 · 请导入游戏资源';
  }, (error) => {
    clearTimeout(waiting);
    status.dataset.state = 'error';
    status.textContent = `引擎加载失败，请刷新重试。${error?.message || ''}`;
  });
})();
