// Isolated Chromium only. Synthetic QA profile never touches a player's browser.
import assert from 'node:assert/strict';
import {mkdir,writeFile} from 'node:fs/promises';
import {join} from 'node:path';
const {chromium}=await import(process.env.PVZ_PLAYWRIGHT||'playwright-core');
const out=process.env.PVZ_QA_OUT||'/tmp/pvz-abstract-qa';await mkdir(out,{recursive:true});
const browser=await chromium.launch({executablePath:process.env.PVZ_CHROME||'/Applications/Google Chrome.app/Contents/MacOS/Google Chrome',headless:true,args:['--no-sandbox','--autoplay-policy=no-user-gesture-required']});
const page=await browser.newPage({viewport:{width:1100,height:750},hasTouch:true});const errors=[],logs=[];
page.on('pageerror',e=>errors.push(e.message));page.on('console',m=>logs.push(m.text()));
async function point(x,y){const b=await page.locator('#canvas').boundingBox();const s=await page.evaluate(()=>[Module.canvas.width,Module.canvas.height]);return [b.x+x*b.width/s[0],b.y+y*b.height/s[1]];}
async function click(x,y){await page.mouse.click(...await point(x,y));await page.waitForTimeout(140);}
async function tap(x,y){await page.touchscreen.tap(...await point(x,y));await page.waitForTimeout(140);}
async function shot(name){await page.screenshot({path:join(out,name+'.png')});}
async function api(...args){return page.evaluate(v=>Module._pvz_sandbox_command(...[...v,0,0,0,0].slice(0,4)),args);}
async function data(){return page.evaluate(()=>{const list=[];for(let i=0;i<180;i++){const type=Module._pvz_sandbox_plant_data(i,0);if(type<0)break;list.push(Array.from({length:7},(_,f)=>Module._pvz_sandbox_plant_data(i,f)));}return list;});}
async function boot(seedLevel=0){await page.goto(process.env.PVZ_QA_URL||'http://127.0.0.1:8097/');await page.waitForFunction(()=>!document.getElementById('start').disabled,{},{timeout:90000});
 if(seedLevel)await page.evaluate(async level=>{
  const FS=Module.FS;if(!FS.analyzePath('/saves/userdata').exists)FS.mkdir('/saves/userdata');
  const name=new TextEncoder().encode('PowerQA'),users=new Uint8Array(16+name.length),u=new DataView(users.buffer);
  u.setUint32(0,14,true);u.setUint16(4,1,true);u.setUint16(6,name.length,true);users.set(name,8);u.setUint32(8+name.length,1,true);u.setUint32(12+name.length,1,true);
  FS.writeFile('/saves/userdata/users.dat',users);const profile=new Uint8Array(4096),p=new DataView(profile.buffer);p.setUint32(0,12,true);p.setUint32(4,level,true);FS.writeFile('/saves/userdata/user1.dat',profile);
  await new Promise((resolve,reject)=>FS.syncfs(false,e=>e?reject(e):resolve()));
 },seedLevel);
 await page.locator('#start').click();await page.waitForTimeout(12000);await click(400,560);await page.waitForTimeout(4500);}

