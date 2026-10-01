// Real WASM/gameplay QA. Only the disposable profile and sandbox use test setup.
import assert from 'node:assert/strict';
import {mkdir,writeFile} from 'node:fs/promises';
import {join} from 'node:path';
const {chromium}=await import(process.env.PVZ_PLAYWRIGHT||'playwright-core');
const out=process.env.PVZ_QA_OUT||'/tmp/pvz-everything-qa';await mkdir(out,{recursive:true});
const browser=await chromium.launch({executablePath:'/Applications/Google Chrome.app/Contents/MacOS/Google Chrome',headless:true,args:['--no-sandbox','--autoplay-policy=no-user-gesture-required']});
const page=await browser.newPage({viewport:{width:1100,height:750}}),results={},errors=[],messages=[];
page.on('pageerror',e=>errors.push(e.message));page.on('console',m=>messages.push(m.text()));
const wait=(f,a,t=30000)=>page.waitForFunction(f,a,{timeout:t});
const api=(...a)=>page.evaluate(a=>Module._pvz_sandbox_command(...[...a,0,0,0,0].slice(0,4)),a);
const seed=(i,f)=>page.evaluate(([i,f])=>Module._pvz_adventure_seed_data(i,f),[i,f]);
const plants=()=>page.evaluate(()=>Array.from({length:40},(_,i)=>Array.from({length:7},(_,f)=>Module._pvz_stink_data(0,i,f))).filter(x=>x[0]>=0));
const stats=()=>page.evaluate(()=>[0,1].map(k=>Array.from({length:17},(_,i)=>Module._pvz_everything_data(k,i))));
const snap=async n=>{await page.mouse.move(0,0);await page.screenshot({path:join(out,n+'.png')});};
async function click(x,y){const b=await page.locator('#canvas').boundingBox(),s=await page.evaluate(()=>[Module.canvas.width,Module.canvas.height]);await page.mouse.click(b.x+x*b.width/s[0],b.y+y*b.height/s[1]);await page.waitForTimeout(100);}
async function placeCard(i,x,y){await page.mouse.click(500,300,{button:'right'});await click(await seed(i,5)+25,await seed(i,6)+35);await click(x,y);}
try{
 await page.goto(process.env.PVZ_QA_URL||'http://127.0.0.1:8097/');await wait(()=>!document.getElementById('start').disabled,undefined,90000);
 await page.evaluate(async()=>{
  const FS=Module.FS;if(!FS.analyzePath('/saves/userdata').exists)FS.mkdir('/saves/userdata');
  const name=new TextEncoder().encode('EverythingQA'),users=new Uint8Array(16+name.length),u=new DataView(users.buffer);
  u.setUint32(0,14,true);u.setUint16(4,1,true);u.setUint16(6,name.length,true);users.set(name,8);u.setUint32(8+name.length,1,true);u.setUint32(12+name.length,1,true);FS.writeFile('/saves/userdata/users.dat',users);
  const profile=new Uint8Array(4096),p=new DataView(profile.buffer);p.setUint32(0,12,true);p.setUint32(4,27,true);FS.writeFile('/saves/userdata/user1.dat',profile);
  await new Promise((r,j)=>FS.syncfs(false,e=>e?j(e):r()));
 });
 await page.locator('#start').click();await page.waitForTimeout(12000);await click(400,560);await page.waitForTimeout(4500);
 // Cold adventure must preload all ammunition without first visiting sandbox.
 if(!process.env.PVZ_QA_SANDBOX_ONLY){
 await click(560,135);await page.waitForTimeout(15000);
 for(const base of [49,51,1,16,3,15]){const slot=base===49?43:base===51?42:base>=8?base+2:base;await click(47+slot%9*53,163+Math.floor(slot/9)*73);}
 await snap('adventure-chooser');await click(258,566);await wait(()=>Module._pvz_adventure_power_data(-1,5)===1);
 assert.deepEqual(await page.evaluate(()=>[0,1,2].map(f=>Module._pvz_adventure_seed_data(0,f))),[529,250,750]);
 assert.deepEqual(await page.evaluate(()=>[0,1].map(f=>Module._pvz_adventure_seed_data(1,f))),[528,125]);
 assert.deepEqual(await page.evaluate(()=>[0,1,2].map(f=>Module._pvz_adventure_seed_data(5,f))),[526,325,3000]);
 await placeCard(2,120,130);const deadline=Date.now()+130000;let secondSun=false;
 while(Date.now()<deadline){
  const sun=await page.evaluate(()=>[Module._pvz_sun_data(0,1),Module._pvz_sun_data(0,2)]);if(sun[0]>=0)await click(...sun);
  const money=await page.evaluate(()=>Module._pvz_adventure_power_data(-1,2));
  if(!secondSun&&money>=50&&await seed(2,4)){await placeCard(2,120,215);secondSun=true;}
  else if(money>=250&&await seed(0,4))break;
  await page.waitForTimeout(200);
 }
 assert.ok(await page.evaluate(()=>Module._pvz_adventure_power_data(-1,2)>=250),'earn normal adventure sun');
 const enemyRow=await page.evaluate(()=>{for(let i=0;i<30;i++){const r=Module._pvz_sandbox_zombie_data(i,1);if(r>=0&&r!==2&&r!==3)return r;}return 0;});
 await placeCard(0,240,80+enemyRow*85+42);await wait(()=>Array.from({length:17},(_,i)=>Module._pvz_everything_data(0,i)).some(n=>n>0),undefined,50000);
 await click(748,14);results.saveBefore=await plants();assert.ok(results.saveBefore.some(p=>p[2]===529));await snap('adventure-new-shooter');
 await click(400,401);await page.waitForTimeout(350);await click(305,394);await page.waitForTimeout(1800);await click(560,135);await wait(()=>Module._pvz_adventure_power_data(-1,5)===1);
 results.saveAfter=await plants();assert.deepEqual(results.saveAfter,results.saveBefore);await snap('adventure-restored');await click(280,371);
 await click(748,14);await click(400,401);await click(305,394);await page.waitForTimeout(1800);results.adventure=true;console.log('Cold adventure, normal sun spending and save/restore passed');
 }
 await click(370,455);await click(208,366);await click(48+6*46,123+5*76);await snap('almanac-everything');
 {const b=await page.locator('#canvas').boundingBox();await page.screenshot({path:join(out,'new-card-detail.png'),clip:{x:b.x+302*b.width/800,y:b.y+472*b.height/600,width:50*b.width/800,height:64*b.height/600}});}
 await click(48+8*46,123+1*76);await snap('almanac-nuke-325');
 await click(48+5*46,123+5*76);await snap('almanac-ice-125');await click(690,580);await page.waitForTimeout(1000);
 await click(260,348);await wait(()=>Module.canvas.width===1024);await api(8,0);await api(4,1);await api(5,4);
 for(const row of [0,2,4])assert.equal(await api(1,529,1,row),1);
 // Slow armored targets do not throw imps onto the shooters (which would
 // invalidate the separate friendly-fire check).
 const replenish=async()=>{await api(6);for(const row of [0,2,4])for(const col of [4,6,8])assert.equal(await api(2,219,col,row),1);};
 await replenish();await snap('sandbox-shooter-layout');await api(4,0);
 const captured=new Set(),end=Date.now()+150000;let nextWave=Date.now()+7000;
 while(Date.now()<end){
  const rare=await page.evaluate(()=>{for(let i=0;i<80;i++){const s=Module._pvz_projectile_data(i,0);if(s>=334&&s<=336)return s;}return -1;});
  if(rare>=334&&!captured.has(rare)){await api(4,1);await snap('projectile-'+rare);captured.add(rare);await api(4,0);}
  if(Date.now()>=nextWave){console.log('Ammunition sample',JSON.stringify(await stats()));await replenish();nextWave=Date.now()+7000;}
  const [fired,hit]=await stats();if(fired.every(n=>n>0)&&hit.every(n=>n>0)&&captured.size===3&&fired.reduce((a,b)=>a+b,0)>200)break;
  await page.waitForTimeout(70);
 }
 await api(4,1);results.ammunition=await stats();results.captured=[...captured];
 assert.ok(results.ammunition[0].every(n=>n>0),'all 14 native and 3 rare ammunition types really fired');
 assert.ok(results.ammunition[1].every(n=>n>0),'all native and rare impacts really occurred');
 assert.deepEqual((await plants()).map(p=>p[2]),[529,529,529]);
 results.health=await page.evaluate(()=>[0,1,2].map(i=>Module._pvz_sandbox_plant_data(i,3)));assert.deepEqual(results.health,[300,300,300],'own basketball, zombie pea and explosives do not harm own shooters');
 await snap('sandbox-final');await page.setViewportSize({width:844,height:390});await snap('phone');
 assert.deepEqual(errors,[]);assert.ok(!messages.some(m=>/unable to load|Aborted|RuntimeError/i.test(m)),messages.join('\n'));
 await writeFile(join(out,'report.json'),JSON.stringify({results,errors,messages},null,2));console.log('Everything shooter browser QA passed',out,JSON.stringify(results));
}catch(e){console.error(e);await snap('failure').catch(()=>{});await writeFile(join(out,'failure.json'),JSON.stringify({error:String(e),results,errors,messages,plants:await plants().catch(()=>[])},null,2));throw e;}
finally{await browser.close();}
