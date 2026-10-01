// Shipped WASM and native UI, isolated browser/profile, no player save changes.
import assert from 'node:assert/strict';
import {mkdir,writeFile} from 'node:fs/promises';
import {join} from 'node:path';
const {chromium}=await import(process.env.PVZ_PLAYWRIGHT||'playwright-core');
const out=process.env.PVZ_QA_OUT||'/tmp/pvz-giant-imp-qa';await mkdir(out,{recursive:true});
const browser=await chromium.launch({executablePath:'/Applications/Google Chrome.app/Contents/MacOS/Google Chrome',headless:true,args:['--no-sandbox','--autoplay-policy=no-user-gesture-required']});
const page=await browser.newPage({viewport:{width:1100,height:750},hasTouch:true}),errors=[],results={};page.on('pageerror',e=>errors.push(e.message));
const wait=(f,a,t=30000)=>page.waitForFunction(f,a,{timeout:t});
const api=(...a)=>page.evaluate(a=>Module._pvz_sandbox_command(...[...a,0,0,0,0].slice(0,4)),a);
const snap=n=>page.screenshot({path:join(out,n+'.png')});
const zombies=()=>page.evaluate(()=>{const out=[];for(let i=0;i<200;++i){const z=Array.from({length:21},(_,f)=>Module._pvz_sandbox_zombie_data(i,f));if(z[0]<0)break;out.push(z);}return out;});
const plant=i=>page.evaluate(i=>Array.from({length:12},(_,f)=>Module._pvz_sandbox_plant_data(i,f)),i);
async function click(x,y){const b=await page.locator('#canvas').boundingBox(),s=await page.evaluate(()=>[Module.canvas.width,Module.canvas.height]);await page.mouse.click(b.x+x*b.width/s[0],b.y+y*b.height/s[1]);await page.waitForTimeout(100);}
async function boot(level=0){
 await page.goto(process.env.PVZ_QA_URL||'http://127.0.0.1:8097/');await wait(()=>!document.getElementById('start').disabled,undefined,90000);
 if(level)await page.evaluate(async level=>{
  const FS=Module.FS;if(!FS.analyzePath('/saves/userdata').exists)FS.mkdir('/saves/userdata');
  const name=new TextEncoder().encode('GiantImpQA'),users=new Uint8Array(16+name.length),u=new DataView(users.buffer);
  u.setUint32(0,14,true);u.setUint16(4,1,true);u.setUint16(6,name.length,true);users.set(name,8);u.setUint32(8+name.length,1,true);u.setUint32(12+name.length,1,true);FS.writeFile('/saves/userdata/users.dat',users);
  const profile=new Uint8Array(4096),p=new DataView(profile.buffer);p.setUint32(0,12,true);p.setUint32(4,level,true);FS.writeFile('/saves/userdata/user1.dat',profile);
  await new Promise((r,j)=>FS.syncfs(false,e=>e?j(e):r()));
 },level);
 await page.locator('#start').click();await page.waitForTimeout(12000);await click(400,560);await page.waitForTimeout(4500);
}
async function fresh(){await api(7);await api(4,1);await api(5,1);}
async function steps(n){for(let i=0;i<n;++i){await api(13);await page.waitForTimeout(30);}}
try{
 if(!process.env.PVZ_QA_ALMANAC_ONLY){
 await boot(22);
 if(!process.env.PVZ_QA_CAMPAIGN_ONLY){
 await click(260,348);await wait(()=>Module.canvas.width===1024);await fresh();
 await click(718,24);await click(81,457);await click(784,330);
 results.spawn=(await zombies())[0];assert.equal(results.spawn[0],215);assert.equal(results.spawn[4],270);assert.equal(results.spawn[15],24);
 await api(2,24,6,1);await api(2,23,7,3);await snap('catalogue-native-imp-and-giant');
 await api(4,0);await page.waitForTimeout(1500);await api(4,1);await snap('walking-head-registration');
 assert.equal((await zombies())[1][0],24);assert.equal((await zombies())[2][0],23);
 // High HP plant dies in one jaw strike; ordinary imp beside it only nibbles.
 await fresh();await api(1,23,4,2);await api(1,23,4,1);await api(2,215,4,2);await api(2,24,4,1);await api(4,0);
 await wait(()=>Module._pvz_sandbox_zombie_data(0,6)===1043);await api(4,1);results.windup=(await zombies())[0];assert.equal((await plant(0))[3],8000);await snap('jaw-windup');
 await steps(20);await snap('jaw-lifting');assert.equal((await plant(0))[3],8000);
 await api(4,0);await wait(()=>Module._pvz_sandbox_plant_data(0,2)===1);await api(4,1);
 results.afterImpact={zombies:await zombies(),plant:await plant(0)};assert.ok(results.afterImpact.plant[3]>7000);assert.equal(results.afterImpact.zombies[1][0],24);await snap('jaw-impact-instant-kill');
 // Freeze holds native attack phase/counter, then impact resumes normally.
 await fresh();await api(1,23,4,2);await api(1,14,0,0);await steps(55);await api(2,215,4,2);await api(4,0);
 await wait(()=>Module._pvz_sandbox_zombie_data(0,17)>0);await api(4,1);const frozen=(await zombies())[0];await steps(35);
 assert.equal((await zombies())[0][8],frozen[8]);assert.equal((await plant(0))[3],8000);results.freezeStopsAttack=true;await snap('frozen-jaw');
 await api(5,4);await api(4,0);await wait(()=>Module._pvz_sandbox_plant_data(0,0)===-1);results.freezeRecovers=true;
 // Giant's thrown child stays a normal imp, not our independent unit.
 await fresh();await api(1,20,0,2);await api(2,23,8,2);await api(5,4);await api(4,0);
 await wait(()=>{for(let i=0;i<30;++i)if(Module._pvz_sandbox_zombie_data(i,15)===24)return true;return false;},undefined,35000);await api(4,1);
 results.thrownChild=await zombies();assert.ok(results.thrownChild.some(z=>z[0]===24));assert.ok(results.thrownChild.every(z=>z[0]!==215));await snap('giant-throws-ordinary-imp');
 console.log('Sandbox jaw, freeze and giant-thrown native imp passed');
 await api(8,1);await api(4,1);assert.equal(await api(2,215,6,2),-5);assert.equal(await api(2,215,6,0),1);await snap('pool-land-lane');
 await page.setViewportSize({width:844,height:390});await page.waitForTimeout(1000);await snap('phone');await page.setViewportSize({width:1100,height:750});
 await api(15);await page.waitForTimeout(1800);
 }
 if(!process.env.PVZ_QA_SANDBOX_ONLY){
  await click(560,135);await page.waitForTimeout(15000);
  await snap('adventure-chooser');
  for(const base of [0,1,3,4,5,16]){const slot=base>=8?base+2:base;await click(47+slot%9*53,163+Math.floor(slot/9)*73);}
  await click(258,566);await wait(()=>Module._pvz_adventure_power_data(-1,5)===1,undefined,30000);
  assert.equal(await page.evaluate(()=>Module._pvz_adventure_power_data(-1,3)),22);
  console.log('Waiting for real adventure 3-2 wave');
  // No command cheats: wait for actual campaign wave, mowers handle early rows.
  await wait(()=>{for(let i=0;i<50;++i)if(Module._pvz_sandbox_zombie_data(i,0)===215)return true;return false;},undefined,150000);
  await click(748,14);results.adventure=await zombies();assert.ok(results.adventure.some(z=>z[0]===215));assert.ok(results.adventure.every(z=>![23,32].includes(z[0])));await snap('adventure-3-2');
  await click(400,401);await page.waitForTimeout(250);await click(305,394);await wait(()=>Module._pvz_adventure_power_data(-1,5)===-1);
  await page.evaluate(()=>new Promise((r,j)=>Module.FS.syncfs(false,e=>e?j(e):r())));await boot();await click(560,135);await wait(()=>Module._pvz_adventure_power_data(-1,5)===1,undefined,30000);
  results.resumed=await zombies();assert.deepEqual(results.resumed,results.adventure);await snap('resumed-giant-imp');
 }
 }
 await boot(37);await click(370,455);await click(590,366);await click(145,524);await page.waitForTimeout(700);await snap('giant-imp-almanac');
 assert.deepEqual(errors,[]);await writeFile(join(out,'report.json'),JSON.stringify({results,errors},null,2));console.log('Giant imp browser QA passed',out,results);
}catch(e){await snap('failure');await writeFile(join(out,'failure.json'),JSON.stringify({error:String(e),results,errors,zombies:await zombies(),log:await page.evaluate(()=>window.pvzEngineLog)},null,2));throw e;}
finally{await browser.close();}
