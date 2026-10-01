import test from 'node:test';
import assert from 'node:assert/strict';
import {readFile} from 'node:fs/promises';
import {execFile} from 'node:child_process';
import {promisify} from 'node:util';
const read=p=>readFile(new URL('../'+p,import.meta.url),'utf8'),run=promisify(execFile);
async function entries(path,descriptionIndex){
 const source=await read(path),block=source.match(/Definitions\{\{([\s\S]*?)\}\};/)[1];
 return block.split('\n').filter(s=>s.trim().startsWith('{')).map(s=>{
  const strings=[...s.matchAll(/"(?:\\.|[^"\\])*"/g)].map(m=>JSON.parse(m[0]));return {name:strings[0],description:strings[descriptionIndex]};
 });
}
test('all sixteen almanac originals keep their names and receive compact ability plus absurd flavour paragraphs',async()=>{
 const plants=await entries('src/MemeCharacters.h',4),zombies=await entries('src/SandboxZombies.h',1);
 assert.deepEqual(plants.map(p=>p.name),['红温豌豆','反咬坚果','射手豌豆','缩头乌葵','双----------双发射手','加特林射手','小·坚果','仙人的掌']);
 assert.deepEqual(zombies.map(p=>p.name),['路易十六','跑路僵尸','雪糕桶包裹我','巨人小鬼','智斗僵尸','路障智斗僵尸','绿路障僵尸','路障叠叠高僵尸']);
 for(const entry of [...plants,...zombies]){
  const paragraphs=entry.description.split('\n\n');assert.equal(paragraphs.length,2,entry.name);
  assert.ok([...paragraphs[1]].length>=25,entry.name+' needs character flavour');
  assert.ok([...entry.description].length<=95,entry.name+' must not overflow the native card');
 }
 assert.match(plants[3].description,/我负责给钱，又没说负责送命/);
 assert.match(zombies[4].description,/60%换行、30%前飞两格/);
 // Almanac prose must not leak into seed selection hints or battle overlays.
 const sandbox=await read('src/SandboxPlants.h');assert.ok(!sandbox.includes('\\n\\n'));
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
