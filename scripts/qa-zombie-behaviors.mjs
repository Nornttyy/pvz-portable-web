// Exercise the actual shipped engine with disposable storage, not a user's save.
import assert from 'node:assert/strict';
import {mkdir,writeFile} from 'node:fs/promises';
import {join} from 'node:path';
const {chromium}=await import(process.env.PVZ_PLAYWRIGHT||'playwright-core');
const out=process.env.PVZ_QA_OUT||'/tmp/pvz-zombie-behaviors';await mkdir(out,{recursive:true});
const browser=await chromium.launch({executablePath:process.env.PVZ_CHROME||'/Applications/Google Chrome.app/Contents/MacOS/Google Chrome',headless:true,args:['--no-sandbox','--autoplay-policy=no-user-gesture-required']});
const page=await browser.newPage({viewport:{width:1100,height:750}}),results={},errors=[];page.on('pageerror',e=>errors.push(e.message));
const api=(...v)=>page.evaluate(v=>Module._pvz_sandbox_command(...[...v,0,0,0].slice(0,4)),v);
const zd=(i,f)=>page.evaluate(([i,f])=>Module._pvz_sandbox_zombie_data(i,f),[i,f]);
const wait=(f,a,t=20000)=>page.waitForFunction(f,a,{timeout:t});
const shot=name=>page.screenshot({path:join(out,name+'.png')});
const passed=name=>{results[name]=true;console.log(name);};
async function click(x,y){const b=await page.locator('#canvas').boundingBox(),s=await page.evaluate(()=>[Module.canvas.width,Module.canvas.height]);await page.mouse.click(b.x+x*b.width/s[0],b.y+y*b.height/s[1]);await page.waitForTimeout(100);}
async function fresh(){await api(7);await api(5,1);await api(12,1);await page.waitForTimeout(400);}
try{
 await page.goto(process.env.PVZ_QA_URL||'http://127.0.0.1:8097/');await wait(()=>!document.getElementById('start').disabled,undefined,90000);
 await page.evaluate(async()=>{const FS=Module.FS;if(!FS.analyzePath('/saves/userdata').exists)FS.mkdir('/saves/userdata');const name=new TextEncoder().encode('ZombieQA'),users=new Uint8Array(16+name.length),v=new DataView(users.buffer);v.setUint32(0,14,true);v.setUint16(4,1,true);v.setUint16(6,name.length,true);users.set(name,8);v.setUint32(8+name.length,1,true);v.setUint32(12+name.length,1,true);FS.writeFile('/saves/userdata/users.dat',users);const profile=new Uint8Array(4096),p=new DataView(profile.buffer);p.setUint32(0,12,true);p.setUint32(4,49,true);FS.writeFile('/saves/userdata/user1.dat',profile);await new Promise((r,j)=>FS.syncfs(false,e=>e?j(e):r()));});
 await page.locator('#start').click();await page.waitForTimeout(12000);await click(400,560);await page.waitForTimeout(4500);await click(719,27);await wait(()=>Module.canvas.width===1024);

 await fresh();await api(2,1,4,2);await api(2,2,5,2);await api(2,2,6,2);await api(4,0);
 await wait(()=>Module._pvz_sandbox_zombie_data(1,6)===1030);await page.waitForTimeout(200);await api(4,1);const hopX=await zd(1,2),timer=await zd(1,8);assert.ok(await zd(1,11)>0);assert.equal(await zd(2,6),0);await shot('flag-hurries-one-follower');
 await page.waitForTimeout(250);assert.equal(await zd(1,8),timer);await api(4,0);await wait(()=>Module._pvz_sandbox_zombie_data(1,6)===0);await api(4,1);assert.ok(await zd(1,2)<hopX);passed('flagPhysicalHopAndPause');

 await fresh();await api(2,6,6,2);await api(2,2,5,2);await api(4,0);await wait(()=>Module._pvz_sandbox_zombie_data(0,6)===1031);await page.waitForTimeout(200);await api(4,1);await shot('door-rushes-to-front');await api(4,0);await wait(()=>Module._pvz_sandbox_zombie_data(0,6)===0);await api(4,1);assert.ok(await zd(0,2)<await zd(1,2));
 const shield=await zd(0,7),armor=await zd(1,5);await api(1,0,1,2);await api(4,0);await wait(n=>Module._pvz_sandbox_zombie_data(0,7)<n,shield);await api(4,1);assert.equal(await zd(1,5),armor);await shot('door-actually-intercepts-pea');passed('doorPhysicalProtection');

 await fresh();await api(2,7,7,2);await api(2,2,5,2);await api(1,3,3,2);await api(4,0);await wait(()=>Module._pvz_sandbox_zombie_data(0,6)===1032);const skidX=await zd(0,2);await page.waitForTimeout(300);await api(4,1);await shot('football-braking-legs');await api(4,0);await wait(()=>Module._pvz_sandbox_zombie_data(0,6)===1026);await api(4,1);assert.ok(await zd(0,2)<skidX);await shot('football-falls-over');await api(4,0);await wait(()=>Module._pvz_sandbox_zombie_data(0,6)===0);passed('footballSkidAndGetUp');

 await fresh();await api(2,0,5,2);await api(2,2,5,2);await api(4,0);await wait(()=>Module._pvz_sandbox_zombie_data(1,6)===1026);await api(4,1);await shot('lying-down-trips-companion');assert.equal(await zd(1,4),270);passed('restingWalkerTripsCompanion');

 await fresh();await api(2,16,6,2);await api(2,2,6,2);const body=await zd(1,4),helmet=await zd(1,5);await api(4,0);await wait(()=>Module._pvz_sandbox_zombie_data(1,6)===1033);await page.waitForTimeout(450);await api(4,1);await shot('balloon-carries-original-walker');assert.ok(await zd(1,11)>0);assert.equal(await zd(2,0),-1);const carryTimer=await zd(1,8);await page.waitForTimeout(200);assert.equal(await zd(1,8),carryTimer);await api(4,0);
 await wait(()=>Module._pvz_sandbox_zombie_data(1,6)===1035);await shot('balloon-releases-original-walker');await wait(()=>Module._pvz_sandbox_zombie_data(1,6)===0);await api(4,1);assert.equal(await zd(1,4),body);assert.equal(await zd(1,5),helmet);assert.equal(await zd(1,11),0);assert.equal(await zd(2,0),-1);passed('balloonNoClonesPreservesArmorAndHealth');

 await fresh();await api(2,16,6,2);await api(2,2,6,2);await api(4,0);await wait(()=>Module._pvz_sandbox_zombie_data(1,6)===1033);await api(4,1);await api(1,27,0,0);await api(4,0);await wait(()=>{for(let i=0;i<4;i++)if(Module._pvz_sandbox_zombie_data(i,0)===2&&Module._pvz_sandbox_zombie_data(i,6)===0)return true;return false;});await api(4,1);await shot('blown-away-carrier-releases-passenger');passed('carrierRemovalReleasesPassenger');

 await fresh();await api(1,3,3,2);await api(2,21,7,2);await api(4,0);await wait(()=>Module._pvz_sandbox_zombie_data(0,6)===1034);await api(4,1);const startY=await zd(0,3);await api(4,0);await page.waitForTimeout(140);await api(4,1);const midY=await zd(0,3);assert.ok(midY<startY&&midY>startY-100);await shot('ladder-mid-lane-step');await api(4,0);await wait(()=>Module._pvz_sandbox_zombie_data(0,6)!==1034);await api(4,1);assert.equal(await zd(0,1),1);assert.ok(await zd(0,7)>0);passed('ladderSmoothLaneChange');
 await page.setViewportSize({width:844,height:390});await shot('mobile-native-zombie-layout');await page.setViewportSize({width:1100,height:750});
 assert.deepEqual(errors,[]);await writeFile(join(out,'report.json'),JSON.stringify({results,errors},null,2));console.log('Zombie behavior QA passed',results);
}catch(e){await shot('failure');await writeFile(join(out,'failure.json'),JSON.stringify({error:String(e),results,errors,zombies:await page.evaluate(()=>Array.from({length:5},(_,i)=>Array.from({length:13},(_,f)=>Module._pvz_sandbox_zombie_data(i,f)))),log:await page.evaluate(()=>window.pvzEngineLog)},null,2));throw e;}finally{await browser.close();}
