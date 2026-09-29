// Remove only retired sandbox assets; verify every native file is unchanged.
import {readFile,writeFile} from 'node:fs/promises';
import {createRequire} from 'node:module';
import {createHash} from 'node:crypto';
import assert from 'node:assert/strict';
const root=new URL('../',import.meta.url),JSZip=createRequire(import.meta.url)('../site/vendor/jszip-3.10.1.min.js');
const manifestPath=new URL('site/resource-manifest.json',root);
const m=JSON.parse(await readFile(manifestPath)),baseline=JSON.parse(await readFile(new URL('tests/baseline-assets.json',root)));
const hash=b=>createHash('sha256').update(b).digest('hex');
const source=await readFile(new URL('site/'+m.bundle.url,root));assert.equal(hash(source),m.bundle.sha256);
const zip=await JSZip.loadAsync(source,{checkCRC32:true});
const kept=m.files.filter(f=>!f.path.startsWith('images/sandbox/'));
assert.equal(kept.length,baseline.fileCount);assert.equal(hash(kept.map(f=>`${f.path}:${f.size}:${f.sha256}`).join('\n')),baseline.sha256);
for(const f of kept){const data=await zip.file(f.path).async('nodebuffer');assert.equal(hash(data),f.sha256);}
for(const name of Object.keys(zip.files))if(name.startsWith('images/sandbox/'))zip.remove(name);
const bytes=await zip.generateAsync({type:'nodebuffer',compression:'DEFLATE',compressionOptions:{level:6},platform:'UNIX'});
m.files=kept;m.totalFiles=kept.length;m.totalBytes=kept.reduce((n,f)=>n+f.size,0);
m.bundle={url:`resources/game-${hash(bytes).slice(0,12)}.zip`,size:bytes.length,sha256:hash(bytes)};
m.originalPlants={files:[],source:'Four fixed characters use native rigs and effects with independent behavior; infusion retired.'};
m.sandboxExpansion={powers:0,fixedCharacters:4,compatibleNativePlants:0,powerResults:0,totalCustomZombies:0,parts:0};
await writeFile(new URL('site/'+m.bundle.url,root),bytes);
await writeFile(manifestPath,JSON.stringify(m,null,2)+'\n');
console.log(new URL('site/'+m.bundle.url,root).pathname);
