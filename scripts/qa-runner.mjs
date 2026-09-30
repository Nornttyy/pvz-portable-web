// Native WASM interaction tests; fresh isolated profile, never player data.
import assert from 'node:assert/strict';
import {mkdir,writeFile} from 'node:fs/promises';
import {join} from 'node:path';
const {chromium}=await import(process.env.PVZ_PLAYWRIGHT||'playwright-core');
const out=process.env.PVZ_QA_OUT||'/tmp/pvz-runner-qa';await mkdir(out,{recursive:true});
const browser=await chromium.launch({executablePath:'/Applications/Google Chrome.app/Contents/MacOS/Google Chrome',headless:true,args:['--no-sandbox','--autoplay-policy=no-user-gesture-required']});
const page=await browser.newPage({viewport:{width:1100,height:750},hasTouch:true}),errors=[],results={};page.on('pageerror',e=>errors.push(e.message));
const wait=(f,a,t=15000)=>page.waitForFunction(f,a,{timeout:t});
async function click(x,y){const b=await page.locator('#canvas').boundingBox(),s=await page.evaluate(()=>[Module.canvas.width,Module.canvas.height]);await page.mouse.click(b.x+x*b.width/s[0],b.y+y*b.height/s[1]);await page.waitForTimeout(100);}
const api=(...a)=>page.evaluate(a=>Module._pvz_sandbox_command(...[...a,0,0,0,0].slice(0,4)),a);
const zd=(i,f)=>page.evaluate(([i,f])=>Module._pvz_sandbox_zombie_data(i,f),[i,f]);
const snap=n=>page.screenshot({path:join(out,n+'.png')});
const sync=()=>page.evaluate(()=>new Promise((r,j)=>Module.FS.syncfs(false,e=>e?j(e):r())));
async function boot(level=0){
 await page.goto(process.env.PVZ_QA_URL||'http://127.0.0.1:8097/');await wait(()=>!document.getElementById('start').disabled,undefined,90000);
 if(level)await page.evaluate(async level=>{const FS=Module.FS;if(!FS.analyzePath('/saves/userdata').exists)FS.mkdir('/saves/userdata');const name=new TextEncoder().encode('RunnerQA'),users=new Uint8Array(16+name.length),u=new DataView(users.buffer);u.setUint32(0,14,true);u.setUint16(4,1,true);u.setUint16(6,name.length,true);users.set(name,8);u.setUint32(8+name.length,1,true);u.setUint32(12+name.length,1,true);FS.writeFile('/saves/userdata/users.dat',users);const profile=new Uint8Array(4096),p=new DataView(profile.buffer);p.setUint32(0,12,true);p.setUint32(4,level,true);FS.writeFile('/saves/userdata/user1.dat',profile);await new Promise((r,j)=>FS.syncfs(false,e=>e?j(e):r()));},level);
 await page.locator('#start').click();await page.waitForTimeout(12000);await click(400,560);await page.waitForTimeout(4500);
}
async function fresh(){await api(7);await api(5,1);await api(4,1);}
async function findRunner(){return page.evaluate(()=>{for(let i=0;i<160;++i)if(Module._pvz_sandbox_zombie_data(i,0)===213)return i;return -1;});}
const runnerData=i=>page.evaluate(i=>Array.from({length:21},(_,f)=>Module._pvz_sandbox_zombie_data(i,f)),i);
async function saveQuit(){await click(400,401);await page.waitForTimeout(250);await click(305,394);await wait(()=>Module._pvz_adventure_power_data(-1,5)===-1);await sync();}
try{
 await boot(8);await click(260,348);await wait(()=>Module.canvas.width===1024);await fresh();
 // Fifth cell in the fifth row: select the real runner card.
 await api(1,3,0,2);await api(1,1,3,2);await api(1,3,6,2);await click(718,24);await click(229,395);await click(944,330);
 assert.equal(await zd(0,0),213);assert.equal(await zd(0,14),1);assert.equal(await zd(0,4),270);results.card=true;
 await api(4,0);await wait(()=>Module._pvz_sandbox_zombie_data(0,2)<480);await api(4,1);await snap('inbound-running');
 assert.equal(await zd(0,19),0);await api(4,0);await wait(()=>Module._pvz_sandbox_zombie_data(0,6)===1041);await api(4,1);await snap('braking-at-back-row');
 results.turnX=await zd(0,2);assert.ok(results.turnX>=40&&results.turnX<90);assert.equal(await zd(0,20),0);
 await api(4,0);await wait(()=>Module._pvz_sandbox_zombie_data(0,6)===1042);await page.waitForTimeout(450);await api(4,1);assert.equal(await zd(0,18),1);await snap('fleeing-facing-right');
 await api(4,0);await wait(()=>Module._pvz_sandbox_zombie_data(0,0)===-1);
 assert.deepEqual(await page.evaluate(()=>[0,1,2].map(i=>Module._pvz_sandbox_plant_data(i,3))),[4000,300,4000]);results.noPlantDamage=true;
 // Empty lane still turns safely instead of reaching the losing boundary.
 await fresh();await api(2,213,8,1);await api(4,0);await wait(()=>Module._pvz_sandbox_zombie_data(0,6)===1042);await api(4,1);assert.ok(await zd(0,2)>=40);results.emptyLane=true;
 // Native freeze and thaw must pause, not consume, the run's progress.
 await fresh();await api(2,213,8,2);await api(1,14,0,0);await api(4,0);await wait(()=>Module._pvz_sandbox_zombie_data(0,17)>0);await api(4,1);
 const frozen=await zd(0,2);await api(4,0);await page.waitForTimeout(400);await api(4,1);assert.equal(await zd(0,2),frozen);await snap('frozen-runner');results.freeze=true;
 await api(5,4);await api(4,0);await wait(()=>Module._pvz_sandbox_zombie_data(0,17)===0,undefined,20000);await api(4,1);assert.ok(await zd(0,16)>0);results.thaw=true;
 // Explosions can kill him mid-charge; no immunity or automatic healing.
 await fresh();await api(2,213,7,2);await api(1,20,0,2);await api(4,0);await wait(()=>Module._pvz_sandbox_zombie_data(0,0)===-1);results.nativeDeath=true;
 await api(8,1);await fresh();await api(2,213,8,2);await api(4,0);await wait(()=>Module._pvz_sandbox_zombie_data(0,6)===1042);await api(4,1);await snap('pool-turn');await api(4,0);await wait(()=>Module._pvz_sandbox_zombie_data(0,0)===-1);results.poolExit=true;
 await api(15);await page.waitForTimeout(1800);await click(560,135);await page.waitForTimeout(15000);
 for(const base of [0,3,1,5,4])await click(47+base*53,163);await click(471,163);await click(258,566);await wait(()=>Module._pvz_adventure_power_data(-1,5)===1,undefined,25000);
 // Real wave schedule, no test-only enemy injections or timer cheats.
 await wait(()=>{for(let i=0;i<160;++i)if(Module._pvz_sandbox_zombie_data(i,0)===213&&Module._pvz_sandbox_zombie_data(i,6)===1042)return true;return false;},undefined,100000);
 await click(748,14);results.saved=await runnerData(await findRunner());assert.equal(results.saved[6],1042);await snap('adventure-runaway');
 await saveQuit();await boot();await click(560,135);await wait(()=>Module._pvz_adventure_power_data(-1,5)===1,undefined,30000);results.resumed=await runnerData(await findRunner());assert.deepEqual(results.resumed,results.saved);await snap('resumed-outbound');results.saveResume=true;
 await click(280,371);await wait(()=>{for(let i=0;i<160;++i)if(Module._pvz_sandbox_zombie_data(i,0)===213)return false;return true;});results.adventureExit=true;
 await boot(37);await click(370,455);await click(590,366);await click(400,524);await page.waitForTimeout(600);await snap('runner-almanac');await page.setViewportSize({width:844,height:390});await snap('phone-almanac');
 assert.deepEqual(errors,[]);await writeFile(join(out,'report.json'),JSON.stringify({results,errors},null,2));console.log('Runaway zombie QA passed',out,results);
}catch(e){await snap('failure');await writeFile(join(out,'failure.json'),JSON.stringify({error:String(e),results,errors,log:await page.evaluate(()=>window.pvzEngineLog)},null,2));throw e;}
finally{await browser.close();}
