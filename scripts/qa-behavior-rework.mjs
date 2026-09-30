// Browser QA drives the shipped native engine, never user profiles or live saves.
import assert from 'node:assert/strict';
import {mkdir,writeFile} from 'node:fs/promises';
import {join} from 'node:path';
const {chromium}=await import(process.env.PVZ_PLAYWRIGHT||'playwright-core');
const out=process.env.PVZ_QA_OUT||'/tmp/pvz-behavior-rework';await mkdir(out,{recursive:true});
const browser=await chromium.launch({executablePath:'/Applications/Google Chrome.app/Contents/MacOS/Google Chrome',headless:true,args:['--no-sandbox','--autoplay-policy=no-user-gesture-required']});
const page=await browser.newPage({viewport:{width:1100,height:750}}),results={},errors=[];page.on('pageerror',e=>errors.push(e.message));
const api=(...v)=>page.evaluate(v=>Module._pvz_sandbox_command(...[...v,0,0,0].slice(0,4)),v);
const zd=(i,f)=>page.evaluate(([i,f])=>Module._pvz_sandbox_zombie_data(i,f),[i,f]);
const wait=(f,a,t=20000)=>page.waitForFunction(f,a,{timeout:t});
async function click(x,y){const b=await page.locator('#canvas').boundingBox(),s=await page.evaluate(()=>[Module.canvas.width,Module.canvas.height]);await page.mouse.click(b.x+x*b.width/s[0],b.y+y*b.height/s[1]);await page.waitForTimeout(100);}
const shot=n=>page.screenshot({path:join(out,n+'.png')});
async function fresh(){await api(7);await api(5,1);await api(12,1);await page.waitForTimeout(600);}
try{
 await page.goto(process.env.PVZ_QA_URL||'http://127.0.0.1:8097/');await wait(()=>!document.getElementById('start').disabled,undefined,90000);
 await page.evaluate(async()=>{const FS=Module.FS;if(!FS.analyzePath('/saves/userdata').exists)FS.mkdir('/saves/userdata');const name=new TextEncoder().encode('BehaviorQA'),users=new Uint8Array(16+name.length),v=new DataView(users.buffer);v.setUint32(0,14,true);v.setUint16(4,1,true);v.setUint16(6,name.length,true);users.set(name,8);v.setUint32(8+name.length,1,true);v.setUint32(12+name.length,1,true);FS.writeFile('/saves/userdata/users.dat',users);const profile=new Uint8Array(4096),p=new DataView(profile.buffer);p.setUint32(0,12,true);p.setUint32(4,49,true);FS.writeFile('/saves/userdata/user1.dat',profile);await new Promise((r,j)=>FS.syncfs(false,e=>e?j(e):r()));});
 await page.locator('#start').click();await page.waitForTimeout(12000);await click(400,560);await page.waitForTimeout(4500);await click(260,348);await wait(()=>Module.canvas.width===1024);
 if(process.env.PVZ_QA_CASE!=='mirror'){
 await fresh();for(let i=0;i<19;i++)assert.ok(await api(1,500+i,1+i%7,Math.floor(i/7))>0);await api(4,0);await page.waitForTimeout(1200);await api(4,1);await shot('all-nineteen-native-rigs');results.all19Spawn=true;
 await page.setViewportSize({width:844,height:390});await shot('all-nineteen-phone');await page.setViewportSize({width:1100,height:750});

 await fresh();await api(1,509,3,2);await api(2,4,4,2);await api(2,0,5,2);await api(2,0,6,2);const hp=await zd(0,4),armor=await zd(0,5);await api(4,0);
 await wait(()=>Module._pvz_sandbox_zombie_data(0,6)===1024);await api(4,1);await shot('chomper-holds-same-living-zombie');const held=await zd(0,8);await page.waitForTimeout(300);assert.equal(await zd(0,8),held);results.pauseHeld=true;
 await api(4,0);await wait(()=>Module._pvz_sandbox_zombie_data(0,6)===1025);await page.waitForTimeout(300);await api(4,1);await shot('chomper-returns-whole-zombie');assert.equal(await zd(0,4),hp);assert.equal(await zd(0,5),armor);await api(4,0);
 await wait(()=>Module._pvz_sandbox_zombie_data(0,6)===0);await api(4,1);assert.equal(await zd(0,4),hp);assert.equal(await zd(0,5),armor);assert.equal(await zd(3,0),-1);assert.ok([await zd(1,6),await zd(2,6)].includes(1026));await shot('return-trips-companion-without-damage');results.returnPreservesIdentityArmorHealth=true;
 await fresh();await api(1,509,3,2);await api(2,0,4,2);await api(4,0);await wait(()=>Module._pvz_sandbox_zombie_data(0,6)===1024);await api(3,0,3,2);await wait(()=>Module._pvz_sandbox_zombie_data(0,6)===0);await api(4,1);assert.equal(await zd(0,0),0);assert.equal(await zd(1,0),-1);results.shovelReleasesPassenger=true;

 await fresh();await api(1,508,1,2);await api(2,2,5,2);await api(2,2,6,2);await api(4,0);await wait(()=>[0,1].every(i=>Module._pvz_sandbox_zombie_data(i,6)===1028));await api(4,1);await shot('cactus-staples-two-ankles');results.pairedStaple=true;
 await api(4,0);await wait(()=>[0,1].every(i=>Module._pvz_sandbox_zombie_data(i,6)!==1028));await api(4,1);results.stapleReleases=true;

 await fresh();await api(1,512,1,2);await api(2,2,5,2);await api(2,2,6,2);await api(5,2);await api(4,0);await wait(()=>[0,1].some(i=>Module._pvz_sandbox_zombie_data(i,6)===1027),undefined,30000);await api(4,1);await api(5,1);await shot('butter-slip-no-extra-corn-volley');results.butterSlips=true;

 await fresh();await api(1,513,3,2);await api(2,2,4,2);await api(4,0);await wait(()=>Module._pvz_sandbox_zombie_data(0,6)===1029);const x=await zd(0,2);await page.waitForTimeout(400);await api(4,1);assert.ok(await zd(0,2)>x);await shot('scared-mushroom-misdirects-walker');results.misdirectionChangesWalkDirection=true;

 // Ordinary-width colliders leave room to observe the return. A giant can
 // immediately absorb it within the same tick, which is not a failed emission.
 await fresh();await api(1,510,1,2);await api(2,4,5,2);await api(2,4,7,2);await api(4,0);await wait(()=>{for(let i=0;i<100;i++){const s=Module._pvz_projectile_data(i,0);if(s<0)break;if(s===295)return true;}return false;});await api(4,1);await shot('cabbage-rolls-back-from-rear');results.returningCabbage=true;
 }
 await fresh();await api(1,515,4,2);await api(1,0,1,2);await api(2,4,8,2);await api(4,0);await wait(()=>{for(let i=0;i<100;i++){const s=Module._pvz_projectile_data(i,0);if(s<0)break;if(s>=512&&Module._pvz_projectile_data(i,3)<0)return true;}return false;});await api(4,1);await shot('split-pea-returns-existing-pea');results.splitReturnsExistingPea=true;
 assert.deepEqual(errors,[]);await writeFile(join(out,'report.json'),JSON.stringify({results,errors},null,2));console.log(results);
}catch(e){await shot('failure');await writeFile(join(out,'failure.json'),JSON.stringify({error:String(e),results,errors,zombies:await page.evaluate(()=>Array.from({length:4},(_,i)=>Array.from({length:13},(_,f)=>Module._pvz_sandbox_zombie_data(i,f)))),plants:await page.evaluate(()=>Array.from({length:4},(_,i)=>Array.from({length:11},(_,f)=>Module._pvz_sandbox_plant_data(i,f)))),shots:await page.evaluate(()=>Array.from({length:8},(_,i)=>Array.from({length:10},(_,f)=>Module._pvz_projectile_data(i,f)))),log:await page.evaluate(()=>window.pvzEngineLog)},null,2));throw e;}finally{await browser.close();}
