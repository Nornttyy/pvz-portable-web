import test from 'node:test';
import assert from 'node:assert/strict';
import {readFile,mkdtemp} from 'node:fs/promises';
import {execFile} from 'node:child_process';
import {promisify} from 'node:util';
import {tmpdir} from 'node:os';
import {join} from 'node:path';
const read=p=>readFile(new URL('../'+p,import.meta.url),'utf8'),run=promisify(execFile);
async function entries(path,descriptionIndex){
 const source=await read(path),block=source.match(/Definitions\{\{([\s\S]*?)\}\};/)[1];
 return block.split('\n').filter(s=>s.trim().startsWith('{')).map(s=>{
  const strings=[...s.matchAll(/"(?:\\.|[^"\\])*"/g)].map(m=>JSON.parse(m[0]));return {name:strings[0],description:strings[descriptionIndex]};
 });
}
test('all twenty originals separate explicit combat statistics from surreal flavour without renaming',async()=>{
 const plants=await entries('src/MemeCharacters.h',4),zombies=await entries('src/SandboxZombies.h',1);
 assert.deepEqual(plants.map(p=>p.name),['红温豌豆','反咬坚果','射手豌豆','缩头乌葵','双----------双发射手','加特林射手','小·坚果','仙人的掌','真·小喷菇','核爆菇','喷粪菇','冰爆辣椒']);
 assert.deepEqual(zombies.map(p=>p.name),['路易十六','跑路僵尸','雪糕桶包裹我','巨人小鬼','智斗僵尸','路障智斗僵尸','绿路障僵尸','路障叠叠高僵尸']);
 for(const entry of [...plants,...zombies]){
  const paragraphs=entry.description.split('\n\n');assert.equal(paragraphs.length,2,entry.name);
  assert.match(paragraphs[0],/^生命\d+/);
  assert.ok(paragraphs[0].includes('\n'),entry.name+' needs separate stat rows');
  assert.ok(paragraphs[0].split('\n').length<=6,entry.name+' stats must leave room for flavour');
  assert.match(paragraphs[1],/^\{KEYWORD\}/); // Native ink colour separates fiction from real stats.
  assert.ok([...paragraphs[1]].length>=35,entry.name+' needs character flavour');
  assert.ok([...entry.description.replace(/\{[^}]+\}/g,'')].length<=240,entry.name+' copy must stay compact');
 }
 assert.match(plants[3].description,/您种的向日葵已离职/);
 assert.match(zombies[4].description,/翻身换行60% \/ 前飞2格30%/);
 // Almanac prose must not leak into seed selection hints or battle overlays.
 const sandbox=await read('src/SandboxPlants.h');assert.ok(!sandbox.includes('\\n\\n'));
});
test('displayed health, armor, attack timing and probabilities agree with live native constants',async()=>{
 const folder=await mkdtemp(join(tmpdir(),'pvz-almanac-stats-')),binary=join(folder,'stats');
 await run(process.env.CXX||'c++',['-std=c++20','-Isrc','tests/almanac-stats-native.cpp','-o',binary],{cwd:new URL('../',import.meta.url).pathname});
 assert.match((await run(binary)).stdout,/Almanac numeric statistics match native constants/);
 const almanac=await read('src/Lawn/Widget/AlmanacDialog.cpp');
 assert.match(almanac,/std::format\("\{:g\} 秒", Plant::GetRefreshTime\(mSelectedSeed, SEED_NONE\) \/ 100\.0\)/);
 assert.match(almanac,/MemeAdventure::Replacement\(int\(mSelectedSeed\)\) \? "种植冷却" : "\[WAIT_TIME\]"/);
 const projectile=await read('src/Lawn/Projectile.cpp'),plant=await read('src/Lawn/Plant.cpp'),combat=await read('src/MemeCharacters.cpp');
 assert.match(projectile,/PROJECTILE_PEA, .mImageRow = 0, .mDamage = 20/);
 assert.match(plant,/mPlantHealth = 300;/);assert.match(plant,/mPlantHealth = 4000;/);
 assert.match(combat,/s\.timer=300;s\.pulse=50/);assert.match(combat,/TakeDamage\(80,0\)/);
 assert.match(combat,/shotStyles\[shot\]=Sexy::Rand\(100\)<20\?CriticalPalmProjectile:PalmProjectile/);
});
test('almanac prose is completely covered by native and full supplemental Chinese glyphs',async()=>{
 const all=[...await entries('src/MemeCharacters.h',4),...await entries('src/SandboxZombies.h',1)];
 const {stdout:font}=await run('unzip',['-p','site/resources/game-b6e781efbe1f.zip','data/BrianneTod12.txt'],{cwd:new URL('../',import.meta.url).pathname,maxBuffer:1048576});
 const list=font.match(/Define CharList0\s*\(([\s\S]*?)\);/)[1];
 const available=new Set([...list.matchAll(/'((?:\\.|[^'])*)'/g)].map(m=>m[1]));
 for(const match of (await read('src/ExactGlyphPixels.h')).matchAll(/\{U'([^']+)'/g))available.add(match[1]);
 for(const match of (await read('src/SandboxFonts.cpp')).matchAll(/Supplement\(font,U'([^']+)'/g))available.add(match[1]);
 available.add('焰');
 for(const entry of all){const missing=[...new Set([...entry.name+entry.description].filter(c=>/\p{Script=Han}/u.test(c)&&!available.has(c)))];assert.deepEqual(missing,[],entry.name+' missing glyphs');}
});
