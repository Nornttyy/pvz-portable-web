// Actual shipped WASM and an isolated synthetic profile, no player save edits.
import assert from 'node:assert/strict';
import {mkdir,writeFile} from 'node:fs/promises';
import {join} from 'node:path';
const {chromium}=await import(process.env.PVZ_PLAYWRIGHT||'playwright-core');
const out=process.env.PVZ_QA_OUT||'/tmp/pvz-cone-wrap-qa';await mkdir(out,{recursive:true});
const browser=await chromium.launch({executablePath:'/Applications/Google Chrome.app/Contents/MacOS/Google Chrome',headless:true,args:['--no-sandbox','--autoplay-policy=no-user-gesture-required']});
const page=await browser.newPage({viewport:{width:1100,height:750}}),errors=[],results={};page.on('pageerror',e=>errors.push(e.message));
const wait=(f,a,t=30000)=>page.waitForFunction(f,a,{timeout:t});
const api=(...a)=>page.evaluate(a=>Module._pvz_sandbox_command(...[...a,0,0,0,0].slice(0,4)),a);
const snap=n=>page.screenshot({path:join(out,n+'.png')});
const zombie=i=>page.evaluate(i=>Array.from({length:21},(_,f)=>Module._pvz_sandbox_zombie_data(i,f)),i);
async function click(x,y){const b=await page.locator('#canvas').boundingBox(),s=await page.evaluate(()=>[Module.canvas.width,Module.canvas.height]);await page.mouse.click(b.x+x*b.width/s[0],b.y+y*b.height/s[1]);await page.waitForTimeout(100);}
async function boot(){
 await page.goto(process.env.PVZ_QA_URL||'http://127.0.0.1:8097/');await wait(()=>!document.getElementById('start').disabled,undefined,90000);
 await page.evaluate(async()=>{
  const FS=Module.FS;if(!FS.analyzePath('/saves/userdata').exists)FS.mkdir('/saves/userdata');
  const name=new TextEncoder().encode('ConeQA'),users=new Uint8Array(16+name.length),u=new DataView(users.buffer);
  u.setUint32(0,14,true);u.setUint16(4,1,true);u.setUint16(6,name.length,true);users.set(name,8);u.setUint32(8+name.length,1,true);u.setUint32(12+name.length,1,true);FS.writeFile('/saves/userdata/users.dat',users);
  const profile=new Uint8Array(4096),p=new DataView(profile.buffer);p.setUint32(0,12,true);p.setUint32(4,16,true);FS.writeFile('/saves/userdata/user1.dat',profile);
  await new Promise((r,j)=>FS.syncfs(false,e=>e?j(e):r()));
 });
 await page.locator('#start').click();await page.waitForTimeout(12000);await click(400,560);await page.waitForTimeout(4500);
 await click(719,27);await wait(()=>Module.canvas.width===1024);await api(7);await api(4,1);
}
try{
 await boot();await click(718,24);await click(31,457);await click(784,330);
 results.spawn=await zombie(0);assert.equal(results.spawn[0],214);assert.equal(results.spawn[4],270);assert.equal(results.spawn[5],2590);assert.equal(results.spawn[9],60);assert.equal(results.spawn[15],2);
 await api(2,2,6,1);await click(31,457);await snap('catalogue-full-armor');
 await api(4,0);await page.waitForTimeout(3000);await api(4,1);
 results.walk=await zombie(0);results.native=await zombie(1);assert.ok(results.walk[2]<results.spawn[2]);assert.equal(results.walk[5],2590);assert.equal(results.native[5],370);assert.equal(results.native[9],100);await snap('walking');
 await api(7);await api(4,1);await api(1,23,5,2);await api(2,214,6,2);await api(5,4);await api(4,0);
 await wait(()=>Module._pvz_sandbox_zombie_data(0,20)===1);await api(4,1);assert.equal((await zombie(0))[5],2590);await snap('eating-full-armor');
 // Armor damage changes cone textures, never reveals native anatomy.
 await api(1,522,1,2);await api(4,0);
 for(const [limit,name] of [[2350,'lightly-damaged'],[2200,'damaged'],[1450,'half-armor'],[330,'heavily-damaged'],[0,'armor-gone-still-cones']]){
  await wait(limit=>{const hp=Module._pvz_sandbox_zombie_data(0,5);return hp>=0&&hp<=limit;},limit);await api(4,1);
  const z=await zombie(0);assert.equal(z[0],214);if(limit>0)assert.equal(z[4],270);results[name]=z;await snap(name);await api(4,0);
 }
 await wait(()=>Module._pvz_sandbox_zombie_data(0,4)<270);await api(4,1);results.bodyDamaged=await zombie(0);
 await api(4,0);await wait(()=>Module._pvz_sandbox_zombie_data(0,0)===-1);results.canDie=true;
 await api(7);await api(4,1);await api(2,214,5,2);await api(1,20,1,2);await api(5,1);await api(4,0);
 await wait(()=>Module._pvz_sandbox_zombie_data(0,6)!==0);await api(4,1);await snap('burned-still-cones');
 await api(4,0);await wait(()=>Module._pvz_sandbox_zombie_data(0,0)===-1);results.burnDeath=true;
 await api(8,1);await api(4,1);await api(2,214,5,2);await api(2,2,5,3);await api(4,0);await page.waitForTimeout(2200);await api(4,1);await snap('pool-clipping');
 assert.equal((await zombie(0))[0],214);assert.equal((await zombie(0))[5],2590);assert.equal((await zombie(1))[5],370);
 // Let the SDL canvas finish resizing before capturing the mobile viewport.
 await page.setViewportSize({width:844,height:390});await page.waitForTimeout(1000);await snap('phone');
 assert.deepEqual(errors,[]);await writeFile(join(out,'report.json'),JSON.stringify({results,errors},null,2));console.log('Cone wrap QA passed',out,results);
}catch(e){await snap('failure');await writeFile(join(out,'failure.json'),JSON.stringify({error:String(e),results,errors,zombie:await zombie(0),log:await page.evaluate(()=>window.pvzEngineLog)},null,2));throw e;}
finally{await browser.close();}
