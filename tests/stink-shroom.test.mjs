import test from 'node:test';
import assert from 'node:assert/strict';
import {readFile,writeFile,mkdtemp} from 'node:fs/promises';
import {execFile} from 'node:child_process';
import {promisify} from 'node:util';
import {tmpdir} from 'node:os';
import {join} from 'node:path';
import {PLANTS} from '../web/sandbox-data.mjs';
const root=new URL('../',import.meta.url).pathname,run=promisify(execFile),read=p=>readFile(join(root,p),'utf8');
test('stink production hit/aura/status functions pass exact probability, timing and save boundaries',async()=>{
 const source=await read('src/StinkShroom.cpp'),dir=await mkdtemp(join(tmpdir(),'pvz-stink-')),binary=join(dir,'combat');
 await writeFile(join(dir,'stink-production.inc'),source.slice(source.indexOf('namespace StinkShroom {'),source.indexOf('void DrawEffects('))+'\n}\n');
 await run(process.env.CXX||'c++',['-std=c++20','-Isrc','-I'+dir,'tests/stink-shroom-native.cpp','-o',binary],{cwd:root});
 assert.match((await run(binary)).stdout,/exact 50\/5\/10 rolls, 0.5-second stun, smooth push\/flee, 20-second exposure, 70-percent work and saved statuses passed/);
});
test('stink replaces the native card with brown penetrating fume; only production and firing are slowed',async()=>{
 assert.equal(PLANTS[10].id,527);assert.equal(PLANTS[10].name,'喷粪菇');
 const p=await read('src/Lawn/Plant.cpp'),z=await read('src/Lawn/Zombie.cpp'),save=await read('src/Lawn/System/SaveGame.cpp');
 assert.ok(p.indexOf('StinkShroom::Hit(this,aZombie)')>p.indexOf('aZombie->TakeDamage(aDamage, theDamageFlags)'));
 assert.match(p,/cloud->OverrideColor\(nullptr,Color\(125,93,44\)\)/);
 for(const fn of ['UpdateProductionPlant','UpdateShooter'])assert.ok(p.includes('void Plant::'+fn+'()\n{\n\tif (!StinkShroom::WorkTick(this)) return;'));
 assert.match(p,/MemeCharacters::StinkShroom \? StinkShroomRules::Interval : mLaunchRate - Sexy::Rand\(15\)/);
 assert.ok(z.indexOf('if (StinkShroom::UpdateZombie(this)) return;')<z.indexOf('if (SandboxZombies::UpdateInteraction(this)) return;'));
 assert.match(z,/if \(StinkShroom::Fleeing\(this\)\) return true;/);
 assert.match(save,/SAVE4_CHUNK_STINK_STATUS = 24/);assert.match(save,/StinkShroom::Restore\(theBoard\)/);assert.match(save,/WriteChunkV4\(aPayload, SAVE4_CHUNK_STINK_STATUS, theBoard\)/);
});
