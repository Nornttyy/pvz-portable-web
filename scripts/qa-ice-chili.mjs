// Actual browser/engine checks; isolated profile, sandbox commands only there.
import assert from 'node:assert/strict';
import {mkdir,writeFile} from 'node:fs/promises';
import {join} from 'node:path';
const {chromium}=await import(process.env.PVZ_PLAYWRIGHT||'playwright-core');
const out=process.env.PVZ_QA_OUT||'/tmp/pvz-ice-chili-qa';await mkdir(out,{recursive:true});
const browser=await chromium.launch({executablePath:'/Applications/Google Chrome.app/Contents/MacOS/Google Chrome',headless:true,args:['--no-sandbox','--autoplay-policy=no-user-gesture-required']});
const page=await browser.newPage({viewport:{width:1100,height:750},hasTouch:true}),results={},errors=[];page.on('pageerror',e=>errors.push(e.message));
const wait=(f,a,t=30000)=>page.waitForFunction(f,a,{timeout:t});
const api=(...a)=>page.evaluate(a=>Module._pvz_sandbox_command(...[...a,0,0,0,0].slice(0,4)),a);
const z=(i,f)=>page.evaluate(([i,f])=>Module._pvz_sandbox_zombie_data(i,f),[i,f]);
const seed=(i,f)=>page.evaluate(([i,f])=>Module._pvz_adventure_seed_data(i,f),[i,f]);
const plants=()=>page.evaluate(()=>Array.from({length:40},(_,i)=>Array.from({length:7},(_,f)=>Module._pvz_stink_data(0,i,f))).filter(x=>x[0]>=0));
const snap=async n=>{await page.mouse.move(0,0);await page.screenshot({path:join(out,n+'.png')});};
async function click(x,y){const b=await page.locator('#canvas').boundingBox(),s=await page.evaluate(()=>[Module.canvas.width,Module.canvas.height]);await page.mouse.click(b.x+x*b.width/s[0],b.y+y*b.height/s[1]);await page.waitForTimeout(100);}
async function step(n){for(let i=0;i<n;++i){const tick=await page.evaluate(()=>Module._pvz_stink_data(3,0,0));await api(13);await wait(t=>Module._pvz_stink_data(3,0,0)>t,tick);}}
async function placeCard(i,x,y){await page.mouse.click(500,300,{button:'right'});await click(await seed(i,5)+25,await seed(i,6)+35);await click(x,y);}
try{
 await page.goto(process.env.PVZ_QA_URL||'http://127.0.0.1:8097/');await wait(()=>!document.getElementById('start').disabled,undefined,90000);
 await page.evaluate(async()=>{
  const FS=Module.FS;if(!FS.analyzePath('/saves/userdata').exists)FS.mkdir('/saves/userdata');
  const name=new TextEncoder().encode('IceQA'),users=new Uint8Array(16+name.length),u=new DataView(users.buffer);
  u.setUint32(0,14,true);u.setUint16(4,1,true);u.setUint16(6,name.length,true);users.set(name,8);u.setUint32(8+name.length,1,true);u.setUint32(12+name.length,1,true);FS.writeFile('/saves/userdata/users.dat',users);
  const profile=new Uint8Array(4096),p=new DataView(profile.buffer);p.setUint32(0,12,true);p.setUint32(4,26,true);FS.writeFile('/saves/userdata/user1.dat',profile);
  await new Promise((r,j)=>FS.syncfs(false,e=>e?j(e):r()));
 });
 await page.locator('#start').click();await page.waitForTimeout(12000);await click(400,560);await page.waitForTimeout(4500);
 await click(370,455);await click(208,366);await click(408,580);await click(155,127);await snap('almanac-ice');
 await click(280,580);await click(259,283);await snap('almanac-original-fire');await click(690,580);await page.waitForTimeout(1000);
 await click(260,348);await wait(()=>Module.canvas.width===1024);await api(8,0);await api(4,1);
 for(const row of [2,1])assert.equal(await api(2,32,7,row),1);
 assert.equal(await api(1,528,2,2),1);await snap('ice-windup');
 const hp=[await z(0,4),await z(1,4)];await step(99);assert.equal(await api(9),1,'one-second windup not shortened');
 await step(1);assert.equal(await api(9),0,'single-use plant consumed');
 results.blast={before:hp,after:[await z(0,4),await z(1,4)],frozen:await z(0,17),adjacentFreeze:await z(1,17),chill:await z(0,16)};
 assert.deepEqual(results.blast.after,[hp[0]-1200,hp[1]]);assert.ok(results.blast.frozen>=299&&results.blast.frozen<=300);assert.equal(results.blast.adjacentFreeze,0);assert.equal(results.blast.chill,0);
 await snap('row-ice-burst');const frozenX=await z(0,28),otherX=await z(1,28);await step(100);
 assert.equal(await z(0,28),frozenX);assert.ok(await z(1,28)<otherX);await snap('frozen-only-one-row');
 await step(200);assert.equal(await z(0,17),0);await step(50);assert.ok(await z(0,28)<frozenX);results.thaw=true;
 // Native fire still burns and does not acquire the ice damage/freeze logic.
 const redBefore=await z(1,4);assert.equal(await api(1,20,2,1),1);await snap('native-fire-windup');await step(100);
 assert.equal(await z(1,4),redBefore-1800);assert.equal(await z(1,17),0);results.nativeFire=true;await snap('native-fire-burst');
 await api(8,1);await api(4,1);assert.equal(await api(1,528,2,2),-4,'water still needs a lily pad');
 assert.equal(await api(1,16,2,2),1);assert.equal(await api(1,528,2,2),1);assert.equal(await api(2,4,7,2),1);
 const poolHp=await z(0,4)+await z(0,5);await step(100);assert.equal(await api(9),1,'lily pad survives');assert.equal(await z(0,4)+await z(0,5),poolHp-1200);assert.ok(await z(0,17)>290);await snap('pool-ice-burst');results.pool=true;
 await page.setViewportSize({width:844,height:390});await snap('phone');await page.setViewportSize({width:1100,height:750});
 await api(15);await page.waitForTimeout(1800);console.log('Sandbox exact damage/freeze, thaw, original fire and pool passed');
 await click(560,135);await page.waitForTimeout(15000);
 for(const base of [51,20,1,16,3,4]){const slot=base===51?42:base>=8?base+2:base;await click(47+slot%9*53,163+Math.floor(slot/9)*73);}
 await snap('adventure-chooser');await click(258,566);await wait(()=>Module._pvz_adventure_power_data(-1,5)===1);
 assert.deepEqual(await page.evaluate(()=>[0,1,2].map(f=>Module._pvz_adventure_seed_data(0,f))),[528,125,5000]);
 assert.deepEqual(await page.evaluate(()=>[0,1].map(f=>Module._pvz_adventure_seed_data(1,f))),[20,125]);
 await placeCard(2,120,130);const deadline=Date.now()+100000;let secondSun=false;
 while(Date.now()<deadline){
  const sun=await page.evaluate(()=>[Module._pvz_sun_data(0,1),Module._pvz_sun_data(0,2)]);if(sun[0]>=0)await click(...sun);
  const money=await page.evaluate(()=>Module._pvz_adventure_power_data(-1,2));
  if(!secondSun&&money>=50&&await seed(2,4)){await placeCard(2,120,215);secondSun=true;}
  else if(money>=150&&await seed(0,4))break;
  await page.waitForTimeout(200);
 }
 assert.ok(await page.evaluate(()=>Module._pvz_adventure_power_data(-1,2)>=150),'earn normal adventure sun');
 await placeCard(0,240,130);await click(748,14);results.saveBefore=await plants();assert.ok(results.saveBefore.some(p=>p[2]===528));assert.ok(await seed(0,3)>4800);await snap('adventure-ice-paused');
 await click(400,401);await page.waitForTimeout(350);await click(305,394);await page.waitForTimeout(1800);await click(560,135);await wait(()=>Module._pvz_adventure_power_data(-1,5)===1);
 results.saveAfter=await plants();assert.deepEqual(results.saveAfter,results.saveBefore);await click(280,371);
 await wait(()=>!Array.from({length:40},(_,i)=>Module._pvz_stink_data(0,i,2)).includes(528));await snap('adventure-ice-detonated');results.adventure=true;
 assert.deepEqual(errors,[]);await writeFile(join(out,'report.json'),JSON.stringify({results,errors},null,2));console.log('Ice chili QA passed',out,results);
}catch(e){console.error(e);await snap('failure').catch(()=>{});await writeFile(join(out,'failure.json'),JSON.stringify({error:String(e),results,errors,plants:await plants().catch(()=>[]),log:await page.evaluate(()=>window.pvzEngineLog).catch(()=>[])},null,2));throw e;}
finally{await browser.close();}
