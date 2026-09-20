// Retain the legacy resource importer; publish the integrated sandbox as the default.
import { readFile, writeFile, copyFile, readdir } from 'node:fs/promises';

const root = new URL('../', import.meta.url);
let html = await readFile(new URL('wasm/shell.html', root), 'utf8');
function replace(before, after) {
  if (!html.includes(before)) throw new Error(`Upstream shell changed: ${before.slice(0, 80)}`);
  html = html.replaceAll(before, after);
}

replace('<html lang="en">', '<html lang="zh-CN">');
replace('width=device-width, initial-scale=1.0', 'width=device-width, initial-scale=1.0, viewport-fit=cover');
replace('<title>PvZ Portable — Web</title>', '<title>PvZ Portable · 网页版</title>\n<meta name="robots" content="noindex, nofollow">');
replace('https://cdn.jsdelivr.net/npm/jszip@3/dist/jszip.min.js', 'vendor/jszip-3.10.1.min.js');
replace('{{{ SCRIPT }}}', '<script async src="pvz-portable.js" onerror="window._rejectModuleReady(new Error(\'引擎文件下载失败，请刷新重试\'))"></script>');
replace('PvZ Portable | Play Anywhere', 'PvZ Portable · 网页版');
replace('    <p id="project-links">', '    <p id="engine-status" role="status" aria-live="polite">正在加载引擎…</p>\n    <p id="project-links">');
replace('>GitHub</a>', '>上游项目</a>');
replace('>Issues</a>', '>上游反馈</a>');
replace('>Releases</a>', '>上游下载</a>\n        <a href="art-preview.html">分件样张</a>\n        <a href="credits.html">来源与许可</a>');
replace('Community-driven re-implementation for <strong>all desktop, mobile, and web</strong>. Play instantly in your browser with no installation. <strong>No game resources are included.</strong>', '社区重实现引擎 · 独立网页部署。<strong>本站不附带原版游戏素材。</strong>');
replace('To use this engine, you <strong>MUST</strong> legally purchase the original game on', '请先通过');
replace('Steam</a> or ', 'Steam</a> 或 ');
replace("EA's official website</a>.<br>", 'EA 官方网站</a>购买原版游戏。<br>');
replace('Please provide your own <code>main.pak</code> and <code>properties/</code> folder from your legally purchased copy of Plants&nbsp;vs.&nbsp;Zombies GOTY Edition.', '从你购买的年度版中导入 <code>main.pak</code> 和 <code>properties/</code> 文件夹。生成的分件样张不能代替这套资源。');
replace('Disclaimer: This strictly educational project is a community-driven engine re-implementation. It is not affiliated with, authorized, or endorsed by PopCap Games or Electronic Arts.', '非官方学习项目，与 PopCap / EA 无隶属、授权或背书关系。');

const messages = [
  ['📂 Drop resources here — import and play instantly!', '导入你的游戏资源'],
  ['Drop a ZIP or folder containing <code>main.pak</code> &amp; <code>properties/</code>, or pick one below:', '拖入包含 <code>main.pak</code> 与 <code>properties/</code> 的 ZIP 或文件夹，也可以点击下方按钮：'],
  ['Import Resource ZIP', '导入资源 ZIP'],
  ['Select a ZIP package that contains <code>main.pak</code> and <code>properties/</code>.', '手机建议使用 ZIP，内含 main.pak 与 properties/。'],
  ['Select Resource Folder', '选择资源文件夹'],
  ['Select a folder that directly contains <code>main.pak</code> and <code>properties/</code>.', '电脑可直接选择包含这两项内容的文件夹。'],
  ['▶ Start Game', '▶ 开始游戏'],
  ["'Start Game'", "'开始游戏'"],
  ['Resources loaded successfully. Click here to launch!', '资源已就绪，点击这里开始。'],
  ['Re-select resources', '重新选择资源'],
  ['Import saves from ZIP before starting', '开始前从 ZIP 导入存档'],
  ['Import saves from a folder before starting', '开始前从文件夹导入存档'],
  ['Delete all saved data stored in this browser', '删除此浏览器中本引擎的全部存档'],
  ['Import Save ZIP', '导入存档 ZIP'],
  ['Import Save Folder', '导入存档文件夹'],
  ['Clear Browser Saves', '清空本地存档'],
  ['Your files stay in your browser and are never uploaded to any server.<br>Resources can be loaded from a ZIP package or a resource folder. Saves are stored locally in your browser via IndexedDB and can be imported from ZIP or folder.', '资源只在你的浏览器中读取，不上传服务器。存档保存在本机浏览器，换设备前请导出备份。关闭网页后再次打开，需要重新选择资源。'],
  ['Starting the game indicates that you have read the notices above and acknowledge the no-warranty terms of the LGPL v3.0-or-later license for this code.', '代码按 LGPL v3.0-or-later 提供，不附带任何担保。<a href="credits.html">查看许可与源代码</a>。'],
  ['Export saves as ZIP', '将存档导出为 ZIP'],
  ['💾 Export Saves', '💾 导出存档'],
  ['⏳ Importing ZIP…', '⏳ 正在导入 ZIP…'],
  ['Resource ZIP import failed: ', '资源 ZIP 导入失败：'],
  ['Preparing WebAssembly runtime…', '正在准备游戏引擎…'],
  ['Writing resources… ', '正在读取资源… '],
  ['Restoring saved data…', '正在恢复存档…'],
  ['⏳ Exporting…', '⏳ 正在导出…'],
  ['No save data found.', '还没有存档。'],
  ['Export failed: ', '导出失败：'],
  ['⏳ Importing…', '⏳ 正在导入…'],
  ['Import failed: ', '导入失败：'],
  ['Save folder import failed: ', '存档文件夹导入失败：'],
  ['This will permanently delete ALL save data from your browser.\\nThis cannot be undone. Continue?', '将永久删除此浏览器中本引擎的全部存档，无法撤销。确定继续？'],
  ['⏳ Clearing…', '⏳ 正在清空…'],
  ['All save data has been cleared.', '本地存档已清空。'],
  ['Clear failed: ', '清空失败：'],
  ['Recent save data may still be syncing. Wait a moment or export saves before leaving.', '最新存档可能尚未写入，请稍候或先导出存档。'],
  ['Resetting WebAssembly runtime…', '正在重置引擎…'],
];
for (const [before, after] of messages) replace(before, after);
replace('</style>', `
body, #upload-screen { min-height: 100dvh; }
#canvas-container { height: 100dvh; }
#upload-screen a { color: #4ecca3; }
#engine-status { font-size: .85rem; }
#engine-status[data-state="error"] { color: #ffaaa5; }
#upload-screen { padding-bottom: max(2rem, env(safe-area-inset-bottom)); }
</style>`);
replace('</body>', '<script src="runtime-status.js"></script>\n</body>');
await writeFile(new URL('site/classic.html', root), html);
for (const name of await readdir(new URL('web/', root))) {
  if (/\.(html|css|js|mjs)$/.test(name)) await copyFile(new URL('web/' + name, root), new URL('site/' + name, root));
}
console.log('Built Chinese integrated sandbox and preserved classic importer. No original game resource packs included.');
