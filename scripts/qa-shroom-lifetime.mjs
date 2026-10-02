// Native WASM lifetime, support cleanup and real adventure save/reload.
// All progress lives in a disposable browser profile, never a player's save.
import assert from 'node:assert/strict';
import {mkdir,writeFile} from 'node:fs/promises';
import {join} from 'node:path';
const {chromium}=await import(process.env.PVZ_PLAYWRIGHT||'playwright-core');
const out=process.env.PVZ_QA_OUT||'/tmp/pvz-shroom-lifetime-qa';await mkdir(out,{recursive:true});
const browser=await chromium.launch({executablePath:process.env.PVZ_CHROME||'/Applications/Google Chrome.app/Contents/MacOS/Google Chrome',headless:true,args:['--no-sandbox']});
const page=await browser.newPage({viewport:{width:1100,height:750},hasTouch:true}),errors=[],results={};
page.on('pageerror',e=>errors.push(e.message));
async function point(x,y){const b=await page.locator('#canvas').boundingBox(),s=await page.evaluate(()=>[Module.canvas.width,Module.canvas.height]);return[b.x+x*b.width/s[0],b.y+y*b.height/s[1]];}
async function click(x,y){await page.mouse.click(...await point(x,y));await page.waitForTimeout(120);}
async function api(...args){return page.evaluate(a=>Module._pvz_sandbox_command(...[...a,0,0,0,0].slice(0,4)),args);}
async function ad(i,f){return page.evaluate(([i,f])=>Module._pvz_adventure_power_data(i,f),[i,f]);}
async function snap(name){await page.screenshot({path:join(out,name+'.png')});}
async function plants(){return page.evaluate(()=>{const list=[];for(let i=0;i<180&&Module._pvz_sandbox_plant_data(i,0)>=0;++i)list.push([0,1,2,3,10].map(f=>Module._pvz_sandbox_plant_data(i,f)));return list;});}
async function wait(fn,arg,timeout=20000){await page.waitForFunction(fn,arg,{timeout});}
async function boot(seed=false){
 await page.goto(process.env.PVZ_QA_URL||'http://127.0.0.1:8097/');await wait(()=>!document.getElementById('start').disabled,undefined,90000);
 if(seed)await page.evaluate(async()=>{
  const FS=Module.FS;if(!FS.analyzePath('/saves/userdata').exists)FS.mkdir('/saves/userdata');
  const name=new TextEncoder().encode('LifetimeQA'),users=new Uint8Array(16+name.length),u=new DataView(users.buffer);
  u.setUint32(0,14,true);u.setUint16(4,1,true);u.setUint16(6,name.length,true);users.set(name,8);u.setUint32(8+name.length,1,true);u.setUint32(12+name.length,1,true);FS.writeFile('/saves/userdata/users.dat',users);
  const profile=new Uint8Array(4096),p=new DataView(profile.buffer);p.setUint32(0,12,true);p.setUint32(4,13,true);FS.writeFile('/saves/userdata/user1.dat',profile);
  await new Promise((r,j)=>FS.syncfs(false,e=>e?j(e):r()));
 });
 await page.locator('#start').click();await page.waitForTimeout(12000);await click(400,560);await page.waitForTimeout(4500);
}
try{
 await boot(true);await click(719,27);await wait(()=>Module.canvas.width===1024);
 await api(7);for(const [type,col] of [[33,1],[502,1],[30,1],[8,3]])assert.ok(await api(1,type,col,2)>0);
 assert.equal((await plants()).find(p=>p[0]===502)[4],6000);await api(5,4);await api(4,0);
 await wait(()=>Module._pvz_sandbox_plant_data(1,10)<5000);await api(4,1);
 const paused=await plants();await page.waitForTimeout(500);assert.deepEqual(await plants(),paused);results.pauseFreezesLifetime=true;
 assert.ok(await api(1,502,2,2)>0);await api(4,0);
 await wait(()=>Module._pvz_sandbox_plant_data(1,0)!==502);await api(4,1);
 let live=await plants();assert.deepEqual(live.map(p=>p[0]),[33,30,8,502]);assert.ok(live[3][4]>0&&live[3][4]<1500);
 await snap('expired-shroom-keeps-pot-pumpkin-and-native-puff');results.independentLifetimes=true;results.potAndPumpkinSurvive=true;results.nativePuffHasNoLifetime=true;
 await api(4,0);await wait(()=>Module._pvz_sandbox_plant_data(3,0)<0);await api(4,1);
 assert.ok(await api(1,502,1,2)>0);assert.equal((await plants()).find(p=>p[0]===502)[4],6000);results.cellReusable=true;
 await api(8,1);for(const type of [16,502,30])assert.ok(await api(1,type,2,2)>0);await api(5,4);await api(4,0);
 await wait(()=>Module._pvz_sandbox_plant_data(1,0)!==502);await api(4,1);
 assert.deepEqual((await plants()).map(p=>p[0]),[16,30]);results.lilyAndPumpkinSurvive=true;await snap('water-support-remains');

 // Select the replacement's ordinary native card in adventure 2-3.
 await api(15);await page.waitForTimeout(900);await click(560,135);await page.waitForTimeout(15000);
 for(const base of [8,1,0,3,5,2]){const slot=base>=8?base+1:base;await click(47+slot%9*53,163+Math.floor(slot/9)*73);}
 await click(258,566);await wait(()=>Module._pvz_adventure_power_data(-1,5)===1,undefined,25000);
 assert.equal(await page.evaluate(()=>Module._pvz_adventure_seed_data(0,0)),502);
 await click(110,43);await click(80,130);assert.equal(await ad(0,0),502);assert.ok(await ad(0,15)>5900);
 await wait(()=>Module._pvz_adventure_power_data(0,15)<=5000,undefined,18000);await click(748,14);
 const remaining=await ad(0,15);assert.ok(remaining>4500&&remaining<=5000);await page.waitForTimeout(400);assert.equal(await ad(0,15),remaining);
 await snap('adventure-paused-lifetime');await click(400,401);await page.waitForTimeout(300);await click(305,394);
 await wait(()=>Module._pvz_adventure_power_data(-1,5)===-1);
 await page.evaluate(()=>new Promise((r,j)=>Module.FS.syncfs(false,e=>e?j(e):r())));
 await boot();await click(560,135);await wait(()=>Module._pvz_adventure_power_data(-1,5)===1,undefined,40000);
 assert.equal(await ad(0,0),502);assert.equal(await ad(0,15),remaining);await page.waitForTimeout(400);assert.equal(await ad(0,15),remaining);
 results.exactAdventureResume=remaining;await page.setViewportSize({width:844,height:390});await page.waitForTimeout(300);await snap('remaining-lifetime-restored-on-phone');
 await page.evaluate(()=>{window.lifeTrace=[];const sample=()=>{const d=Module._pvz_adventure_power_data;window.lifeTrace.push([d(0,0),d(0,15),d(0,3)]);if(d(0,0)===502)requestAnimationFrame(sample);};requestAnimationFrame(sample);});
 await page.touchscreen.tap(...await point(280,371));await wait(()=>Module._pvz_adventure_power_data(0,0)!==502,undefined,65000);
 const trace=await page.evaluate(()=>window.lifeTrace);const alive=trace.filter(p=>p[0]===502);
 assert.ok(alive.length>100&&alive.at(-1)[1]<=5);assert.ok(alive.every(p=>p[2]===300));
 for(let i=1;i<alive.length;++i)assert.ok(alive[i][1]<=alive[i-1][1]);
 results.adventureExpiresNaturally=true;await snap('adventure-expired-no-ghost');
 await page.touchscreen.tap(...await point(110,43));await page.touchscreen.tap(...await point(80,130));await page.waitForTimeout(150);
 assert.equal(await ad(0,0),502);assert.ok(await ad(0,15)>5900);results.mobileReplant=true;
 assert.deepEqual(errors,[]);await writeFile(join(out,'report.json'),JSON.stringify({results,errors,lastLifetimeSamples:trace.slice(-15)},null,2));console.log('Shroom lifetime QA passed',results);
}catch(e){await snap('failure');await writeFile(join(out,'failure.json'),JSON.stringify({error:String(e),results,errors,plants:await plants(),adventure:[await ad(0,0),await ad(0,15)]},null,2));throw e;}
finally{await browser.close();}
