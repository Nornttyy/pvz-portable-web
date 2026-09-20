import {readFile,writeFile,mkdir} from 'node:fs/promises';
import {createHash} from 'node:crypto';
import assert from 'node:assert/strict';
export async function assembleResourceBundle(root){
  const manifest=JSON.parse(await readFile(new URL('site/resource-manifest.json',root)));
  if(manifest.delivery!=='bundled')return;
  const plan=JSON.parse(await readFile(new URL('resource-bundle/manifest.json',root)));
  const hash=bytes=>createHash('sha256').update(bytes).digest('hex');
  assert.equal(plan.sha256,manifest.bundle.sha256);assert.equal(plan.size,manifest.bundle.size);
  assert.equal(manifest.bundle.url,`resources/game-${plan.sha256.slice(0,12)}.zip`);
  assert.ok(plan.parts.length>0&&plan.parts.length<32);
  const parts=[];
  for(const [index,part]of plan.parts.entries()){
    assert.equal(part.path,`${plan.sha256.slice(0,12)}/part-${String(index+1).padStart(2,'0')}.bin`);
    assert.ok(part.size>0&&part.size<=8*1048576);
    const bytes=await readFile(new URL('resource-bundle/'+part.path,root));
    assert.equal(bytes.length,part.size);assert.equal(hash(bytes),part.sha256);parts.push(bytes);
  }
  const bytes=Buffer.concat(parts);
  assert.equal(bytes.length,manifest.bundle.size);assert.equal(hash(bytes),manifest.bundle.sha256);
  await mkdir(new URL('site/resources/',root),{recursive:true});
  await writeFile(new URL('site/'+manifest.bundle.url,root),bytes);
}
