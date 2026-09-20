# 独立 GitHub Pages 网站

网址：https://nornttyy.github.io/pvz-portable-web/

这是 PvZ-Portable 0.2.3 的独立静态部署，不涉及《海边寿司店》的目录、仓库、网址或存档。

## 资源要求

引擎需要用户自行导入正版年度版 `main.pak` 和 `properties/`。本站不附带这套资源，也不上传用户选择的文件。手机可使用包含两项内容的 ZIP。存档保存于浏览器，换设备请先导出。

`art-preview.html` 展示最新生成的静态分件图，不包含骨骼动画，不是完整替代素材包，不参与引擎加载。

## 构建与自动发布

无需 npm 依赖，Node.js 22 或以上即可：

```sh
node scripts/build-site.mjs
node --test tests/pages.test.mjs
python3 -m http.server 8793 --bind 127.0.0.1 --directory site
```

推送到本仓库 `main` 后，`Publish PvZ website` 工作流会自动构建中文入口、验证引擎完整性与静态链接，再发布 `site/`。所有资源 URL 相对当前目录，适配 GitHub Pages 项目子路径。原上游多平台编译任务改为手动触发，避免每次改网页触发整套原生编译。

## 上游与修改

- 上游：https://github.com/wszqkzqk/PvZ-Portable
- 版本：0.2.3 / `147ce06c5ad08ac7975c4cc20e90978c724be8c4`
- `src/`、`wasm/shell.html` 和构建脚本保留对应上游源代码。重新编译引擎见原 README 与 `wasm/build-wasm.sh`。
- `site/pvz-portable.js`、`.wasm`、`.html` 直接来自上游版本发行包，未修改；散列见 `site/upstream-release.json`。
- 中文入口由上游 `wasm/shell.html` 派生，只修改页面文案、资源引用和状态提示，不修改 C++ 或 WASM。
- JSZip 固定为本地 3.10.1，中文入口不依赖外部 CDN。
- 许可与署名保留，网站提供源代码与许可证入口。
- 新增文件日期：2026-09-20。

## 检查范围

自动测试覆盖静态资源、链接、脚本语法、WASM 编译、发行文件散列、入口状态提示和无资源自动启动保护。不代表完整游戏已实机通关；当前没有导入用户正版资源，也没有可连接的浏览器进行视觉与交互验收。

网站为公开 Pages 页面；noindex 仅用于减少搜索索引，不提供访问控制。
