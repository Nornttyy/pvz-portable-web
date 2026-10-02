import test from 'node:test';
import assert from 'node:assert/strict';
import {readFile,writeFile,mkdtemp} from 'node:fs/promises';
import {execFile} from 'node:child_process';
import {promisify} from 'node:util';
import {tmpdir} from 'node:os';
import {join} from 'node:path';
import {PLANTS} from '../web/sandbox-data.mjs';
const root=new URL('../',import.meta.url).pathname,run=promisify(execFile),read=p=>readFile(join(root,p),'utf8');
test('production coin plants preserve exact probabilities, wallet units, lifetime, cap and save state',async()=>{
 const source=await read('src/CoinPlants.cpp'),dir=await mkdtemp(join(tmpdir(),'pvz-coin-plants-')),binary=join(dir,'combat');
 await writeFile(join(dir,'coin-helpers.inc'),source.slice(source.indexOf('float Height('),source.indexOf('Sexy::Image* CoinImage(')));
 await writeFile(join(dir,'coin-production.inc'),source.slice(source.indexOf('bool ShooterSlot('),source.indexOf('bool DrawShot(')));
 await run(process.env.CXX||'c++',['-std=c++20','-Isrc','-I'+dir,'tests/coin-plants-native.cpp','-o',binary],{cwd:root});
 assert.match((await run(binary)).stdout,/exact odds and debit, clocks, zero funds, pending rolls, save restore, cap and one-hit orbit contacts passed/);
});
test('money ammunition is not collectible, area damage or a second peashooter replacement',async()=>{
 assert.equal(PLANTS.find(p=>p.id===530).base,50);assert.equal(PLANTS.find(p=>p.id===531).base,38);assert.equal(PLANTS.find(p=>p.id===500).base,0);
 const source=await read('src/CoinPlants.cpp'),plant=await read('src/Lawn/Plant.cpp'),shot=await read('src/Lawn/Projectile.cpp'),save=await read('src/Lawn/System/SaveGame.cpp');
 assert.doesNotMatch(source,/AddCoin\(|KillAllZombiesInRadius|DoSplashDamage|mSunMoney|WriteCurrentUserConfig/);
 assert.match(source,/AddCoins\(-Units\(kind\)\)/);assert.match(source,/s\.coins\[slot\]=0/);
 assert.match(shot,/if \(CoinPlants::Impact\(this,theZombie\)\) return;/);assert.match(shot,/if \(CoinPlants::DrawShot\(g,this\)\) return;/);
 assert.match(plant,/CoinPlants::DrawOrbit\(g,this,false\)/);assert.match(plant,/CoinPlants::DrawOrbit\(g,this,true\)/);
 assert.match(source,/OnBone\(g,head,"idle_mouth"/);assert.match(source,/OnBone\(g,head,"anim_face"/);
 assert.match(save,/SAVE4_CHUNK_COIN_PLANTS = 25/);assert.match(save,/CoinPlantRules::Valid\(p.state\)/);assert.match(save,/CoinPlants::Restore\(theBoard\)/);
 assert.match(await read('src/Sandbox.cpp'),/sandboxProfile->mCoins = adventureProfile->mCoins/);
 const chooser=await read('src/Lawn/Widget/SeedChooserScreen.cpp');
 assert.match(chooser,/SeedType aSeedType = SeedHitTest\(x, y\);\s*if \(aSeedType == SEED_NONE && !mBoard->mSeedBank->ContainsPoint/);
});
