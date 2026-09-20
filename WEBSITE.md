# GitHub Pages · 中文沙盒

网址：https://nornttyy.github.io/pvz-portable-web/

默认入口已接入当前中文版的原生主菜单沙盒、8 种原创植物、分件动画与按钮居中修复。保留原 800 × 600 画布和游戏内 UI，不跳转到另一套沙盒画面。

## 首次游玩

1. 点击「选择本机资源包」，选择本地中文版修复加载问题后的 `resources/local-resources.zip`，约 50.5 MB，无需解压。
2. 完成校验后点击「开始游戏」，从本体主菜单进入「沙盒模式」。
3. 浏览器允许时会记住资源，下次自动读取；无痕或存储受限时需要重新选择。

公开网站只托管代码、引擎、资源校验清单及现有生成样张，不上传或提供原版图片、字体、音乐包。选择的资源只在你的浏览器中读取。资源清单锁定已修复字体注释格式的版本，旧资源包会显示版本提示，不会进入有问题的加载过程。

`classic.html` 保留之前的年度版 `main.pak` + `properties/` 导入流程和固定上游引擎。

本地网址和 GitHub 网址使用不同的存档空间。迁移进度：先在本地游戏的「工具 → 备份存档」导出，再在新网站的「工具 → 恢复存档」导入；阵型另用游戏内沙盒菜单导出/导入。不要清除网站数据。仅更换资源缓存不会清除存档。

## 构建与发布

需要 Node.js 22+，无需安装 npm 依赖：

```sh
node scripts/build-site.mjs
node --test tests/pages.test.mjs
```

`web/` 是默认网页源码，构建复制到 `site/`；旧入口由 `wasm/shell.html` 生成到 `site/classic.html`。全部网页资源使用相对路径，支持 GitHub Pages 项目子目录。

推送 `main` 后由 `Publish PvZ website` 自动验证并发布 `site/`。工作流不会读取本地资源包、存档、个人目录或密钥。

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

自动测试覆盖真实 WASM 初始化及沙盒接口、构建散列、GitHub 子路径引用、原版资源不随站点发布、首次导入/错误重试/再次读取、缓存禁用和超时、存档路径保护与加载状态转换。

自动测试不是浏览器画面或通关验收；本次环境无可连接浏览器。部署后另外检查公网入口与 JS / WASM / 资源清单的实际 HTTP 响应及散列。
