import test from 'node:test';
import assert from 'node:assert/strict';
import {readFile,writeFile,mkdtemp} from 'node:fs/promises';
import {execFile} from 'node:child_process';
import {promisify} from 'node:util';
import {tmpdir} from 'node:os';
import {join} from 'node:path';
import {PLANTS} from '../web/sandbox-data.mjs';
const root=new URL('../',import.meta.url).pathname,run=promisify(execFile),read=p=>readFile(join(root,p),'utf8');
test('nuclear shroom production combat: 3x3 holes, five green pulses, pauses and native saved clocks',async()=>{
 const source=await read('src/NukeShroom.cpp'),folder=await mkdtemp(join(tmpdir(),'pvz-nuke-')),binary=join(folder,'combat');
 await writeFile(join(folder,'nuke-production.inc'),source.slice(source.indexOf('namespace NukeShroom {'),source.indexOf('bool DrawCrater('))+'\n}\n');
 await run(process.env.CXX||'c++',['-std=c++20','-Isrc','-I'+folder,'tests/nuke-shroom-native.cpp','-o',binary],{cwd:root});
 assert.match((await run(binary)).stdout,/nine cells at every board position, five timed pulses, native exclusions, saves and green art passed/);
});
test('nuclear replacement retains native sleep and portable crater serialization',async()=>{
 assert.equal(PLANTS[15].id,526);assert.equal(PLANTS[15].name,'核爆菇');
 const plant=await read('src/Lawn/Plant.cpp');
 assert.match(plant,/case SeedType::SEED_DOOMSHROOM:\s*\{?\s*if \(NukeShroom::Detonate\(this\)\) break;/);
 const nocturnal=plant.slice(plant.indexOf('bool Plant::IsNocturnal'),plant.indexOf('bool Plant::IsAquatic'));assert.ok(nocturnal.includes('SEED_DOOMSHROOM'));
 const save=await read('src/Lawn/System/SaveGame.cpp');
 for(const field of ['mGridItemState','mGridItemCounter','mSunCount','mTransparentCounter','mPosX','mPosY'])assert.ok(save.includes('theItem.'+field),field);
 assert.match(await read('src/Lawn/Board.cpp'),/NukeShroom::UpdateCrater\(aGridItem\)/);
 assert.match(await read('src/Lawn/GridItem.cpp'),/if \(NukeShroom::DrawCrater\(g,\s*this\)\) return;/);
 const body=await read('src/SandboxPlants.cpp');assert.match(body,/DrawNukePreview/);assert.match(body,/SandboxArt::DrawNukeEnergy/);
});
