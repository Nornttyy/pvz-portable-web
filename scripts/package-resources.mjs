// Explicitly package only the audited game bundle, never the project or player saves.
import {readFile, copyFile, mkdir, writeFile} from 'node:fs/promises';
import {resolve, dirname} from 'node:path';
import {createRequire} from 'node:module';
import assert from 'node:assert/strict';
import {validateManifest, sha256} from '../web/resource-utils.mjs';
const JSZip = createRequire(import.meta.url)('../site/vendor/jszip-3.10.1.min.js');
const source = process.argv[2];
if (!source) throw Error('Usage: node scripts/package-resources.mjs /path/to/local-resources.zip');
const root = new URL('../', import.meta.url);
const manifestPath = new URL('site/resource-manifest.json', root);
const manifest = validateManifest(JSON.parse(await readFile(manifestPath)));
const bytes = await readFile(resolve(source));
assert.equal(bytes.length, manifest.bundle.size);
assert.equal(await sha256(bytes), manifest.bundle.sha256);
const zip = await JSZip.loadAsync(bytes, {checkCRC32:true});
assert.equal(Object.values(zip.files).filter(f => !f.dir).length, manifest.files.length);
for (const file of manifest.files) {
  const entry = zip.file(file.path);
  assert.ok(entry && (!entry.unsafeOriginalName || entry.unsafeOriginalName === file.path));
  const value = await entry.async('uint8array');
  assert.equal(value.length, file.size, file.path);
  assert.equal(await sha256(value), file.sha256, file.path);
}
delete manifest.localOnly;
manifest.delivery = 'bundled';
manifest.bundle.url = `resources/game-${manifest.bundle.sha256.slice(0,12)}.zip`;
validateManifest(manifest);
const target = new URL('site/' + manifest.bundle.url, root);
await mkdir(dirname(target.pathname), {recursive:true});
await copyFile(resolve(source), target);
// Keep each Git blob small; the build reassembles the exact original ZIP.
const parts=[];
const partRoot=new URL(`resource-bundle/${manifest.bundle.sha256.slice(0,12)}/`,root);
await mkdir(partRoot,{recursive:true});
for(let offset=0,index=1;offset<bytes.length;offset+=8*1048576,index++){
  const part=bytes.subarray(offset,Math.min(offset+8*1048576,bytes.length));
  const file=`part-${String(index).padStart(2,'0')}.bin`;
  await writeFile(new URL(file,partRoot),part);
  parts.push({path:manifest.bundle.sha256.slice(0,12)+'/'+file,size:part.length,sha256:await sha256(part)});
}
await writeFile(new URL('resource-bundle/manifest.json',root),JSON.stringify({size:bytes.length,sha256:manifest.bundle.sha256,parts},null,2)+'\n');
await writeFile(manifestPath, JSON.stringify(manifest,null,2)+'\n');
console.log(`Packaged all ${manifest.files.length} verified game files (${bytes.length} bytes); no player saves.`);
