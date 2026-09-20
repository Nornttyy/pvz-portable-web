// A timed-out restore must not start a game while its original database read is pending.
export function syncFileSystem(FS, populate, timeoutMs = 15000) {
  return new Promise((resolve, reject) => {
    let settled = false;
    const finish = error => {
      if (settled) return;
      settled = true;
      clearTimeout(timer);
      error ? reject(error) : resolve();
    };
    const timer = setTimeout(() => {
      const error = new Error(populate ? '读取存档超时，请关闭同一游戏的其他标签页后重试。已有存档不会删除。' : '保存暂时没有响应，请先手动备份存档。');
      error.name = 'TimeoutError';
      finish(error);
    }, timeoutMs);
    try { FS.syncfs(populate, finish); } catch (error) { finish(error); }
  });
}
export function fitCanvas(width, height, availableWidth, availableHeight) {
  if (![width, height, availableWidth, availableHeight].every(n => Number.isFinite(n) && n > 0)) return null;
  const scale = Math.min(availableWidth / width, availableHeight / height);
  return {width: Math.floor(width * scale), height: Math.floor(height * scale)};
}
