# GitHub Pages · 中文沙盒

网址：https://nornttyy.github.io/pvz-portable-web/

默认入口已接入中文版的原生主菜单沙盒、18 种原创植物、10 种原创僵尸、分件动画与按钮居中修复。保留原 800 × 600 画布和游戏内 UI，不跳转到另一套沙盒画面。

沙盒补字沿用同一套位图字库的偏旁，修复「焰」字过大、风格不一致和覆盖相邻文字的问题；保留原字号、基线和字距，不需要变更素材包。

## 首次游玩

1. 打开网址，网站自动下载约 53.7 MB 的完整游戏资源，无需选择、导入或解压文件。
2. 完成加载后点击「开始游戏」，从本体主菜单进入「沙盒模式」。
3. 浏览器允许时会记住资源，下次优先使用缓存；无痕或存储受限时自动重新下载。

网站托管项目当前整套素材，保留原有场景、角色分件、动画、字体、音乐与音效。打包时校验全部 3046 个文件，包括 40 个生成的子弹与技能特效素材，发布与运行时均校验包散列。素材与代码的权利说明分别保留，详见来源与许可页。

`classic.html` 保留之前的年度版 `main.pak` + `properties/` 导入流程和固定上游引擎。

本地网址和 GitHub 网址使用不同的存档空间。迁移进度：先在本地游戏的「工具 → 备份存档」导出，再在新网站的「工具 → 恢复存档」导入；阵型另用游戏内沙盒菜单导出/导入。不要清除网站数据。仅更换资源缓存不会清除存档。

## 构建与发布

启动页提供实时下载进度、自动重试、首次使用说明、网址复制和断网提示；没有素材选择框。下载失败不改动存档。存档读取超过 15 秒会停止启动并显示重试，不会以空进度覆盖已有存档；保存超时可手动备份。所有入口脚本、样式、模块与 WASM 使用同一构建版本号，避免混用新旧代码。手机横竖屏变化会重新计算画面尺寸并避开屏幕安全区域。

需要 Node.js 22+，无需安装 npm 依赖：

```sh
node scripts/build-site.mjs
node --test tests/pages.test.mjs
```

`web/` 是默认网页源码，构建复制到 `site/`；旧入口由 `wasm/shell.html` 生成到 `site/classic.html`。全部网页资源使用相对路径，支持 GitHub Pages 项目子目录。

推送 `main` 后由 `Publish PvZ website` 自动验证并发布 `site/`。仓库中的 `resource-bundle/` 保存逐字节分段的完整游戏包，构建校验后合并到 `site/resources/`，不缩图、不转码、不删素材；工作流不读取玩家存档、个人目录或密钥。更新包前运行 `node scripts/package-resources.mjs /path/to/local-resources.zip`，仅接受清单中匹配的文件与内容。

## 引擎源码和重建

- 上游基线：PvZ-Portable 0.2.3，`147ce06c5ad08ac7975c4cc20e90978c724be8c4`。
- `src/` 包含发布的完整修改源码：`Sandbox*` 及 Board / Plant / GameSelector / PlayerInfo / SaveGame / LawnApp 的集成点。
- `site/sandbox-engine/` 是对应的编译结果，散列见 `build.json`。已在发布前与本机已验证版本逐字节比对。
- 工具链：Emscripten 4.0.16、CMake 3.31.6、Ninja 1.11.1.4、libopenmpt 0.8.4。使用上游 `wasm/build-wasm.sh` 或在配置好 Emscripten 和 openmpt 的环境执行：

```sh
emcmake cmake -S . -B build-wasm -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_FIND_ROOT_PATH=/path/to/openmpt-prefix
cmake --build build-wasm
```

将 `build-wasm/pvz-portable.js` 与 `.wasm` 放入 `site/sandbox-engine/`，更新构建散列后验证。许可和署名入口保留在 `site/credits.html`；旧引擎文件与散列保持不变。

## 验证范围

自动测试覆盖真实 WASM 初始化及沙盒接口、完整素材包校验、GitHub 子路径引用、无素材选择步骤、首次自动下载/错误重试/缓存读取、缓存禁用和超时、存档路径保护与加载状态转换。

自动测试不是浏览器画面或通关验收；本次环境无可连接浏览器。部署后另外检查公网入口与 JS / WASM / 资源清单的实际 HTTP 响应及散列。
