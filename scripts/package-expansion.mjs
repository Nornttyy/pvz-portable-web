// Add only independently drawn Technology Garden rig parts to the complete,
// verified user-resource pack.  The source atlas images never go in the ZIP:
// the runtime receives the native-size skeletal pieces and separate VFX only.
import {readFile,writeFile,readdir} from 'node:fs/promises';
import {createRequire} from 'node:module';
import {createHash} from 'node:crypto';
import {resolve} from 'node:path';
import assert from 'node:assert/strict';
import {validateManifest} from '../web/resource-utils.mjs';
const root=new URL('../',import.meta.url),JSZip=createRequire(import.meta.url)('../site/vendor/jszip-3.10.1.min.js');
const hash=bytes=>createHash('sha256').update(bytes).digest('hex');
const base=process.argv[2],target=process.argv[3];
if(!base||!target)throw Error('Usage: node scripts/package-expansion.mjs audited-base.zip output.zip');
const baseline=JSON.parse(await readFile(new URL('tests/baseline-assets.json',root)));
const original=await readFile(resolve(base));assert.equal(hash(original),baseline.baseBundle.sha256);
const zip=await JSZip.loadAsync(original,{checkCRC32:true});
// The audited base can contain an earlier sandbox experiment.  This release is
// a replacement, not an append: old fire/ice files must never survive beside
// Technology Garden's declared runtime inventory.
for(const path of Object.keys(zip.files)){
 if(path.startsWith('images/sandbox/'))zip.remove(path);
}
const manifest=JSON.parse(await readFile(new URL('site/resource-manifest.json',root)));
const runtime=JSON.parse(await readFile(new URL('art/tech/runtime-parts.json',root)));
const vfxFiles=(await readdir(new URL('art/tech/parts/',root))).filter(file=>/^vfx-[a-z0-9-]+\.png$/.test(file)).sort();
const requiredVfx=[
 'vfx-pulse.png','vfx-arc.png','vfx-magnet.png','vfx-resin.png','vfx-scout.png',
 'vfx-pulse-hit-0.png','vfx-pulse-hit-1.png','vfx-arc-hit-0.png','vfx-arc-hit-1.png',
 'vfx-magnet-hit-0.png','vfx-magnet-hit-1.png','vfx-resin-hit-0.png','vfx-resin-hit-1.png',
 'vfx-scout-hit-0.png','vfx-scout-hit-1.png','vfx-tech-muzzle.png','vfx-link-0.png','vfx-link-1.png',
 'vfx-root-charge-off.png','vfx-root-charge-on.png','vfx-root-resonance.png','vfx-prism-beam.png',
 'vfx-prism-muzzle.png','vfx-thorn-hit.png','vfx-tech-spark.png','vfx-turbine-burst.png',
 'vfx-hook-snap.png','vfx-hook-line.png','vfx-bolt-hit-0.png','vfx-repair-pulse.png','vfx-bolt-ready.png',
 'vfx-bolt-shot.png','vfx-drone-hum.png','vfx-jam-wave.png','vfx-magnet-field.png','vfx-holo-glitch.png',
 'vfx-holo-ready.png','vfx-battery-core.png'
];
for(const file of requiredVfx)assert.ok(vfxFiles.includes(file),`missing drawn runtime VFX: ${file}`);
const parts=[
 ...runtime.map(part=>({...part,source:new URL(`art/tech/runtime/${part.file}`,root)})),
 ...vfxFiles.map(file=>({file,source:new URL(`art/tech/parts/${file}`,root),role:'effect-or-projectile'}))
];
const seen=new Set;
for(const part of parts){
 assert.match(part.file,/^[a-z0-9-]+\.png$/);assert.ok(!seen.has(part.file),`duplicate sandbox asset: ${part.file}`);seen.add(part.file);
 const bytes=await readFile(part.source);
 assert.equal(bytes.toString('ascii',1,4),'PNG');
 if(part.width){assert.equal(bytes.readUInt32BE(16),part.width);assert.equal(bytes.readUInt32BE(20),part.height);}
 zip.file('images/sandbox/'+part.file,bytes,{date:new Date('2026-09-26T00:00:00Z'),createFolders:false});
}
const files=[];
for(const file of Object.values(zip.files).filter(x=>!x.dir).sort((a,b)=>a.name.localeCompare(b.name,'en'))){
 const bytes=await file.async('nodebuffer');files.push({path:file.name,size:bytes.length,sha256:hash(bytes)});
}
// Preserve the base manifest's original ordering for an independent exact-content fingerprint.
const baseFiles=manifest.files.filter(f=>!f.path.startsWith('images/sandbox/'));
assert.equal(baseFiles.length,baseline.fileCount);
assert.equal(hash(baseFiles.map(f=>`${f.path}:${f.size}:${f.sha256}`).join('\n')),baseline.sha256);
for(const f of baseFiles){const actual=files.find(x=>x.path===f.path);assert.deepEqual(actual,f);}
manifest.files=[...baseFiles,...files.filter(f=>f.path.startsWith('images/sandbox/'))];
const bytes=await zip.generateAsync({type:'nodebuffer',compression:'DEFLATE',compressionOptions:{level:6},platform:'UNIX'});
manifest.bundle={url:`resources/game-${hash(bytes).slice(0,12)}.zip`,size:bytes.length,sha256:hash(bytes)};
manifest.totalFiles=manifest.files.length;manifest.totalBytes=manifest.files.reduce((n,f)=>n+f.size,0);
delete manifest.originalPlants;
manifest.technologyGarden={
 addedPlants:10,addedZombies:10,totalCustomPlants:10,totalCustomZombies:10,
 files:manifest.files.filter(f=>f.path.startsWith('images/sandbox/')).map(f=>f.path),
 mechanic:'根网共振：菌缆菇连接相邻科技植物，三格能量由玩家点按启动；干扰、断缆和修复都有独立特效。'
};
manifest.sandboxExpansion={
 addedPlants:10,addedZombies:10,totalCustomPlants:10,totalCustomZombies:10,
 parts:runtime.length,vfxParts:vfxFiles.length,
 source:'Built-in image generation, exported as separate transparent skeletal rig pieces at native track dimensions. Source atlas and extraction manifests live in art/tech; no legacy expansion art is bundled.'
};
manifest.delivery='bundled';delete manifest.localOnly;validateManifest(manifest);
await writeFile(resolve(target),bytes);
await writeFile(new URL('site/resource-manifest.json',root),JSON.stringify(manifest,null,2)+'\n');
console.log(`Expansion: ${manifest.totalFiles} files / ${bytes.length} ZIP bytes. All ${baseline.fileCount} non-sandbox files unchanged.`);
