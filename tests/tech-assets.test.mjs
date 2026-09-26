import test from 'node:test';
import assert from 'node:assert/strict';
import {readFile,readdir,stat} from 'node:fs/promises';
import {fileURLToPath} from 'node:url';
import {join} from 'node:path';

const root=fileURLToPath(new URL('../',import.meta.url));
const readJson=async file=>JSON.parse(await readFile(join(root,file),'utf8'));

async function png(file){
 const bytes=await readFile(file);
 assert.deepEqual([...bytes.subarray(0,8)],[137,80,78,71,13,10,26,10],file);
 return {width:bytes.readUInt32BE(16),height:bytes.readUInt32BE(20),colorType:bytes[25]};
}

test('every declared Technology Garden source part is a real transparent PNG, never a full atlas placeholder',async()=>{
 const plant=await readJson('art/tech/plant-parts.json');
 const zombie=(await readJson('art/tech/zombie-parts.json')).parts;
 // 10 complete enemy rigs, ten torn upper-arm stages, armor states and all
 // sixty independent leg bones are present as real extracted parts.
 assert.ok(plant.length>=160);assert.ok(zombie.length>=220);
 for(const part of [...plant,...zombie]){
  const file=join(root,'art/tech',part.file);
  const info=await png(file);
  assert.equal(info.width,part.width,file);assert.equal(info.height,part.height,file);
  assert.ok(info.colorType===4||info.colorType===6,`no alpha channel: ${file}`);
  assert.ok((await stat(file)).size>200,file);
 }
});

test('runtime rig pieces use native bone canvases so scale and anchors cannot regress',async()=>{
 const runtime=await readJson('art/tech/runtime-parts.json');
 assert.equal(new Set(runtime.map(part=>part.file)).size,runtime.length);
 assert.ok(runtime.length>=200);
 for(const part of runtime){
  const file=join(root,'art/tech/runtime',part.file),info=await png(file);
  assert.equal(info.width,part.width,file);assert.equal(info.height,part.height,file);
  assert.ok(info.colorType===4||info.colorType===6,`no alpha channel: ${file}`);
 }
 const byName=new Map(runtime.map(part=>[part.file,part]));
 for(const slug of ['pulse-pod','magnet-maw','amber-gourd','scout-bloom','prism-reed']){
  assert.deepEqual([byName.get(`${slug}-head.png`).width,byName.get(`${slug}-head.png`).height],[70,65]);
  assert.deepEqual([byName.get(`${slug}-mouth.png`).width,byName.get(`${slug}-mouth.png`).height],[35,49]);
 }
 for(const slug of ['leak-pack','weld-shield','turbine-boot','cable-hook','wrench-tech','bolt-thrower','hover-drone','jammer-aerial','magnet-salvager','holo-decoy']){
  assert.deepEqual([byName.get(`${slug}-head.png`).width,byName.get(`${slug}-head.png`).height],[53,48]);
  assert.deepEqual([byName.get(`${slug}-body.png`).width,byName.get(`${slug}-body.png`).height],[53,63]);
  assert.deepEqual([byName.get(`${slug}-hand.png`).width,byName.get(`${slug}-hand.png`).height],[25,27]);
  assert.deepEqual([byName.get(`${slug}-outer-upper-damaged.png`).width,byName.get(`${slug}-outer-upper-damaged.png`).height],[17,35]);
  assert.deepEqual([byName.get(`${slug}-inner-leg-upper.png`).width,byName.get(`${slug}-inner-leg-upper.png`).height],[15,26]);
  assert.deepEqual([byName.get(`${slug}-inner-leg-lower.png`).width,byName.get(`${slug}-inner-leg-lower.png`).height],[32,36]);
  assert.deepEqual([byName.get(`${slug}-inner-leg-foot.png`).width,byName.get(`${slug}-inner-leg-foot.png`).height],[27,17]);
  assert.deepEqual([byName.get(`${slug}-outer-leg-upper.png`).width,byName.get(`${slug}-outer-leg-upper.png`).height],[21,39]);
  assert.deepEqual([byName.get(`${slug}-outer-leg-lower.png`).width,byName.get(`${slug}-outer-leg-lower.png`).height],[24,30]);
  assert.deepEqual([byName.get(`${slug}-outer-leg-foot.png`).width,byName.get(`${slug}-outer-leg-foot.png`).height],[42,21]);
 }
});

test('all code-addressable effects are independently drawn transparent files',async()=>{
 const files=new Set(await readdir(join(root,'art/tech/parts')));
 const required=['vfx-pulse.png','vfx-arc.png','vfx-magnet.png','vfx-resin.png','vfx-scout.png','vfx-link-0.png','vfx-link-1.png','vfx-root-charge-off.png','vfx-root-charge-on.png','vfx-root-resonance.png','vfx-prism-beam.png','vfx-prism-muzzle.png','vfx-thorn-hit.png','vfx-tech-muzzle.png','vfx-tech-spark.png','vfx-turbine-burst.png','vfx-hook-snap.png','vfx-hook-line.png','vfx-bolt-hit-0.png','vfx-repair-pulse.png','vfx-bolt-ready.png','vfx-bolt-shot.png','vfx-drone-hum.png','vfx-jam-wave.png','vfx-magnet-field.png','vfx-holo-glitch.png','vfx-holo-ready.png','vfx-battery-core.png'];
 for(const name of required){assert.ok(files.has(name),name);const info=await png(join(root,'art/tech/parts',name));assert.ok(info.colorType===4||info.colorType===6,name);}
});
