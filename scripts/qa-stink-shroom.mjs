// Isolated real-engine checks. Commands mutate sandbox only; adventure uses UI.
import assert from 'node:assert/strict';
import {mkdir,writeFile} from 'node:fs/promises';
import {join} from 'node:path';
const {chromium}=await import(process.env.PVZ_PLAYWRIGHT||'playwright-core');
const out=process.env.PVZ_QA_OUT||'/tmp/pvz-stink-qa';await mkdir(out,{recursive:true});
const browser=await chromium.launch({executablePath:'/Applications/Google Chrome.app/Contents/MacOS/Google Chrome',headless:true,args:['--no-sandbox','--autoplay-policy=no-user-gesture-required']});
const page=await browser.newPage({viewport:{width:1100,height:750},hasTouch:true}),results={},errors=[];page.on('pageerror',e=>errors.push(e.message));
const wait=(f,a,t=30000)=>page.waitForFunction(f,a,{timeout:t});
const api=(...a)=>page.evaluate(a=>Module._pvz_sandbox_command(...[...a,0,0,0,0].slice(0,4)),a);
const data=(...a)=>page.evaluate(a=>Module._pvz_stink_data(...a),a);
const seed=(i,f)=>page.evaluate(([i,f])=>Module._pvz_adventure_seed_data(i,f),[i,f]);
const plants=()=>page.evaluate(()=>Array.from({length:40},(_,i)=>Array.from({length:7},(_,f)=>Module._pvz_stink_data(0,i,f))).filter(x=>x[0]>=0));
const snap=async n=>{await page.mouse.move(0,0);await page.waitForTimeout(120);await page.screenshot({path:join(out,n+'.png')});};
async function click(x,y){const b=await page.locator('#canvas').boundingBox(),s=await page.evaluate(()=>[Module.canvas.width,Module.canvas.height]);await page.mouse.click(b.x+x*b.width/s[0],b.y+y*b.height/s[1]);await page.waitForTimeout(100);}
async function step(n){for(let i=0;i<n;++i){const tick=await data(3,0,0);await api(13);await wait(t=>Module._pvz_stink_data(3,0,0)>t,tick);}}
async function placeCard(i,x,y){await click(await seed(i,5)+25,await seed(i,6)+35);await click(x,y);}
try{
 await page.goto(process.env.PVZ_QA_URL||'http://127.0.0.1:8097/');await wait(()=>!document.getElementById('start').disabled,undefined,90000);
 await page.evaluate(async()=>{
  const FS=Module.FS;if(!FS.analyzePath('/saves/userdata').exists)FS.mkdir('/saves/userdata');
  const name=new TextEncoder().encode('StinkQA'),users=new Uint8Array(16+name.length),u=new DataView(users.buffer);
  u.setUint32(0,14,true);u.setUint16(4,1,true);u.setUint16(6,name.length,true);users.set(name,8);u.setUint32(8+name.length,1,true);u.setUint32(12+name.length,1,true);FS.writeFile('/saves/userdata/users.dat',users);
  const profile=new Uint8Array(4096),p=new DataView(profile.buffer);p.setUint32(0,12,true);p.setUint32(4,18,true);FS.writeFile('/saves/userdata/user1.dat',profile);
  await new Promise((r,j)=>FS.syncfs(false,e=>e?j(e):r()));
 });
 await page.locator('#start').click();await page.waitForTimeout(12000);await click(400,560);await page.waitForTimeout(4500);
 await click(370,455);await click(208,366);await click(155,205);await snap('almanac');await click(690,580);await page.waitForTimeout(1000);
 if(!process.env.PVZ_QA_ADVENTURE_ONLY){
  await click(260,348);await wait(()=>Module.canvas.width===1024);await api(8,0);await api(4,1);await api(12,1);
  assert.equal(await api(1,527,2,2),1);assert.equal(await api(1,520,2,1),1);assert.equal(await api(1,520,0,0),1);
  for(let i=0;i<12;++i)assert.equal(await api(2,4,5,2),1);
  await page.evaluate(()=>{window.stinkTrace=[];let old=0;window.stinkMonitor=setInterval(()=>{
   const counts=[0,1,2,3].map(f=>Module._pvz_stink_data(2,0,f)),total=counts.reduce((a,b)=>a+b,0);
   if(total!==old){old=total;window.stinkTrace.push({tick:Module._pvz_stink_data(3,0,0),counts});}
  },5);});
  await api(4,0);await wait(()=>window.stinkTrace.length>0);await api(4,1);results.firstHit=await page.evaluate(()=>window.stinkTrace[0]);await snap('brown-spray');
  results.stunned=[];for(let i=0;i<12;++i)if(await data(1,i,0)>0)results.stunned.push(i);assert.ok(results.stunned.length>0);
  await snap('nauseated-zombies');
  await api(4,0);await wait(()=>window.stinkTrace.length>=4);await api(4,1);results.cadence=await page.evaluate(()=>window.stinkTrace);
  for(let i=1;i<results.cadence.length;++i){const delta=results.cadence[i].tick-results.cadence[i-1].tick;assert.ok(delta>=195&&delta<=205,'native two-second interval: '+delta);}
  await page.evaluate(()=>clearInterval(window.stinkMonitor));
  await api(6); // Keep the exposure test independent of the attacking zombie pile.
  await api(1,527,3,2); // Neighbouring Stink Shrooms must both remain immune.
  await api(1,501,1,2);await api(1,6,1,1);await api(1,500,1,3);
  await api(5,4);await api(4,0);await wait(()=>Module._pvz_stink_data(0,5,0)===2000);await api(4,1);await api(5,1);
  assert.equal(await data(0,1,1),1);assert.equal(await data(0,2,1),0);assert.equal(await data(0,0,1),0);
  results.exposure=await plants();await snap('neighbour-nausea');
  assert.equal(results.exposure.filter(p=>p[2]===527).length,2);
  assert.ok(results.exposure.filter(p=>p[2]===527).every(p=>p[0]===0&&p[1]===0),'all Stink Shrooms immune, not only their own aura');
  results.outcomes=await page.evaluate(()=>[0,1,2,3].map(f=>Module._pvz_stink_data(2,0,f)));assert.ok(results.outcomes[1]>0&&results.outcomes[2]>0&&results.outcomes[3]>0);
  await api(6); // Remove combat noise before measuring production with exact native ticks.
  while(await data(0,1,5)<300||await data(0,2,5)<400)await step(10);
  const before=await plants();await step(100);const after=await plants();
  results.production={near:before[1][5]-after[1][5],far:before[2][5]-after[2][5]};assert.deepEqual(results.production,{near:140,far:200});
  await api(3,0,3,2);await api(3,0,2,2);assert.equal(await data(0,0,1),0);results.removalRestores=true;await snap('recovered-colours');
  // A daytime sleeping shroom does not expose neighbours or spray.
  await api(7);await api(4,1);await api(12,0);await api(1,527,2,2);await api(1,520,2,1);await api(5,4);await api(4,0);await page.waitForTimeout(6000);await api(4,1);assert.equal(await data(0,1,0),0);await snap('sleeping');
  await page.setViewportSize({width:844,height:390});await snap('phone');await page.setViewportSize({width:1100,height:750});
  await api(15);await page.waitForTimeout(1800);console.log('Sandbox spray, 2-second cadence, all hit effects, exposure and exact 70% production passed');
 }
 await click(560,135);await page.waitForTimeout(15000);
 for(const base of [10,1,8,3,4,5]){const slot=base>=8?base+2:base;await click(47+slot%9*53,163+Math.floor(slot/9)*73);}
 await snap('adventure-chooser');await click(258,566);await wait(()=>Module._pvz_adventure_power_data(-1,5)===1);
 assert.deepEqual(await page.evaluate(()=>[0,1,2].map(f=>Module._pvz_adventure_seed_data(0,f))),[527,75,750]);
 await placeCard(1,120,330);let puffs=0;const deadline=Date.now()+90000;
 while(Date.now()<deadline){
  const sun=await page.evaluate(()=>[Module._pvz_sun_data(0,1),Module._pvz_sun_data(0,2)]);if(sun[0]>=0)await click(...sun);
  if(puffs<4&&await seed(2,4)){await placeCard(2,280,[130,230,430,530][puffs]);++puffs;}
  if(await page.evaluate(()=>Module._pvz_adventure_power_data(-1,2)>=75)&&await seed(0,4))break;
  await page.waitForTimeout(300);
 }
 await page.mouse.click(500,300,{button:'right'});await placeCard(0,240,330);assert.ok(await seed(0,3)>700);
 await wait(()=>Array.from({length:20},(_,i)=>Module._pvz_stink_data(0,i,1)).some(n=>n===1),undefined,45000);
 await click(748,14);results.saveBefore=await plants();assert.ok(results.saveBefore.some(p=>p[1]===1));await snap('adventure-pause');
 await click(400,401);await page.waitForTimeout(350);await click(305,394);await page.waitForTimeout(1800);await click(560,135);await wait(()=>Module._pvz_adventure_power_data(-1,5)===1);
 results.saveAfter=await plants();assert.deepEqual(results.saveAfter,results.saveBefore);await click(280,371);await page.waitForTimeout(1200);await snap('adventure-resumed');
 assert.deepEqual(errors,[]);await writeFile(join(out,'report.json'),JSON.stringify({results,errors},null,2));console.log('Stink shroom QA passed',out,results);
}catch(e){console.error('QA failure:',e);await snap('failure').catch(()=>{});await writeFile(join(out,'failure.json'),JSON.stringify({error:String(e),results,errors,plants:await plants().catch(()=>[]),log:await page.evaluate(()=>window.pvzEngineLog).catch(()=>[])},null,2));throw e;}
finally{await browser.close();}
