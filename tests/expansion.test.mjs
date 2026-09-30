import test from 'node:test';
import assert from 'node:assert/strict';
import {readFile,mkdtemp,copyFile} from 'node:fs/promises';
import {execFile} from 'node:child_process';
import {promisify} from 'node:util';
import {join} from 'node:path';
import {tmpdir} from 'node:os';
import {fileURLToPath} from 'node:url';
import {createHash} from 'node:crypto';
import {ORIGINAL_PLANTS,RETIRED_PLANTS,ORIGINAL_ZOMBIES,ZOMBIES,validateLayout} from '../web/sandbox-data.mjs';
const run=promisify(execFile),root=fileURLToPath(new URL('../',import.meta.url));
const read=name=>readFile(join(root,name));
test('meme powers exercise real production combat and preserve native pixel shading',async()=>{
 const dir=await mkdtemp(join(tmpdir(),'pvz-meme-combat-')),binary=join(dir,'combat');
 for(const f of ['SandboxPlants.cpp','SandboxZombies.cpp','MemeCharacters.cpp'])await copyFile(join(root,'src',f),join(dir,f));
 await run(process.env.CXX||'c++',['-std=c++20','-Itests/combat-stubs','-Isrc',join(dir,'SandboxPlants.cpp'),join(dir,'SandboxZombies.cpp'),join(dir,'MemeCharacters.cpp'),'tests/meme-combat.cpp','-o',binary],{cwd:root});
 assert.match((await run(binary)).stdout,/Meme powers:.*passed/);
});
test('power token is not a plant or a save entry; only results round-trip',()=>{
 for(const type of [500,501,502,503,504])assert.equal(validateLayout({schema:1,map:0,plants:[{type,col:0,row:0}]}).plants[0].type,type);
 assert.throws(()=>validateLayout({schema:1,map:0,plants:[{type:180,col:0,row:0}]}));
});
test('release vocal is a dedicated non-looping native SFX, with overlap and volume guards',async()=>{
 const app=(await read('src/LawnApp.cpp')).toString(),foley=(await read('src/PvzpLib/PvzpFoley.cpp')).toString();
 const start=app.indexOf('void LawnApp::PlayRageRelease()'),body=app.slice(start,app.indexOf('std::string LawnApp::GetStageString',start));
 assert.match(body,/mMuteSoundsForCutscene/);assert.match(body,/IsFoleyPlaying\(FOLEY_RAGE_RELEASE\)/);assert.match(body,/PlayFoleyPitch\(FOLEY_RAGE_RELEASE, 7\.0f\)/);
 assert.match(foley,/FOLEY_RAGE_RELEASE,.*SOUND_CRAZYDAVECRAZY.*mFoleyFlags = 0U/);
 assert.match(foley,/if \(theFoleyType == FOLEY_RAGE_RELEASE\)\s*aSoundInstance->SetVolume\(0\.60\)/);
 const mixer=(await read('src/SexyAppFramework/sound/SDLSoundInstance.cpp')).toString();assert.match(mixer,/mBaseVolume \* mVolume \* mSoundManagerP->mMasterVolume/);
});
test('persistent native sidebar fits both rosters and keeps the lawn in original units',async()=>{
 const dir=await mkdtemp(join(tmpdir(),'pvz-sidebar-')),binary=join(dir,'sidebar');
 await run(process.env.CXX||'c++',['-std=c++20','-Isrc','tests/sidebar-layout.cpp','-o',binary],{cwd:root});
 assert.match((await run(binary)).stdout,/99 grass\/pool cell mappings passed/);
 const ui=(await read('src/SandboxUI.cpp')).toString();assert.match(ui,/class SandboxOverlay final : public Widget/);assert.doesNotMatch(ui,/showZombies|OpenPanel\(1\)|OpenPanel\(2\)/);
});
test('actual projectile/effect draw code preserves bone and world coordinate contracts',async()=>{
 const dir=await mkdtemp(join(tmpdir(),'pvz-visual-contracts-')),binary=join(dir,'visual');
 for(const f of ['SandboxPlants.cpp','SandboxZombies.cpp','SandboxArt.cpp','MemeCharacters.cpp'])await copyFile(join(root,'src',f),join(dir,f));
 await run(process.env.CXX||'c++',['-std=c++20','-Itests/combat-stubs','-Isrc',...['SandboxPlants.cpp','SandboxZombies.cpp','SandboxArt.cpp','MemeCharacters.cpp'].map(f=>join(dir,f)),'tests/visual-coordinates.cpp','-o',binary],{cwd:root});
 assert.match((await run(binary)).stdout,/Visual coordinate contracts passed/);
});
test('native projectile integration retains splats, centered scaling and fire attachments',async()=>{
 const render=(await read('src/Lawn/Projectile.cpp')).toString();
 assert.match(render,/MemeShooterRules::OverlapsY/);
 assert.match(render,/SandboxPlants::UpdateShot\(this\);\s*if \(mDead\) return;/);
 const impact=render.slice(render.indexOf('void Projectile::DoImpact('),render.indexOf('void Projectile::Draw('));
 assert.match(impact,/if \(SandboxPlants::UsesCustomShotArt\(this\)\) \{ Die\(\); return; \}/);
 assert.doesNotMatch(impact,/if \(SandboxPlants::HasShot\(this\)\)/);
 for(const effect of ['PARTICLE_PEA_SPLAT','PARTICLE_SNOWPEA_SPLAT','REANIM_JALAPENO_FIRE'])assert.ok(impact.includes(effect));
 const draw=render.slice(render.indexOf('void Projectile::Draw('),render.indexOf('void Projectile::DrawShadow('));
 assert.match(draw,/if \(SandboxPlants::DrawShot\(g,this\)\) return;/);
 for(const native of ['IMAGE_PROJECTILEPEA','IMAGE_PROJECTILESNOWPEA','AttachmentDraw','SandboxPlants::ShotScale(this)','SandboxVisualRules::NativePeaOffset'])assert.ok(draw.includes(native));
 const source=(await read('src/Lawn/Plant.cpp')).toString();
 const plant=source.slice(source.indexOf('void Plant::Fire('),source.indexOf('Zombie* Plant::FindTargetZombie('));
 assert.ok(plant.indexOf('aProjectile->ConvertToFireball(mPlantCol)')>plant.indexOf('SandboxPlants::OnFired(this,aProjectile,theTargetZombie)'),'fire attaches only after muzzle correction');
});
test('five mechanic characters replace infusion; old results migrate without losing plants',async()=>{
 assert.equal(ORIGINAL_PLANTS.length,5);assert.deepEqual(ORIGINAL_PLANTS.map(p=>p.id),[500,501,502,503,504]);assert.deepEqual(ORIGINAL_ZOMBIES,[]);assert.equal(ZOMBIES.length,23);
 const bases=[0,7,7,18,18,40,40,7,0,3,1,8,0,0,0,0,0,0,3,0];
 for(let id=100;id<120;++id)assert.equal(validateLayout({schema:1,map:0,plants:[{type:id,col:0,row:0}]}).plants[0].type,bases[id-100]);
 for(const p of RETIRED_PLANTS){
  if(p.base===11){assert.throws(()=>validateLayout({schema:1,map:0,plants:[{type:p.id,col:0,row:0}]}));continue;}
  const water=[16,19,24,43].includes(p.base),plants=[{type:p.id,col:0,row:water?2:0}];
  if(p.base===35)plants.push({type:8,col:0,row:0});
  assert.ok(validateLayout({schema:1,map:water?1:0,plants}).plants.some(n=>n.type===p.base));
 }
 for(const id of [144,180,181,182,200,211])assert.throws(()=>validateLayout({schema:1,map:0,plants:[{type:id,col:0,row:0}]}));
 const m=JSON.parse(await read('site/resource-manifest.json'));assert.equal(m.totalFiles,2912);assert.equal(m.files.filter(f=>f.path.startsWith('images/sandbox/')).length,0);
 const code=(await read('src/SandboxPlants.cpp')).toString();assert.doesNotMatch(code,/NutMouthMatrix|dandelion|poison|echo-lily|Rig\(/);
 assert.doesNotMatch((await read('src/SandboxUI.cpp')).toString(),/Button\(g,CustomFilter/);
});
test('adventure replaces native cards, preserving optional saves and combat ticks',async()=>{
 const adventure=(await read('src/MemeAdventure.cpp')).toString(),save=(await read('src/Lawn/System/SaveGame.cpp')).toString();
 assert.doesNotMatch(adventure,/TakeSunMoney|SandboxDrawButton|选卡后直接种植/);assert.match(adventure,/SandboxPlants::Tick\(b\)/);
 assert.match(save,/SAVE4_CHUNK_MEME_POWERS = 21/);assert.match(save,/MemeAdventure::Restore\(theBoard\)/);
 assert.match(save,/SAVE4_CHUNK_MEME_PROJECTILES = 22/);assert.match(save,/MemeAdventure::LoadShots\(save.shots\)/);
 assert.match(adventure,/CURSOR_TYPE_NORMAL/);assert.match(adventure,/MemeCharacters::Assign\(p,d->id\)/);
 assert.match(adventure,/GetPlantDefinition\(p->mSeedType\).mLaunchRate/);
 const board=(await read('src/Lawn/Board.cpp')).toString();assert.match(board,/MemeAdventure::OnPlanted\(aPlant\)/);
 const plants=(await read('src/Lawn/Plant.cpp')).toString();
 assert.match(plants,/MemeAdventure::Replacement\(int\(theSeedType\), int\(theImitaterType\)\)\) return 300/);
 assert.match(plants,/MemeCharacters::Is\(this\) && !MemeCharacters::Producing\(this\)/);
 assert.match(adventure,/for\(auto\* p:b->mPlants\)OnPlanted\(p\)/);
 assert.doesNotMatch((await read('src/Lawn/SeedPacket.cpp')).toString(),/MemeAdventure::DrawCardName/);
 assert.doesNotMatch(adventure,/void DrawCardName/);
 const characters=(await read('src/MemeCharacters.cpp')).toString();
 assert.doesNotMatch(characters,/NutJaw|d->shortName/);
 assert.doesNotMatch((await read('src/SandboxArt.cpp')).toString(),/NutJaw|for\(int tooth/);
 assert.doesNotMatch((await read('src/SandboxUI.cpp')).toString(),/PvzpDrawString\(g,d.name|Say\(fused->name\)/);
 assert.match((await read('src/PvzpLib/PvzpStringFile.cpp')).toString(),/MemeAdventure::Translate\(theName,anItr->second\)/);
 const special=plants.slice(plants.indexOf('void Plant::DoSpecial()'),plants.indexOf('void Plant::ImitaterMorph()'));
 assert.doesNotMatch(special,/void Plant::DoSpecial\(\)\s*\{\s*SandboxPlants::OneShot/);
 assert.match(special,/BurnRow\(mRow\);\s*mBoard->mIceTimer\[mRow\] = 20;\s*SandboxPlants::OneShot\(this\)/);
 const squash=plants.slice(plants.indexOf('void Plant::DoSquashDamage()'),plants.indexOf('Zombie* Plant::FindSquashTarget()'));
 assert.ok(squash.indexOf('SandboxPlants::OneShot')>squash.indexOf('TakeDamage(1800'),'bonus effect cannot push a victim out of the original attack');
 assert.match(plants,/const int inheritedPower=SandboxPlants::Power\(this\);/);
 assert.match(save,/MemeAdventure::Reset\(\)/);assert.match(save,/count>1024/);
 assert.doesNotMatch(adventure,/mNumPackets\s*=|mSunMoney\s*=|mLevel\s*=/);
});
test('deferred disposal cannot reset a newly loaded board',async()=>{
 const board=(await read('src/Lawn/Board.cpp')).toString(),app=(await read('src/LawnApp.cpp')).toString();
 assert.match(board,/Board::~Board\(\) = default;/);
 const kill=app.slice(app.indexOf('void LawnApp::KillBoard()'),app.indexOf('bool LawnApp::CanPauseNow()'));
 for(const state of ['MemeAdventure','SandboxPlants','SandboxZombies'])assert.ok(kill.indexOf(state+'::Reset()')<kill.indexOf('SafeDeleteWidget(mBoard)'));
});
