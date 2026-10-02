// Actual native engine; disposable browser profile, never player save files.
import assert from 'node:assert/strict';
import {mkdir,writeFile} from 'node:fs/promises';
import {join} from 'node:path';
const {chromium}=await import(process.env.PVZ_PLAYWRIGHT||'playwright-core');
const out=process.env.PVZ_QA_OUT||'/tmp/pvz-louis-qa';await mkdir(out,{recursive:true});
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
 if(level)await page.evaluate(async level=>{const FS=Module.FS;if(!FS.analyzePath('/saves/userdata').exists)FS.mkdir('/saves/userdata');const name=new TextEncoder().encode('LouisQA'),users=new Uint8Array(16+name.length),u=new DataView(users.buffer);u.setUint32(0,14,true);u.setUint16(4,1,true);u.setUint16(6,name.length,true);users.set(name,8);u.setUint32(8+name.length,1,true);u.setUint32(12+name.length,1,true);FS.writeFile('/saves/userdata/users.dat',users);const profile=new Uint8Array(4096),p=new DataView(profile.buffer);p.setUint32(0,12,true);p.setUint32(4,level,true);FS.writeFile('/saves/userdata/user1.dat',profile);await new Promise((r,j)=>FS.syncfs(false,e=>e?j(e):r()));},level);
 await page.locator('#start').click();await page.waitForTimeout(12000);await click(400,560);await page.waitForTimeout(4500);
}
async function fresh(){await api(7);await api(5,1);await api(4,0);await page.waitForTimeout(700);}
try{
 if(!process.env.PVZ_QA_ALMANAC_ONLY){
 await boot(8);await click(719,27);await wait(()=>Module.canvas.width===1024);await fresh();
 for(let id=200;id<=211;++id)assert.equal(await api(2,id,6,2),-2);
 // Select the actual last zombie card, not just the C API.
 await click(718,24);await click(181,395);await click(904,330);await api(4,1);
 assert.equal(await zd(0,0),212);assert.equal(await zd(0,13),0);assert.equal(await zd(0,14),1);assert.equal(await zd(0,15),0);assert.equal(await zd(0,4),270);
 await api(2,0,7,1);assert.equal(await zd(1,13),1);await snap('native-and-louis');results.actualCardPlacesLouis=true;
 const x=await zd(0,2);await api(5,4);await api(4,0);await wait(()=>Module._pvz_sandbox_zombie_data(0,10)>=2100,undefined,25000);await api(4,1);
 assert.equal(await zd(0,4),270);assert.equal(await zd(0,14),1);assert.ok(await zd(0,2)<x);results.survivesWithoutHeadDecay=true;
 await fresh();await api(1,3,4,2);await api(2,212,4,2);await api(5,4);await wait(()=>Module._pvz_sandbox_plant_data(0,3)<4000);await api(4,1);assert.equal(await zd(0,4),270);await snap('eating-without-head');results.eating=true;
 await fresh();await api(1,0,1,2);await api(1,0,2,2);await api(2,212,7,2);await api(5,4);await wait(()=>Module._pvz_sandbox_zombie_data(0,14)===0,undefined,30000);await api(4,1);await snap('injured-no-phantom-head');results.injuredHP=await zd(0,4);await api(4,0);await wait(()=>Module._pvz_sandbox_zombie_data(0,0)===-1,undefined,15000);results.normalDeath=true;
 await api(8,1);await fresh();await api(2,212,7,2);await page.waitForTimeout(300);await api(4,1);assert.equal(await zd(0,0),212);assert.equal(await zd(0,13),0);await snap('headless-pool-walker');results.pool=true;
 await api(15);await page.waitForTimeout(1800);await click(560,135);await page.waitForTimeout(15000);
 for(const base of [0,3,1,5,4])await click(47+base*53,163);await click(471,163);await click(258,566);
 await wait(()=>Module._pvz_adventure_power_data(-1,5)===1,undefined,25000);await wait(()=>Module._pvz_sandbox_zombie_data(0,0)===212,undefined,45000);
 await wait(()=>Module._pvz_sandbox_zombie_data(0,2)<730,undefined,30000);await snap('adventure-walking');
 await click(748,14);results.adventure=await page.evaluate(()=>Array.from({length:16},(_,f)=>Module._pvz_sandbox_zombie_data(0,f)));assert.equal(results.adventure[4],270);assert.equal(results.adventure[13],0);assert.equal(results.adventure[14],1);await snap('adventure-spawn');
 await click(400,401);await page.waitForTimeout(250);await click(305,394);await wait(()=>Module._pvz_adventure_power_data(-1,5)===-1);await sync();await boot();await click(560,135);await wait(()=>Module._pvz_adventure_power_data(-1,5)===1,undefined,30000);
 results.resumed=await page.evaluate(()=>Array.from({length:16},(_,f)=>Module._pvz_sandbox_zombie_data(0,f)));assert.deepEqual(results.resumed,results.adventure);await snap('resumed-headless');results.saveResume=true;
 }
 // Seed a separate high-level test profile to open the native almanac.
 await boot(37);await click(370,455);await page.waitForTimeout(600);await snap('almanac-index');await click(590,366);await page.waitForTimeout(500);await click(315,524);await page.waitForTimeout(800);await snap('louis-almanac');
 await click(60,124);await page.waitForTimeout(300);await snap('normal-zombie-still-has-head');
 assert.deepEqual(errors,[]);await writeFile(join(out,'report.json'),JSON.stringify({results,errors},null,2));console.log('Louis XVI browser QA passed',out,results);
}catch(e){await snap('failure');await writeFile(join(out,'failure.json'),JSON.stringify({error:String(e),results,errors,log:await page.evaluate(()=>window.pvzEngineLog)},null,2));throw e;}
finally{await browser.close();}
