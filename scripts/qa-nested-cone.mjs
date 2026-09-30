// Real native battles and adventure UI, using an isolated browser profile.
import assert from 'node:assert/strict';
import {mkdir,writeFile} from 'node:fs/promises';
import {join} from 'node:path';
const {chromium}=await import(process.env.PVZ_PLAYWRIGHT||'playwright-core');
const out=process.env.PVZ_QA_OUT||'/tmp/pvz-nested-cone-qa';await mkdir(out,{recursive:true});
const browser=await chromium.launch({executablePath:process.env.PVZ_CHROME||'/Applications/Google Chrome.app/Contents/MacOS/Google Chrome',headless:true,args:['--no-sandbox']});
const page=await browser.newPage({viewport:{width:1100,height:750},hasTouch:true}),errors=[],results={};page.on('pageerror',e=>errors.push(e.stack||e.message));
async function point(x,y){const b=await page.locator('#canvas').boundingBox(),s=await page.evaluate(()=>[Module.canvas.width,Module.canvas.height]);return [b.x+x*b.width/s[0],b.y+y*b.height/s[1]];}
async function click(x,y){await page.mouse.click(...await point(x,y));await page.waitForTimeout(100);}
async function shot(name){await page.screenshot({path:join(out,name+'.png')});}
async function api(...a){return page.evaluate(a=>Module._pvz_sandbox_command(...[...a,0,0,0,0].slice(0,4)),a);}
async function pd(f){return page.evaluate(f=>Module._pvz_sandbox_plant_data(0,f),f);}
async function zd(f){return page.evaluate(f=>Module._pvz_sandbox_zombie_data(0,f),f);}
async function ad(i,f){return page.evaluate(([i,f])=>Module._pvz_adventure_power_data(i,f),[i,f]);}
async function seed(i,f){return page.evaluate(([i,f])=>Module._pvz_adventure_seed_data(i,f),[i,f]);}
async function boot(initial=false){
 await page.goto(process.env.PVZ_QA_URL||'http://127.0.0.1:8097/');await page.waitForFunction(()=>!document.getElementById('start').disabled,{},{timeout:90000});
 if(initial)await page.evaluate(async()=>{
  const FS=Module.FS;if(!FS.analyzePath('/saves/userdata').exists)FS.mkdir('/saves/userdata');
  const name=new TextEncoder().encode('NestedQA'),users=new Uint8Array(16+name.length),u=new DataView(users.buffer);
  u.setUint32(0,14,true);u.setUint16(4,1,true);u.setUint16(6,name.length,true);users.set(name,8);u.setUint32(8+name.length,1,true);u.setUint32(12+name.length,1,true);FS.writeFile('/saves/userdata/users.dat',users);
  const profile=new Uint8Array(4096),p=new DataView(profile.buffer);p.setUint32(0,12,true);p.setUint32(4,6,true);FS.writeFile('/saves/userdata/user1.dat',profile);await new Promise((r,j)=>FS.syncfs(false,e=>e?j(e):r()));
 });
 await page.locator('#start').click();await page.waitForTimeout(12000);await click(400,560);await page.waitForTimeout(4500);
}
try{
 await boot(true);await click(260,348);await page.waitForFunction(()=>Module.canvas.width===1024,{},{timeout:20000});
 if(!process.env.PVZ_QA_CONE_ONLY){
 // The new card is reachable through the actual five-column native sidebar.
 await click(125,533);await click(130,236);await click(544,330);assert.equal(await pd(0),507);assert.equal(await pd(7),3);await shot('new-card-planted');
 await api(5,4);await api(4,0);await page.waitForFunction(()=>Module._pvz_sandbox_plant_data(0,8)===16,{},{timeout:14000});await api(4,1);await shot('potato-layer-3');
 for(let remaining=2;remaining>=0;--remaining){
  assert.ok(await api(2,0,2,2)>0);await api(4,0);
  await page.waitForFunction(n=>n?Module._pvz_sandbox_plant_data(0,7)===n:Module._pvz_sandbox_plant_data(0,0)<0,remaining,{timeout:8000});await api(4,1);
  if(remaining){
   assert.equal(await pd(3),300);assert.equal(await pd(8),0);assert.ok(await pd(9)>450);await shot('potato-reburied-'+remaining);
   const frozen=[await pd(7),await pd(9)];await page.waitForTimeout(200);assert.deepEqual([await pd(7),await pd(9)],frozen);
   await api(4,0);await page.waitForFunction(()=>Module._pvz_sandbox_plant_data(0,8)===16,{},{timeout:6000});await api(4,1);await shot('potato-layer-'+remaining);
  }
 }
 assert.equal(await api(9),0);results.threeRealExplosions=true;await shot('third-explosion-finished');
 }
 await api(7);await api(1,0,0,2);await api(2,2,8,2);await shot('cone-intact');
 await page.evaluate(()=>{window.coneTrace=[];const sample=()=>{const a=[0,2,4,5,6,8,9].map(f=>Module._pvz_sandbox_zombie_data(0,f));const previous=window.coneTrace.at(-1);if(!previous||a.some((v,i)=>i!==1&&v!==previous[i]))window.coneTrace.push(a);if(window.coneTrace.length<1000&&Module._pvz_sandbox_zombie_data(0,0)>=0)window.coneTraceFrame=requestAnimationFrame(sample);};window.coneTraceFrame=requestAnimationFrame(sample);});
 await api(5,4);await api(4,0);
 await page.waitForFunction(()=>Module._pvz_sandbox_zombie_data(0,8)>0,{},{timeout:23000});await api(4,1);await api(5,1);await api(3,0,0,2);
 const x=await zd(2),health=await zd(4);assert.equal(await zd(5),0);assert.equal(await zd(9),0);
 for(let i=0;i<30;++i){await api(13);await page.waitForTimeout(20);}await shot('cone-playing-dead');assert.equal(await zd(2),x);
 const frozen=await zd(8);await page.waitForTimeout(200);assert.equal(await zd(8),frozen);
 await api(4,0);await page.waitForTimeout(850);await api(4,1);assert.equal(await zd(2),x);assert.ok(await zd(4)<=health);await shot('cone-still-targetable');
 await api(4,0);await page.waitForFunction(()=>Module._pvz_sandbox_zombie_data(0,8)===0,{},{timeout:6000});await page.waitForTimeout(400);await api(4,1);
 assert.equal(await zd(9),100);assert.ok(await zd(2)<x);assert.ok(await zd(4)<=health);await shot('cone-back-up');results.feignAndStand=true;
 await page.evaluate(()=>cancelAnimationFrame(window.coneTraceFrame));
 await page.setViewportSize({width:844,height:390});await page.waitForTimeout(250);await shot('mobile-new-content');
 await page.setViewportSize({width:1100,height:750});await page.waitForTimeout(250);await api(15);await page.waitForTimeout(800);await click(560,135);await page.waitForTimeout(15000);
 await page.waitForFunction(()=>Module._pvz_adventure_power_data(-1,5)===1,{},{timeout:40000});
 assert.equal(await seed(4,0),507);assert.equal(await seed(4,1),75);assert.equal(await ad(-1,3),6);await shot('adventure-1-6-new-card');
 const deadline=Date.now()+35000;
 while(await ad(-1,2)<75){assert.ok(Date.now()<deadline);const pos=[await ad(0,10),await ad(0,11)];if(pos[0]>=0)await click(...pos);await page.waitForTimeout(100);}
 await click(await seed(4,5)+25,await seed(4,6)+35);await click(320,330);
 assert.equal(await ad(0,0),507);assert.equal(await ad(0,14),3);await shot('adventure-nested-potato');
 await click(748,14);const state=[await ad(0,0),await ad(0,3),await ad(0,14),await ad(-1,2),await ad(-1,3)];
 await click(400,401);await page.waitForTimeout(250);await click(305,394);await page.waitForFunction(()=>Module._pvz_adventure_power_data(-1,5)===-1,{},{timeout:10000});
 await page.evaluate(()=>new Promise((r,j)=>Module.FS.syncfs(false,e=>e?j(e):r())));
 await boot();await click(560,135);await page.waitForTimeout(15000);await page.waitForFunction(()=>Module._pvz_adventure_power_data(-1,5)===1,{},{timeout:40000});
 assert.deepEqual([await ad(0,0),await ad(0,3),await ad(0,14),await ad(-1,2),await ad(-1,3)],state);await shot('adventure-restored');results.adventureAndSave=true;
 assert.deepEqual(errors,[]);await writeFile(join(out,'report.json'),JSON.stringify({results,errors},null,2));console.log('Nested potato and cone QA passed',results);
}catch(e){await shot('failure');const diagnostics=await page.evaluate(()=>({trace:window.coneTrace,log:window.pvzEngineLog}));await writeFile(join(out,'failure.json'),JSON.stringify({error:String(e),errors,...diagnostics},null,2));console.log('CONE',diagnostics.trace);console.log('ERRORS',errors);throw e;}
finally{await browser.close();}
