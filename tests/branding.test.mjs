import test from 'node:test';
import assert from 'node:assert/strict';
import {readFile} from 'node:fs/promises';
const read=p=>readFile(new URL('../'+p,import.meta.url));
test('NB branding uses the same transparent badge on the web and inside the game',async()=>{
 const badge=await read('addons/images/nb-edition.png');
 assert.equal(badge.readUInt32BE(16),256);assert.equal(badge.readUInt32BE(20),100);assert.equal(badge[25],6);
 assert.deepEqual(badge,await read('site/branding/nb-edition.png'));
 const logo=await read('site/branding/original-logo.png');assert.equal(logo.readUInt32BE(16),700);assert.equal(logo.readUInt32BE(20),116);assert.equal(logo[25],6);
 const title=(await read('src/Lawn/Widget/TitleScreen.cpp')).toString();
 assert.match(title,/DrawImage\(IMAGE_PVZ_LOGO,/);assert.match(title,/GetImage\("\/addons\/images\/nb-edition.png"\)/);
 assert.match(title,/mTitleStateCounter <= 50/);assert.doesNotMatch(title,/sin\(.*mTitleAge/);
 const html=(await read('web/index.html')).toString(),css=(await read('web/game.css')).toString();
 assert.match(html,/<title>植物大战僵尸 NB版<\/title>/);assert.match(html,/branding\/original-logo.png/);assert.match(html,/branding\/nb-edition.png/);
 assert.match(css,/prefers-reduced-motion: reduce/);assert.doesNotMatch(css,/nb-peek[^;]*infinite/);
 const cmake=(await read('CMakeLists.txt')).toString();assert.match(cmake,/LINK_DEPENDS[\s\S]*addons\/images\/nb-edition.png/);
});
test('compact sandbox entry renders the small native font and shares its hitbox',async()=>{
 const selector=(await read('src/Lawn/Widget/GameSelector.cpp')).toString();
 const button=selector.slice(selector.indexOf('class SandboxMenuButton'),selector.indexOf('class SandboxMenuButton')+1800);
 assert.match(button,/SandboxDrawButton[\s\S]*mIsOver,\s*false/);
 assert.match(selector,/SandboxUIRules::MenuEntry/);
});
test('main menu reuses the original logo and badge without adding an interactive overlay',async()=>{
 const source=(await read('src/Lawn/Widget/GameSelector.cpp')).toString();
 const draw=source.slice(source.indexOf('void GameSelector::Draw(Graphics* g)'),source.indexOf('void GameSelector::DrawOverlay('));
 assert.match(draw,/mSelectorState == SELECTOR_IDLE && !mStartingGame && IMAGE_PVZ_LOGO/);
 assert.match(draw,/SandboxUIRules::MenuLogo/);assert.match(draw,/DrawImage\(IMAGE_PVZ_LOGO, logo.x, logo.y, logo.w, logo.h\)/);
 assert.match(draw,/GetImage\("\/addons\/images\/nb-edition.png"\)/);assert.match(draw,/SandboxUIRules::MenuLogoBadge/);
 assert.match(draw,/badge.x[^\n]*g->mTransX/);assert.match(draw,/badge.y[^\n]*g->mTransY/);
 assert.doesNotMatch(draw,/new.*Button|AddWidget|GetImage\([^\n]*original-logo/);
});