const results={};
async function zd(i,f){return page.evaluate(([i,f])=>Module._pvz_sandbox_zombie_data(i,f),[i,f]);}
async function ad(i,f){return page.evaluate(([i,f])=>Module._pvz_adventure_power_data(i,f),[i,f]);}
async function sd(i,f){return page.evaluate(([i,f])=>Module._pvz_adventure_seed_data(i,f),[i,f]);}
async function pauseAt(f,v){await page.waitForFunction(([f,v])=>Module._pvz_sandbox_plant_data(0,f)===v,[f,v],{timeout:18000});await api(4,1);}
try {
 await boot(process.env.PVZ_QA_WAVE2?9:8);
 await click(260,348);await page.waitForFunction(()=>Module.canvas.width===1024,{},{timeout:20000});
 await tap(234,153);await tap(464,330);assert.equal((await data())[0][0],504);await shot('seven-cards');
 if(process.env.PVZ_QA_WAVE2){
  await api(7);await api(1,501,3,2);await api(2,0,3,2);const hp=await zd(0,4);await shot('nut-calm');await api(4,0);await pauseAt(4,1);
  const x=await zd(0,2);for(let i=0;i<24;++i){await api(13);await page.waitForTimeout(20);}await shot('nut-angry-counter');
  assert.equal(await zd(0,4),hp-80);assert.ok(await zd(0,2)<=x+1);assert.ok((await data())[0][6]>250);results.nutNoPush=true;
  await api(6);await shot('nut-angry-face');
  await api(4,0);await pauseAt(4,0);await shot('nut-calm-again');
  await api(7);await api(1,505,1,2);await api(1,506,2,2);await api(2,0,6,2);await api(4,0);
  await page.waitForFunction(()=>Module._pvz_projectile_data(1,0)>=0,{},{timeout:5000});await api(4,1);
  const styles=await page.evaluate(()=>[0,1].map(i=>Module._pvz_projectile_data(i,0)));assert.ok(styles.includes(19)&&styles.includes(0));await shot('retreat-ice-and-echo');
  await page.evaluate(()=>{window.biggestIcePush=0;let last=Module._pvz_sandbox_zombie_data(0,2);const poll=()=>{const x=Module._pvz_sandbox_zombie_data(0,2);window.biggestIcePush=Math.max(window.biggestIcePush,x-last);last=x;if(window.biggestIcePush<30)requestAnimationFrame(poll);};requestAnimationFrame(poll);});
  await api(4,0);await page.waitForFunction(()=>window.biggestIcePush>=30,{},{timeout:6000});await api(4,1);results.icePush=true;results.echo=true;
  await api(7);await api(1,0,0,2);await api(2,4,5,2);const helmet=await zd(0,5);await api(4,0);
  await page.waitForFunction(hp=>Module._pvz_sandbox_zombie_data(0,5)<hp,helmet,{timeout:5000});const hitX=await zd(0,2);await page.waitForTimeout(300);await api(4,1);
  assert.ok(await zd(0,2)<=hitX);assert.equal(await zd(0,8),0);assert.equal(await zd(0,9),100);await shot('ordinary-hit-no-retreat');results.bucketNoRetreat=true;
  await api(7);await api(2,0,6,1);assert.equal(await zd(0,9),100);await api(2,1,6,2);assert.equal(await zd(0,9),150);
  await api(2,1,6,1);assert.equal(await zd(0,9),150);await api(4,0);await page.waitForTimeout(300);await api(4,1);await shot('haste-flags');results.flagAura=true;
  await api(15);await page.waitForTimeout(900);await click(560,135);await page.waitForTimeout(15000);
  for(const id of [5,7,0,1,3,2])await click(47+(id%8)*53,163+Math.floor(id/8)*73);await click(232,566);
  await page.waitForFunction(()=>Module._pvz_adventure_power_data(-1,5)===1,{},{timeout:20000});
  assert.equal(await sd(0,0),505);assert.equal(await sd(0,1),175);assert.equal(await sd(1,0),506);assert.equal(await sd(1,1),200);
  await page.setViewportSize({width:844,height:390});await shot('new-native-adventure-cards');results.adventureCards=true;
 }
 if(!process.env.PVZ_QA_UI_ONLY&&!process.env.PVZ_QA_WAVE2){
 await api(7);await api(1,501,3,2);await api(2,0,3,2);const hp=await zd(0,4);await api(4,0);await pauseAt(4,1);
 for(let i=0;i<20;++i){await api(13);await page.waitForTimeout(18);}await shot('nut-bumps-forward');
 const nutFrozen=await data();await page.waitForTimeout(250);assert.deepEqual(await data(),nutFrozen);
 await api(4,0);await pauseAt(4,0);assert.ok(await zd(0,4)<hp,'walnut bump causes real damage');assert.equal(await zd(0,1),2);assert.equal((await data())[0][1],3);await shot('nut-back-at-anchor');
 results.bump={before:hp,after:await zd(0,4)};
 await api(7);await api(1,504,1,2);await api(2,0,4,2);const hp2=await zd(0,4);await api(4,0);await pauseAt(4,1);
 for(let i=0;i<20;++i){await api(13);await page.waitForTimeout(18);}await shot('self-thrower-flight');
 const frozen=await data();await page.waitForTimeout(250);assert.deepEqual(await data(),frozen);
 await api(4,0);await pauseAt(4,0);assert.ok(await zd(0,4)<hp2);assert.equal((await data())[0][0],504);await shot('self-thrower-returns');
 results.selfThrower={before:hp2,after:await zd(0,4)};
 await api(7);await api(1,0,0,2);await api(2,5,6,2);await shot('phone-rig-idle');await api(4,0);await api(5,2);
 await page.waitForFunction(()=>Module._pvz_sandbox_zombie_data(0,7)===0,{},{timeout:20000});
 await api(3,0,0,2);await api(4,1);await shot('broken-phone-rage');
 const wounded=await zd(0,4);await api(4,0);
 await page.waitForFunction(()=>Module._pvz_sandbox_zombie_data(0,7)===100,{},{timeout:12000});await api(4,1);
 assert.equal(await zd(0,4),wounded,'backup phone does not heal zombie');await shot('backup-phone');
 results.phone={health:wounded,shield:await zd(0,7)};
 await page.setViewportSize({width:844,height:390});await page.waitForTimeout(300);await shot('mobile-abstract');
 await page.setViewportSize({width:1100,height:750});await api(15);await page.waitForTimeout(900);
 await click(560,135);await page.waitForTimeout(15000);await shot('adventure-new-card');
 for(const id of [0,1,3,5,2])await click(47+(id%8)*53,163+Math.floor(id/8)*73);
 await click(471,163);await click(232,566);
 await page.waitForFunction(()=>Module._pvz_adventure_power_data(-1,5)===1,{},{timeout:20000});
 assert.equal(await sd(5,0),504);assert.equal(await sd(5,1),125);assert.equal(await sd(5,2),300);
 const until=Date.now()+65000;
 while(await ad(-1,2)<125){assert.ok(Date.now()<until);const x=await ad(0,10),y=await ad(0,11);if(x>=0)await click(x,y);else await page.waitForTimeout(180);}
 const sun=await ad(-1,2);await click(await sd(5,5)+25,await sd(5,6)+35);await click(240,330);
 assert.equal(await ad(0,0),504);assert.equal(await ad(-1,2),sun-125);await shot('adventure-original-planted');results.adventureNewCard=true;
 await click(748,14);const cooldown=await sd(5,3);await page.waitForTimeout(400);assert.equal(await sd(5,3),cooldown);
 await click(400,401);await page.waitForTimeout(450);await click(305,394);
 await page.waitForFunction(()=>Module._pvz_adventure_power_data(-1,5)===-1,{},{timeout:10000});
 await boot();await click(560,135);await page.waitForFunction(()=>Module._pvz_adventure_power_data(-1,5)===1,{},{timeout:20000});
 await page.waitForTimeout(250);await click(280,371);assert.equal(await ad(0,0),504);assert.equal(await ad(-1,3),8);await shot('new-card-save-resumed');results.saveResume=true;
 }
 assert.deepEqual(errors,[]);assert.ok(!logs.some(s=>/Aborted\(|unreachable|memory access out of bounds/.test(s)));
 await writeFile(join(out,'report.json'),JSON.stringify({results,errors,logs},null,2));console.log('Abstract QA passed',results);
}catch(e){await shot('failure');console.log('PLANTS',await data(),'ZOMBIE',await Promise.all(Array.from({length:9},(_,f)=>zd(0,f))));console.log('LOG',await page.evaluate(()=>window.pvzEngineLog));throw e;}
finally{await browser.close();}
