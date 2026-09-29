# 网梗之力：素材沿用记录

以下全文为已退役融合版本的历史素材和测试记录。当前版本使用原生卡槽中的替换植物及新增植物，融合入口已撤下。最新规则见 [WEBSITE.md](../../WEBSITE.md)，本批素材记录见 [abstract/README.md](../abstract/README.md)，当前实机检查脚本为 `scripts/qa-abstract.mjs` 和 `scripts/qa-adventure-replacements.mjs`。

本轮不使用图片生成，不加新的角色 PNG，不改动资源包。力量卡 ID 180 不是植物；0、1、3 与力量合成后分别变为 120、121、122。旧内容和存档格式保持兼容。

## 原版素材复用

- 豌豆：`PeaShooter_Head.png`、`PeaShooter_mouth.png`、两张 blink。身体、叶片、茎和完整原版骨骼保持原样。
- 表情：`PeaShooter_eyebrow.png`。按 `PeaShooter.reanim` 第 29 帧记录的面部/眉毛相对变换，挂到当前头部矩阵；升温后渐显，恢复时消失。
- 向日葵：`SunFlower_head.png` 和两张 blink 运行时换色；花瓣和茎叶保留原色。
- 坚果：`Wallnut_body.png`、`Wallnut_cracked1.png`、`Wallnut_cracked2.png` 和两张 blink，保留原生损伤/碎屑状态。仅绘制期间覆盖图像，结束立即恢复，避免把调色图误认为新的受损切换。
- 特效：`puff_3.png`、`puff_4.png`，用于融合瞬间、热气与推退反馈。子弹、火炬转化、阳光和音效均复用原版。
- 力量卡：原版辣椒作为红温符号，卡面底部标注「力量」，没有单独新增辣椒植物。

`SandboxArt::WarmNative` 将原图复制后分 24 档调色并缓存。原始尺寸、透明通道、黑色轮廓、白色眼睛及明暗保持；叶片不跟着全身变红。共享原图从不写入，普通植物不会被旁边的红温植物染色。

`SandboxMemeRules.h` 定义统一的升温/爆发/恢复状态机，暂停、睡眠、被提起或压扁时不偷跑。运动幅度限制在原版尺寸附近，不添加全屏震动或独立弹窗。

## 验证和复现

```sh
node scripts/build-site.mjs
node --test tests/pages.test.mjs
PVZ_PLAYWRIGHT=/path/to/playwright-core/index.mjs \
PVZ_QA_URL=http://127.0.0.1:8097/ \
node scripts/qa-meme-powers.mjs
```

浏览器脚本使用全新的隔离 Chrome profile，实际操作原生菜单、力量卡和三种融合，检查草地/泳池、南瓜底座、阵型存取、爆发/冷却、坚果两级破损、横屏和竖屏适配。截图是实际 WASM 运行画面，不是概念图。自动测试另覆盖子弹/阳光容量、无目标冷却、不可击退目标、原位融合保留血量、临时图层恢复和共享像素不变。

当前仍是沙盒试玩，未宣称完成冒险关卡投放或经济平衡。

68 项自动测试通过；第二轮实机截图：[力量入口](qa/power-entry.png)、[红温连发](qa/heat-burst.png)、[坚果严重破损](qa/nut-damage-2.png)、[手机横屏](qa/phone-powers.png)。[实机阶段记录](qa/report.json) 包含实际血量、热量、阶段与浏览器错误检查。
