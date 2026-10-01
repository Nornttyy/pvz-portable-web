import test from 'node:test';
import assert from 'node:assert/strict';
import {readFile} from 'node:fs/promises';
import {PLANTS,validateLayout,requiresStacking} from '../web/sandbox-data.mjs';
const read=p=>readFile(new URL('../'+p,import.meta.url),'utf8');
const puff=(col=2,row=2)=>({type:525,col,row});
test('tiny puff replaces its native card; five fit naturally, but six never fit',()=>{
 assert.equal(PLANTS[8].id,525);assert.equal(PLANTS[8].name,'真·小喷菇');
 for(let n=1;n<=5;++n){const plants=Array.from({length:n},()=>puff());assert.equal(requiresStacking(plants),false);assert.equal(validateLayout({schema:1,map:0,plants}).plants.length,n);}
 for(const stacked of [false,true])assert.throws(()=>validateLayout({schema:1,map:0,stacked,plants:Array.from({length:6},()=>puff())}),/最多5只/);
 assert.throws(()=>validateLayout({schema:1,map:0,plants:[puff(),{type:500,col:2,row:2}]}),/冲突/);
 assert.throws(()=>validateLayout({schema:1,map:1,plants:[puff()]}),/睡莲/);
 const plants=[...Array.from({length:5},()=>puff()),{type:16,col:2,row:2}];
 assert.equal(requiresStacking(plants),false);assert.equal(validateLayout({schema:1,map:1,plants}).plants.length,6);
});
test('native stacking stays behind terrain checks; nocturnal and visual particle contracts are preserved',async()=>{
 const board=await read('src/Lawn/Board.cpp'),placement=board.slice(board.indexOf('PlantingReason Board::CanPlantAt('),board.indexOf('void Board::UpdateCursor('));
 const cap=placement.indexOf('MemeCharacters::PuffCount');
 for(const guard of ['GetCraterAt','IsIceAt','PLANTING_NOT_ON_WATER','PLANTING_NEEDS_POT'])assert.ok(placement.indexOf(guard)<cap);
 assert.match(placement,/PuffCount\(this, theGridX, theGridY\) >= TinyPuffRules::Limit/);
 const plant=await read('src/Lawn/Plant.cpp'),nocturnal=plant.slice(plant.indexOf('bool Plant::IsNocturnal'),plant.indexOf('bool Plant::IsAquatic'));
 assert.ok(nocturnal.includes('SEED_PUFFSHROOM'));assert.ok(!nocturnal.includes('MemeAdventure::Replacement'));
 assert.match(plant,/PuffMuzzle\(this,x,y\)/);assert.match(plant,/puff->OverrideScale\(nullptr,TinyPuffRules::Scale\)/);
 const projectile=await read('src/Lawn/Projectile.cpp');assert.match(projectile,/AttachmentOverrideScale\(mAttachmentID,TinyPuffRules::Scale\)/);
});
