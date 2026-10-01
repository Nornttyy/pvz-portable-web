// Real browser/engine QA with a disposable profile; never touches player saves.
import assert from 'node:assert/strict';
import {mkdir,writeFile} from 'node:fs/promises';
import {join} from 'node:path';
const {chromium}=await import(process.env.PVZ_PLAYWRIGHT||'playwright-core');
const out=process.env.PVZ_QA_OUT||'/tmp/pvz-nuke-shroom-qa';await mkdir(out,{recursive:true});
const browser=await chromium.launch({executablePath:'/Applications/Google Chrome.app/Contents/MacOS/Google Chrome',headless:true,args:['--no-sandbox','--autoplay-policy=no-user-gesture-required']});
const page=await browser.newPage({viewport:{width:1100,height:750},hasTouch:true}),results={},errors=[];page.on('pageerror',e=>errors.push(e.message));
const wait=(f,a,t=30000)=>page.waitForFunction(f,a,{timeout:t});
const api=(...a)=>page.evaluate(a=>Module._pvz_sandbox_command(...[...a,0,0,0,0].slice(0,4)),a);
const holes=()=>page.evaluate(()=>Array.from({length:Module._pvz_nuke_data(-1,0)},(_,i)=>Array.from({length:6},(_,f)=>Module._pvz_nuke_data(i,f))));
const seed=(i,f)=>page.evaluate(([i,f])=>Module._pvz_adventure_seed_data(i,f),[i,f]);
const snap=async n=>{await page.mouse.move(0,0);await page.waitForTimeout(120);await page.screenshot({path:join(out,n+'.png')});};
async function click(x,y){const b=await page.locator('#canvas').boundingBox(),s=await page.evaluate(()=>[Module.canvas.width,Module.canvas.height]);await page.mouse.click(b.x+x*b.width/s[0],b.y+y*b.height/s[1]);await page.waitForTimeout(100);}
async function quit(){await click(748,14);await page.waitForTimeout(300);await click(400,401);await page.waitForTimeout(300);await click(305,394);await wait(()=>Module._pvz_adventure_power_data(-1,5)===-1);await page.waitForTimeout(1800);}
try{
 await page.goto(process.env.PVZ_QA_URL||'http://127.0.0.1:8097/');await wait(()=>!document.getElementById('start').disabled,undefined,90000);
 await page.evaluate(async()=>{
  const FS=Module.FS;if(!FS.analyzePath('/saves/userdata').exists)FS.mkdir('/saves/userdata');
  const name=new TextEncoder().encode('NukeQA'),users=new Uint8Array(16+name.length),u=new DataView(users.buffer);
  u.setUint32(0,14,true);u.setUint16(4,1,true);u.setUint16(6,name.length,true);users.set(name,8);u.setUint32(8+name.length,1,true);u.setUint32(12+name.length,1,true);FS.writeFile('/saves/userdata/users.dat',users);
  const profile=new Uint8Array(4096),p=new DataView(profile.buffer);p.setUint32(0,12,true);p.setUint32(4,38,true);FS.writeFile('/saves/userdata/user1.dat',profile);
  await new Promise((r,j)=>FS.syncfs(false,e=>e?j(e):r()));
 });
 await page.locator('#start').click();await page.waitForTimeout(12000);await click(400,560);await page.waitForTimeout(4500);
 // Check the actual almanac, including the complete new glyphs and native footer.
 await click(370,455);await click(208,366);await click(48+8*46,123+76);await snap('almanac');
 await click(690,580);await page.waitForTimeout(1000);
 if(!process.env.PVZ_QA_ADVENTURE_ONLY){
 await click(260,348);await wait(()=>Module.canvas.width===1024);await api(8,0);await api(4,1);await api(12,0);
 assert.equal(await api(1,526,4,2),1);assert.equal(await api(1,15,2,2),1);
 await api(4,0);await page.waitForTimeout(400);await api(4,1);await snap('sleeping-native-comparison');assert.equal((await holes()).length,0);
 await api(3,0,2,2);await api(12,1);await api(4,0);await page.waitForTimeout(250);await api(4,1);await snap('green-energy-windup');
 for(const row of [0,4])assert.equal(await api(2,32,8,row),1);
 await api(1,501,3,1);await api(1,501,5,3);await api(1,501,1,4);
 await api(4,0);await wait(()=>Module._pvz_nuke_data(-1,0)===9);await api(4,1);
 results.first=await holes();assert.equal(results.first[0][3],4);assert.ok(results.first.every(h=>h[0]>=3&&h[0]<=5&&h[1]>=1&&h[1]<=3));
 assert.equal(await api(9),1);results.farEnemyHP=await page.evaluate(()=>[Module._pvz_sandbox_zombie_data(0,4),Module._pvz_sandbox_zombie_data(1,4)]);assert.deepEqual(results.farEnemyHP,[4200,4200]);
 await snap('green-first-pulse');
 for(const h of results.first)assert.equal(await api(1,501,h[0],h[1]),-4);
 const frozen=await holes();await page.waitForTimeout(400);assert.deepEqual(await holes(),frozen);
 results.pulses=1;
 for(let remaining=3;remaining>=0;--remaining){await api(4,0);await wait(n=>Module._pvz_nuke_data(0,3)===n,remaining);await api(4,1);++results.pulses;
  if(remaining===3)assert.equal(await page.evaluate(()=>Module._pvz_sandbox_zombie_data(0,4)),2400);
  if(remaining===0)await snap('fifth-pulse');
 }
 await api(4,0);await page.waitForTimeout(2500);await api(4,1);await snap('three-by-three-crater');assert.equal((await holes()).length,9);
 await page.setViewportSize({width:844,height:390});await snap('phone');await page.setViewportSize({width:1100,height:750});
 // Full nine-cell footprint in corners, including mixed land/water terrain.
 results.edges=[];
 for(const [map,c,r]of [[0,0,0],[0,8,4],[1,8,5],[1,4,2]]){
  await api(8,map);await api(4,1);await api(12,1);if(map===1&&r===2)assert.equal(await api(1,16,c,r),1);
  assert.equal(await api(1,526,c,r),1);await api(4,0);await wait(()=>Module._pvz_nuke_data(-1,0)===9);await api(4,1);
  const h=await holes();assert.equal(h.length,9);assert.equal(new Set(h.map(x=>x[0]+','+x[1])).size,9);assert.ok(h.every(x=>x[0]>=0&&x[0]<9&&x[1]>=0&&x[1]<(map?6:5)));
  results.edges.push({map,c,r,cells:h.map(x=>x.slice(0,2))});
  if(map){await api(4,0);await page.waitForTimeout(6500);await api(4,1);await snap('pool-'+r);}
 }
 console.log('Sandbox explosions, all nine blocked cells, edge and pool checks passed');
 await api(15);await page.waitForTimeout(1800);
 }
 await click(560,135);await page.waitForTimeout(15000);
 for(const base of [15,1,8,16,3,0]){const slot=base>=8?base+2:base;await click(47+slot%9*53,163+Math.floor(slot/9)*73);}
 await snap('adventure-chooser');await click(258,566);await wait(()=>Module._pvz_adventure_power_data(-1,5)===1);
 results.card=await page.evaluate(()=>[0,1,2].map(f=>Module._pvz_adventure_seed_data(0,f)));assert.deepEqual(results.card,[526,250,3000]);
 // Earn sun through normal controls; no health, money, clock or save cheats.
 let flowers=0,puffs=0;
 async function placeCard(i,x,y){await click(await seed(i,5)+25,await seed(i,6)+35);await click(x,y);}
 const deadline=Date.now()+150000;
 while(Date.now()<deadline){
  const sun=await page.evaluate(()=>[Module._pvz_sun_data(0,1),Module._pvz_sun_data(0,2)]);if(sun[0]>=0)await click(...sun);
  const money=await page.evaluate(()=>Module._pvz_adventure_power_data(-1,2));
  if(flowers<3&&money>=50&&await seed(1,4)){await placeCard(1,120,flowers===0?130:flowers===1?215:470);++flowers;}
  if(puffs<4&&await seed(2,4)){await placeCard(2,200,[130,215,470,555][puffs]);++puffs;}
  if(flowers===3&&money>=250&&await seed(0,4))break;
  await page.waitForTimeout(350);
 }
 assert.ok(await page.evaluate(()=>Module._pvz_adventure_power_data(-1,2)>=250),'normal production earned enough sun');
 await page.mouse.click(500,300,{button:'right'});await placeCard(0,320,215);
 results.afterPlant={card:await page.evaluate(()=>Array.from({length:7},(_,f)=>Module._pvz_adventure_seed_data(0,f))),money:await page.evaluate(()=>Module._pvz_adventure_power_data(-1,2))};
 await snap('adventure-nuke-planted');assert.ok(await seed(0,3)>2800);await wait(()=>Module._pvz_nuke_data(-1,0)===9);
 await click(748,14);results.saveBefore=await holes();assert.ok(results.saveBefore[0][3]>0);await snap('adventure-pause-mid-burst');
 await click(400,401);await page.waitForTimeout(350);await click(305,394);await page.waitForTimeout(1800);await click(560,135);await wait(()=>Module._pvz_adventure_power_data(-1,5)===1);
 results.saveAfter=await holes();assert.equal(results.saveAfter.length,9);assert.ok(results.saveAfter[0][3]<=results.saveBefore[0][3]);
 await click(280,371); // Native save-resume confirmation; it deliberately pauses the board.
 await wait(()=>Module._pvz_nuke_data(0,3)===0);await page.waitForTimeout(2200);await snap('adventure-resumed-crater');
 assert.deepEqual(errors,[]);await writeFile(join(out,'report.json'),JSON.stringify({results,errors},null,2));console.log('Nuclear shroom QA passed',out,results);
}catch(e){await snap('failure');await writeFile(join(out,'failure.json'),JSON.stringify({error:String(e),results,errors,log:await page.evaluate(()=>window.pvzEngineLog)},null,2));throw e;}
finally{await browser.close();}
