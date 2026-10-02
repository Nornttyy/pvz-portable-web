import test from 'node:test';
import assert from 'node:assert/strict';
import {readFile} from 'node:fs/promises';
const read=path=>readFile(new URL('../'+path,import.meta.url),'utf8');
test('retired Zombatar has neither menu art, a hidden hit target, editor nor preload',async()=>{
 const source=await read('src/Lawn/Widget/GameSelector.cpp');
 assert.match(source,/AssignRenderGroupToTrack\("woodsign3", RENDER_GROUP_HIDDEN\)/);
 assert.match(source,/mZombatarButton = nullptr/);
 assert.doesNotMatch(source,/mZombatarButton->|AddWidget\(mZombatarButton\)|TrackButton\(mZombatarButton/);
 assert.doesNotMatch(source,/make_unique<ZombatarWidget>|mZombatarWidget->|Widget\(mZombatarWidget.get\(\)\)|BringToFront\(mZombatarWidget/);
 assert.doesNotMatch(source,/DelayLoad_Zombatar|SetImageOverride\("woodsign3"/);
 assert.match(source,/case GameSelector::GameSelector_Zombatar:\s*return;/);
 const entry=source.slice(source.indexOf('void GameSelector::ShowZombatarScreen()'),source.indexOf('void GameSelector::ShowAchievementsScreen()'));
 assert.doesNotMatch(entry,/ShowZombatarTOS|->Open\(/);
 assert.match(source,/AddWidget\(mAchievementsWidget.get\(\)\)/);
 assert.match(source,/mAchievementsWidget->mY = aNewY \+ mApp->mHeight/);
 assert.match(source,/AddWidget\(mChangeUserButton\)/);
 assert.match(source,/AddWidget\(mAlmanacButton\)/);
 assert.match(source,/AddWidget\(mSandboxButton\)/);
});
test('removing the avatar editor does not remove legacy profile records or zombie portraits',async()=>{
 const profile=await read('src/Lawn/System/PlayerInfo.cpp');
 assert.match(profile,/mZombatarData/);
 const zombies=await read('src/SandboxZombies.cpp'),almanac=await read('src/Lawn/Widget/AlmanacDialog.cpp');
 assert.match(zombies,/void DrawPortrait\(/);
 assert.match(almanac,/SandboxZombies::DrawPortrait\(/);
});
