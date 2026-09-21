// Synchronize generated build/data artifacts to the existing local preview.
import {readFile,writeFile,copyFile,mkdir,access} from 'node:fs/promises';
import {createHash} from 'node:crypto';
import assert from 'node:assert/strict';
const root=new URL('../',import.meta.url),local=process.argv[2];
if(!local)throw Error('Expected local preview directory');
const target=new URL('file://'+local.replace(/\/$/,'')+'/');
const manifest=JSON.parse(await readFile(new URL('site/resource-manifest.json',root)));
const bytes=await readFile(new URL('resources/expansion-resources.zip',target));
assert.equal(createHash('sha256').update(bytes).digest('hex'),manifest.bundle.sha256);
const parts=JSON.parse(await readFile(new URL('art/expansion/parts.json',root)));
await mkdir(new URL('resources/source/images/sandbox/',target),{recursive:true});
for(const part of parts)await copyFile(new URL('art/expansion/parts/'+part.file,root),new URL('resources/source/images/sandbox/'+part.file,target));
const backup=new URL('resources/pre-expansion-resources.zip',target);
try{await access(backup);}catch{await copyFile(new URL('resources/local-resources.zip',target),backup);}
await copyFile(new URL('resources/expansion-resources.zip',target),new URL('resources/local-resources.zip',target));
manifest.localOnly=true;delete manifest.delivery;manifest.bundle.url='/local-resources.zip';
await writeFile(new URL('resources/manifest.json',target),JSON.stringify(manifest,null,2)+'\n');
const legacyPath=new URL('resources/original-plants/manifest.json',target);
const legacy=JSON.parse(await readFile(legacyPath));
for(const entry of legacy.files){const latest=manifest.files.find(f=>f.path===entry.path);assert.ok(latest);Object.assign(entry,latest);}
await writeFile(legacyPath,JSON.stringify(legacy,null,2)+'\n');
console.log('Local preview synchronized; prior complete ZIP retained as pre-expansion-resources.zip.');
