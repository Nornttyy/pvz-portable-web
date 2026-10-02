// Actual WebAssembly combat, native cards and adventure integration. No player saves.
import assert from 'node:assert/strict';
import {mkdir,writeFile} from 'node:fs/promises';
import {join} from 'node:path';
const {chromium}=await import(process.env.PVZ_PLAYWRIGHT||'playwright-core');
const out=process.env.PVZ_QA_OUT||'/tmp/pvz-ten-character-qa';await mkdir(out,{recursive:true});
const browser=await chromium.launch({executablePath:process.env.PVZ_CHROME||'/Applications/Google Chrome.app/Contents/MacOS/Google Chrome',headless:true,args:['--no-sandbox']});
const page=await browser.newPage({viewport:{width:1100,height:750},hasTouch:true}),errors=[],results={};page.on('pageerror',e=>errors.push(e.stack||e.message));
async function api(...a){return page.evaluate(a=>Module._pvz_sandbox_command(...[...a,0,0,0,0].slice(0,4)),a);}
async function pd(i,f){return page.evaluate(([i,f])=>Module._pvz_sandbox_plant_data(i,f),[i,f]);}
async function zd(i,f){return page.evaluate(([i,f])=>Module._pvz_sandbox_zombie_data(i,f),[i,f]);}
async function enemies(){return page.evaluate(()=>{let n=0;while(n<300&&Module._pvz_sandbox_zombie_data(n,0)>=0)++n;return n;});}
async function snap(name){await page.screenshot({path:join(out,name+'.png')});}
async function click(x,y){const b=await page.locator('#canvas').boundingBox(),s=await page.evaluate(()=>[Module.canvas.width,Module.canvas.height]);await page.mouse.click(b.x+x*b.width/s[0],b.y+y*b.height/s[1]);await page.waitForTimeout(100);}
async function wait(fn,arg,timeout=10000){await page.waitForFunction(fn,arg,{timeout});}
async function fresh(){await api(7);await api(4,1);await api(5,4);}
async function shots(){return page.evaluate(()=>{const a=[];for(let i=0;i<100;i++){if(Module._pvz_projectile_data(i,0)<0)break;a.push(Array.from({length:9},(_,f)=>Module._pvz_projectile_data(i,f)));}return a;});}
try{
 await page.goto(process.env.PVZ_QA_URL||'http://127.0.0.1:8097/');await wait(()=>!document.getElementById('start').disabled,undefined,90000);
 await page.evaluate(async()=>{
  const FS=Module.FS;if(!FS.analyzePath('/saves/userdata').exists)FS.mkdir('/saves/userdata');
  const name=new TextEncoder().encode('BatchQA'),users=new Uint8Array(16+name.length),u=new DataView(users.buffer);u.setUint32(0,14,true);u.setUint16(4,1,true);u.setUint16(6,name.length,true);users.set(name,8);u.setUint32(8+name.length,1,true);u.setUint32(12+name.length,1,true);FS.writeFile('/saves/userdata/users.dat',users);
  const profile=new Uint8Array(4096),p=new DataView(profile.buffer);p.setUint32(0,12,true);p.setUint32(4,43,true);FS.writeFile('/saves/userdata/user1.dat',profile);await new Promise((r,j)=>FS.syncfs(false,e=>e?j(e):r()));
 });
 await page.locator('#start').click();await page.waitForTimeout(12000);await click(400,560);await page.waitForTimeout(4500);await click(719,27);await wait(()=>Module.canvas.width===1024,undefined,20000);
 if(!process.env.PVZ_QA_TAIL_ONLY){
 // Click every added card in the real sidebar, not just the placement API.
 await api(4,1);for(let n=8;n<14;++n){await click(30+n%5*51,153+Math.floor(n/5)*78);await click(344+(n-8)*80,330);assert.equal(await pd(n-8,0),500+n);}await snap('six-new-cards-and-native-rigs');results.cards=true;

 await fresh();await api(1,508,0,2);await api(2,4,8,2);await api(4,0);
 await wait(()=>Module._pvz_sandbox_plant_data(0,7)>0);await wait(()=>Module._pvz_sandbox_plant_data(0,7)===0);await api(4,1);
 let flight=await shots();assert.equal(flight.length,6);assert.ok(flight.every(s=>s[8]===8));await snap('cactus-six-native-spikes');results.cactus=true;
 await fresh();await api(1,508,0,2);await api(2,16,8,2);await api(4,0);await wait(()=>Module._pvz_sandbox_plant_data(0,8)===32);await snap('cactus-native-tall-pose');await wait(()=>Module._pvz_sandbox_zombie_data(0,11)<20,undefined,14000);await api(4,1);results.cactusAir=true;

 await fresh();await api(1,509,3,2);await api(2,0,4,2);await api(4,0);await wait(()=>Module._pvz_sandbox_plant_data(0,8)===13,undefined,18000);await api(4,1);
 const before=await pd(0,9);await api(1,0,3,1);await api(2,4,8,1);await api(4,0);await wait(()=>Module._pvz_sandbox_plant_data(0,6)>0);await api(4,1);assert.ok(before-await pd(0,9)>=200);assert.equal(await pd(0,3),300);await snap('chomper-neighbor-feeding');results.chomper=true;

 await fresh();await api(1,510,0,2);await api(2,4,3,2);await api(2,4,8,2);await api(4,0);await wait(()=>Module._pvz_projectile_data(0,0)>=0);await api(4,1);flight=await shots();assert.ok(flight[0][3]>4000);assert.equal(flight[0][8],2);await snap('cabbage-lobs-over-frontline');results.cabbage=true;

 await fresh();await api(1,511,3,2);await api(2,4,7,2);await api(4,0);await wait(()=>Module._pvz_sandbox_plant_data(0,5)===1);await api(4,1);const first=await shots();assert.equal(first.length,5);
 await api(4,0);await wait(()=>Module._pvz_sandbox_plant_data(0,5)===2);await api(4,1);const second=(await shots()).filter(s=>s[6]<50);assert.equal(second.length,5);assert.ok(first.some(s=>s[3]===0&&s[4]>3000));assert.ok(second.some(s=>s[3]<-1000&&s[4]>3000));await snap('starfruit-rotated-native-stars');results.starfruit=true;

 await fresh();await api(1,512,0,2);await api(2,23,7,2);await api(4,0);await wait(()=>Module._pvz_sandbox_plant_data(0,7)===5,undefined,18000);await api(4,1);assert.ok((await shots()).some(s=>s[8]===12));
 await api(4,0);await wait(()=>Module._pvz_sandbox_plant_data(0,7)===0);await api(4,1);assert.ok((await shots()).filter(s=>s[8]===10).length>=4);await snap('corn-butter-followup');results.corn=true;

 await fresh();await api(1,513,2,2);await api(1,3,2,1);await api(2,4,3,1);await api(4,0);await wait(()=>Module._pvz_sandbox_plant_data(0,7)===6,undefined,18000);await api(4,1);await snap('shroom-storing-retorts');await api(6);await api(2,4,8,2);await api(4,0);await wait(()=>Module._pvz_sandbox_plant_data(0,7)===0);await api(4,1);assert.ok((await shots()).length>=5);await snap('shroom-release');results.shroom=true;

 await fresh();await api(2,0,7,2);await api(4,0);await wait(()=>Module._pvz_sandbox_zombie_data(0,12)===1);await api(4,1);const x=await zd(0,2);await snap('slacker-resting');await api(4,0);await page.waitForTimeout(180);await api(4,1);assert.equal(await zd(0,2),x);await api(4,0);await wait(()=>Module._pvz_sandbox_zombie_data(0,12)===0);results.slacker=true;
 await fresh();await api(2,7,7,2);assert.equal(await zd(0,9),180);await api(4,0);await wait(()=>Module._pvz_sandbox_zombie_data(0,12)===1);await api(4,1);await snap('football-braking');assert.equal(await zd(0,9),0);await api(4,0);await wait(()=>Module._pvz_sandbox_zombie_data(0,9)===100);results.football=true;

 await fresh();await api(1,0,0,2);await api(2,4,7,2);await api(2,6,7,1);const shield=await zd(1,7);await api(4,0);await wait(n=>Module._pvz_sandbox_zombie_data(1,7)<n,shield);await api(4,1);assert.ok(await zd(0,5)<1100);await snap('door-absorbs-neighbor-damage');results.guard=true;
 }
 await fresh();assert.ok(await api(2,16,6,2)>0);await api(4,0);
 results.airdropTrace=[];for(let n=0;n<100;++n){const a=await page.evaluate(()=>{let count=0;while(count<300&&Module._pvz_sandbox_zombie_data(count,0)>=0)++count;return[count,...[0,2,6,10,11].map(f=>Module._pvz_sandbox_zombie_data(0,f))];});results.airdropTrace.push(a);if(a[0]===2)break;await page.waitForTimeout(30);}
 await api(4,1);assert.equal(await enemies(),2);assert.equal(await zd(1,0),0);assert.ok(await zd(1,11)>0);await snap('balloon-airdrop');await api(4,0);await page.waitForTimeout(2200);await api(4,1);assert.equal(await enemies(),2);results.balloon=true;

 await fresh();for(let i=0;i<6;++i)await api(1,508+i,1+i,2);await page.setViewportSize({width:844,height:390});await page.waitForTimeout(300);await snap('mobile-batch-layout');await page.setViewportSize({width:1100,height:750});await page.waitForTimeout(300);
 await api(15);await page.waitForTimeout(800);await click(560,135);await page.waitForTimeout(15000);await snap('adventure-roof-chooser');
 for(const base of [26,6,32,29,34,13]){const slot=base>=8?base+1:base;await click(47+(slot%9)*53,163+Math.floor(slot/9)*73);}
 await page.waitForTimeout(300);await snap('adventure-six-new-native-slots');await click(232,566);
 // This QA deliberately fills all six slots with new roles; confirm the
 // original game's missing-sunflower and missing-flowerpot warnings.
 await click(305,367);await page.waitForTimeout(300);await click(305,367);
 await wait(()=>Module._pvz_adventure_power_data(-1,5)===1,undefined,25000);
 const bank=await page.evaluate(()=>Array.from({length:6},(_,i)=>[0,1,2].map(f=>Module._pvz_adventure_seed_data(i,f))));assert.deepEqual(bank.map(s=>s[0]),[508,509,510,511,512,513]);assert.ok(bank.every(s=>s[2]===300));await snap('adventure-new-roster-live');results.adventure=bank;
 assert.deepEqual(errors,[]);await writeFile(join(out,'report.json'),JSON.stringify({results,errors},null,2));console.log('Ten-character native QA passed',results);
}catch(e){await snap('failure');await writeFile(join(out,'failure.json'),JSON.stringify({error:String(e),results,errors,plants:await page.evaluate(()=>Array.from({length:8},(_,i)=>Array.from({length:10},(_,f)=>Module._pvz_sandbox_plant_data(i,f)))),shots:await shots()},null,2));throw e;}
finally{await browser.close();}
