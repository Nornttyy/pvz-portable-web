import test from 'node:test';
import assert from 'node:assert/strict';
import {readFile} from 'node:fs/promises';
import {createRequire} from 'node:module';
import {sha256,validateManifest} from '../web/resource-utils.mjs';
import {loadResourceBundle} from '../web/resource-loader.mjs';
const JSZip = createRequire(import.meta.url)('../site/vendor/jszip-3.10.1.min.js');
const bytes = new Uint8Array([1,2,3,4,5]);
const digest = await sha256(bytes);
const manifest = {delivery:'bundled',bundle:{url:`resources/game-${digest.slice(0,12)}.zip`,size:bytes.length,sha256:digest}};

test('first visit streams all bytes and caches without a DOM or user-selected file', async () => {
  const statuses=[],writes=[];
  const result=await loadResourceBundle(manifest,(s,p)=>statuses.push([s,p]),{
    cache:async(mode,key,data)=>{assert.equal(key,digest);if(mode==='put')writes.push(data);return null;},
    fetcher:async(url,options)=>{
      assert.equal(url,manifest.bundle.url);assert.ok(options.signal);
      return new Response(new ReadableStream({start(c){c.enqueue(bytes.slice(0,2));c.enqueue(bytes.slice(2));c.close();}}));
    },
  });
  assert.deepEqual(result,bytes);assert.deepEqual(writes,[bytes]);
  assert.ok(statuses.some(([s])=>s.startsWith('正在下载游戏')));
  assert.ok(statuses.every(([,p])=>p>=8&&p<=56));
});
test('valid cache avoids download; invalid cache is repaired without save database access', async () => {
  let downloads=0;
  const fetcher=async()=>{downloads++;return new Response(bytes);};
  assert.deepEqual(await loadResourceBundle(manifest,()=>{},{cache:async()=>bytes,fetcher}),bytes);
  assert.equal(downloads,0);
  assert.deepEqual(await loadResourceBundle(manifest,()=>{},{cache:async mode=>mode==='get'?new Uint8Array([0]):true,fetcher}),bytes);
  assert.equal(downloads,1);
});
test('temporary download error retries once automatically', async () => {
  let calls=0;
  const value=await loadResourceBundle(manifest,()=>{},{cache:async()=>null,fetcher:async()=>{
    if(calls++===0)throw Error('network interrupted');return new Response(bytes);
  }});
  assert.deepEqual(value,bytes);assert.equal(calls,2);
});
test('404, truncated, oversized and corrupted downloads never reach the cache', async () => {
  for(const response of [()=>new Response('',{status:404}),()=>new Response(bytes.slice(1)),()=>new Response(new Uint8Array(6)),()=>new Response(new Uint8Array(5))]){
    let writes=0,calls=0;
    await assert.rejects(loadResourceBundle(manifest,()=>{},{cache:async mode=>{if(mode==='put')writes++;return null;},fetcher:async()=>{calls++;return response();}}));
    assert.equal(writes,0);assert.equal(calls,2);
  }
});
test('network timeout settles with a retry message, rather than endless loading', async () => {
  await assert.rejects(loadResourceBundle(manifest,()=>{},{cache:async()=>null,timeoutMs:5,
    fetcher:async(_url,{signal})=>new Promise((_r,reject)=>signal.addEventListener('abort',()=>reject(new DOMException('aborted','AbortError')))),
  }),/下载超时/);
});
test('resource URL cannot leave the GitHub project path', async () => {
  for(const url of ['https://example.com/game.zip','/game.zip','../game.zip','resources/game-other.zip']){
    await assert.rejects(loadResourceBundle({...manifest,bundle:{...manifest.bundle,url}},()=>{}, {fetcher:()=>assert.fail('must not fetch')}),/地址不正确/);
  }
});
test('deployed ZIP contains every original manifest entry, including animations and audio, with no extra files', async () => {
  const read=path=>readFile(new URL('../site/'+path,import.meta.url));
  const plan=validateManifest(JSON.parse(await read('resource-manifest.json')));
  assert.equal(plan.delivery,'bundled');
  const data=await read(plan.bundle.url);
  assert.equal(data.length,plan.bundle.size);assert.equal(await sha256(data),plan.bundle.sha256);
  const zip=await JSZip.loadAsync(data,{checkCRC32:true});
  assert.equal(Object.values(zip.files).filter(f=>!f.dir).length,plan.totalFiles);
  for(const entry of plan.files){
    const source=zip.file(entry.path);assert.ok(source,entry.path);
    const actual=await source.async('uint8array');
    assert.equal(actual.length,entry.size,entry.path);assert.equal(await sha256(actual),entry.sha256,entry.path);
  }
  for(const prefix of ['reanim/','sounds/','data/','images/','particles/'])assert.ok(plan.files.some(f=>f.path.startsWith(prefix)),prefix);
  const baseline=JSON.parse(await readFile(new URL('../tests/baseline-assets.json',import.meta.url)));
  const tech=plan.files.filter(f=>f.path.startsWith('images/sandbox/'));
  assert.equal(plan.technologyGarden.addedPlants,10);
  assert.equal(plan.technologyGarden.addedZombies,10);
  assert.equal(tech.length,252,'only the declared Technology Garden runtime pieces may extend the base pack');
  assert.equal(plan.totalFiles,baseline.fileCount+tech.length);
});
