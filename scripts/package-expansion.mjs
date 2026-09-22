// Add only generated rig parts to the complete, verified existing resource pack.
import {readFile,writeFile,copyFile,mkdir,readdir} from 'node:fs/promises';
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
const manifest=JSON.parse(await readFile(new URL('site/resource-manifest.json',root)));
const parts=[...JSON.parse(await readFile(new URL('art/expansion/parts.json',root))),...JSON.parse(await readFile(new URL('art/expansion/vfx-parts.json',root)))];
for(const part of parts){
 assert.match(part.file,/^[a-z0-9-]+\.png$/);
 const bytes=await readFile(new URL('art/expansion/parts/'+part.file,root));
 assert.equal(bytes.readUInt32BE(16),part.width);assert.equal(bytes.readUInt32BE(20),part.height);
 zip.file('images/sandbox/'+part.file,bytes,{date:new Date('2026-09-21T00:00:00Z'),createFolders:false});
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
manifest.originalPlants.files=manifest.files.filter(f=>f.path.startsWith('images/sandbox/')).map(f=>f.path);
manifest.sandboxExpansion={addedPlants:9,addedZombies:12,totalCustomPlants:17,totalCustomZombies:12,parts:parts.length,vfxParts:48,source:'Built-in image_gen edits with native joint registration; matching head/blink/mouth sets, three-stage storm mushroom, two ranged zombie rigs and damaged arms; original Gatling hardware reused unchanged; separate rig parts and 48 projectile/VFX sprites; prompts in art/expansion'};
manifest.delivery='bundled';delete manifest.localOnly;validateManifest(manifest);
await writeFile(resolve(target),bytes);
await writeFile(new URL('site/resource-manifest.json',root),JSON.stringify(manifest,null,2)+'\n');
console.log(`Expansion: ${manifest.totalFiles} files / ${bytes.length} ZIP bytes. All ${baseline.fileCount} non-sandbox files unchanged.`);
