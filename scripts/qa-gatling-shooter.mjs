// Actual WASM with an isolated synthetic profile; never changes player saves.
import assert from 'node:assert/strict';
import {mkdir,writeFile} from 'node:fs/promises';
import {join} from 'node:path';
const {chromium}=await import(process.env.PVZ_PLAYWRIGHT||'playwright-core');
const out=process.env.PVZ_QA_OUT||'/tmp/pvz-gatling-qa';await mkdir(out,{recursive:true});
const browser=await chromium.launch({executablePath:'/Applications/Google Chrome.app/Contents/MacOS/Google Chrome',headless:true,args:['--no-sandbox','--autoplay-policy=no-user-gesture-required']});
const page=await browser.newPage({viewport:{width:1100,height:750}}),errors=[],results={};page.on('pageerror',e=>errors.push(e.message));
const wait=(f,a,t=20000)=>page.waitForFunction(f,a,{timeout:t});
const snap=n=>page.screenshot({path:join(out,n+'.png')});
async function click(x,y){const b=await page.locator('#canvas').boundingBox(),s=await page.evaluate(()=>[Module.canvas.width,Module.canvas.height]);await page.mouse.click(b.x+x*b.width/s[0],b.y+y*b.height/s[1]);await page.waitForTimeout(100);}
const api=(...a)=>page.evaluate(a=>Module._pvz_sandbox_command(...[...a,0,0,0,0].slice(0,4)),a);
const plant=()=>page.evaluate(()=>Array.from({length:12},(_,f)=>Module._pvz_sandbox_plant_data(0,f)));
const zombie=()=>page.evaluate(()=>Array.from({length:21},(_,f)=>Module._pvz_sandbox_zombie_data(0,f)));
async function step(n=1){for(let i=0;i<n;++i){await api(13);await page.waitForTimeout(45);}}
async function boot(level=9){
 await page.goto(process.env.PVZ_QA_URL||'http://127.0.0.1:8097/');await wait(()=>!document.getElementById('start').disabled,undefined,90000);
 await page.evaluate(async level=>{
  const FS=Module.FS;if(!FS.analyzePath('/saves/userdata').exists)FS.mkdir('/saves/userdata');
  const name=new TextEncoder().encode('GatlingQA'),users=new Uint8Array(16+name.length),u=new DataView(users.buffer);
  u.setUint32(0,14,true);u.setUint16(4,1,true);u.setUint16(6,name.length,true);users.set(name,8);u.setUint32(8+name.length,1,true);u.setUint32(12+name.length,1,true);FS.writeFile('/saves/userdata/users.dat',users);
  const profile=new Uint8Array(4096),p=new DataView(profile.buffer);p.setUint32(0,12,true);p.setUint32(4,level,true);
  p.setUint32(416,1,true); // Native purchase array entry 0: Gatling Pea.
  FS.writeFile('/saves/userdata/user1.dat',profile);await new Promise((r,j)=>FS.syncfs(false,e=>e?j(e):r()));
 },level);
 await page.locator('#start').click();await page.waitForTimeout(12000);await click(400,560);await page.waitForTimeout(4500);
}
try{
 await boot();await click(260,348);await wait(()=>Module.canvas.width===1024);await api(7);await api(4,1);await api(5,1);
 assert.ok(await api(1,522,6,2)>0,'sandbox retains its existing free upgrade placement');
 await api(7);await api(4,1);await api(5,1);
 assert.ok(await api(1,521,6,2)>0);assert.ok(await api(1,522,6,2)>0);assert.equal((await plant())[0],522);
 await api(2,23,8,2);const health=(await zombie())[4];await step(9);
 assert.equal(await page.evaluate(()=>Module._pvz_projectile_data(0,0)),-1);
 await step();assert.equal(await page.evaluate(()=>Module._pvz_projectile_data(0,0)),298);
 await api(3,0,6,2);await api(4,0);
 await wait(()=>Module._pvz_sandbox_zombie_data(0,4)<3000);await api(4,1);
 assert.equal(health-(await zombie())[4],15);results.actualHitDamage=15;
 await api(7);await api(4,1);await api(5,1);assert.ok(await api(1,521,1,2)>0);
 // Catalogue slot 40 on page 2 is replaced, not appended as a new card.
 await click(222,571);await click(30,387);await click(384,330);assert.equal((await plant())[0],522);
 await api(2,32,8,2);await api(4,0);
 await wait(()=>Module._pvz_sandbox_plant_data(0,5)>=60);await api(4,1);await snap('half-heat');
 await api(4,0);await wait(()=>Module._pvz_sandbox_plant_data(0,4)===1);await api(4,1);
 const hot=await plant();assert.equal(hot[5],120);assert.ok(hot[6]>300&&hot[6]<=350);await snap('overheated');
 await page.waitForTimeout(500);assert.deepEqual(await plant(),hot);results.pauseFreezesCooldown=true;
 await step(hot[6]-1);assert.equal((await plant())[4],1);assert.equal((await plant())[6],1);assert.equal((await plant())[5],120);
 await step();const ready=await plant();assert.equal(ready[4],0);assert.equal(ready[6],0);assert.equal(ready[5],1);results.resumesOnCooldownBoundary=true;await snap('cooled-and-firing');
 await page.setViewportSize({width:844,height:390});await snap('phone');await page.setViewportSize({width:1100,height:750});
 await api(15);await page.waitForTimeout(1800);await click(560,135);await page.waitForTimeout(15000);await snap('adventure-chooser');
 for(const base of [0,1,3,5,7,40]){const slot=base>=8?base+1:base;await click(47+(slot%9)*53,158+Math.floor(slot/9)*70);}
 await click(258,566);await wait(()=>Module._pvz_adventure_power_data(-1,5)===1);
 results.adventureCards=await page.evaluate(()=>Array.from({length:6},(_,i)=>Module._pvz_adventure_seed_data(i,0)));
 assert.deepEqual(results.adventureCards,[500,520,501,5,521,522]);assert.equal(await page.evaluate(()=>Module._pvz_adventure_seed_data(5,1)),250);
 await boot(37);await click(370,455);await click(208,366);await click(278,427);await page.waitForTimeout(600);await snap('gatling-almanac');
 assert.deepEqual(errors,[]);await writeFile(join(out,'report.json'),JSON.stringify({results,errors},null,2));console.log('Gatling QA passed',out,results);
}catch(e){await snap('failure');await writeFile(join(out,'failure.json'),JSON.stringify({error:String(e),results,errors,log:await page.evaluate(()=>window.pvzEngineLog)},null,2));throw e;}
finally{await browser.close();}
