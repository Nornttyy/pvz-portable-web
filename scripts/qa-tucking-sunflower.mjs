// Actual native WASM, isolated synthetic save profile; never edits player saves.
import assert from 'node:assert/strict';
import {mkdir,writeFile} from 'node:fs/promises';
import {join} from 'node:path';
const {chromium}=await import(process.env.PVZ_PLAYWRIGHT||'playwright-core');
const out=process.env.PVZ_QA_OUT||'/tmp/pvz-tucking-qa';await mkdir(out,{recursive:true});
const browser=await chromium.launch({executablePath:'/Applications/Google Chrome.app/Contents/MacOS/Google Chrome',headless:true,args:['--no-sandbox','--autoplay-policy=no-user-gesture-required']});
const page=await browser.newPage({viewport:{width:1100,height:750},hasTouch:true}),errors=[],results={};page.on('pageerror',e=>errors.push(e.message));
const wait=(f,a,t=15000)=>page.waitForFunction(f,a,{timeout:t});
async function click(x,y){const b=await page.locator('#canvas').boundingBox(),s=await page.evaluate(()=>[Module.canvas.width,Module.canvas.height]);await page.mouse.click(b.x+x*b.width/s[0],b.y+y*b.height/s[1]);await page.waitForTimeout(100);}
const api=(...a)=>page.evaluate(a=>Module._pvz_sandbox_command(...[...a,0,0,0,0].slice(0,4)),a);
const snap=n=>page.screenshot({path:join(out,n+'.png')});
const data=i=>page.evaluate(i=>Array.from({length:12},(_,f)=>Module._pvz_sandbox_plant_data(i,f)),i);
const zombie=i=>page.evaluate(i=>Array.from({length:21},(_,f)=>Module._pvz_sandbox_zombie_data(i,f)),i);
const adv=i=>page.evaluate(i=>Array.from({length:16},(_,f)=>Module._pvz_adventure_power_data(i,f)),i);
async function boot(level=0){
 await page.goto(process.env.PVZ_QA_URL||'http://127.0.0.1:8097/');await wait(()=>!document.getElementById('start').disabled,undefined,90000);
 if(level)await page.evaluate(async level=>{const FS=Module.FS;if(!FS.analyzePath('/saves/userdata').exists)FS.mkdir('/saves/userdata');const name=new TextEncoder().encode('AwkwardQA'),users=new Uint8Array(16+name.length),u=new DataView(users.buffer);u.setUint32(0,14,true);u.setUint16(4,1,true);u.setUint16(6,name.length,true);users.set(name,8);u.setUint32(8+name.length,1,true);u.setUint32(12+name.length,1,true);FS.writeFile('/saves/userdata/users.dat',users);const profile=new Uint8Array(4096),p=new DataView(profile.buffer);p.setUint32(0,12,true);p.setUint32(4,level,true);FS.writeFile('/saves/userdata/user1.dat',profile);await new Promise((r,j)=>FS.syncfs(false,e=>e?j(e):r()));},level);
 await page.locator('#start').click();await page.waitForTimeout(12000);await click(400,560);await page.waitForTimeout(4500);
}
async function step(){await api(13);await page.waitForTimeout(45);}
try{
 await boot(9);await click(260,348);await wait(()=>Module.canvas.width===1024);await api(7);await api(4,1);
 // Select the replacement in the native Repeater's catalogue slot (index 7).
 await click(132,231);await click(384,330);assert.equal((await data(0))[0],521);
 await api(2,4,8,2);await api(5,1);
 for(let tick=0;tick<50;++tick)await step();
 assert.equal((await data(0))[7],49);
 for(let tick=0;tick<98;++tick)await step();
 assert.equal((await data(0))[4],0);assert.equal((await data(0))[7],0);
 results.repeaterShots=await page.evaluate(()=>{const shots=[];for(let i=0;i<200;++i){const style=Module._pvz_projectile_data(i,0);if(style<0)break;shots.push(Array.from({length:9},(_,f)=>Module._pvz_projectile_data(i,f)));}return shots;});
 assert.equal(results.repeaterShots.length,50);assert.ok(results.repeaterShots.every(s=>s[0]===297&&s[4]===0&&s[8]===0));await snap('repeater-fifty-native-peas');
 await api(7);await api(4,1);
 // The second card replaces sunflower, with no separate New Cards category.
 await click(81,153);await click(624,330);assert.equal((await data(0))[0],520);await api(7);
 for(const [id,col,row] of [[520,4,2],[1,1,2],[520,4,1],[520,7,4]])assert.ok(await api(1,id,col,row)>0);
 assert.equal((await data(0))[0],520);await click(718,62);await snap('replacement-cards-and-idle');
 await api(2,0,6,2);await api(5,4);await api(4,0);
 await wait(()=>Module._pvz_sandbox_plant_data(0,4)===1,undefined,30000);
 await api(4,1);
 const hidden=await data(0);assert.equal(hidden[3],300);assert.equal(hidden[4],1);
 assert.equal((await data(2))[4],0);assert.equal((await data(3))[4],0);await snap('tucked-before-contact');
 await api(5,1);const normal=await data(1);for(let i=0;i<20;++i)await step();
 assert.equal(hidden[11],(await data(0))[11]);assert.equal(normal[11]-(await data(1))[11],40);results.hiddenProductionPaused=true;
 await api(5,4);await api(4,0);
 await wait(()=>Module._pvz_sandbox_zombie_data(0,2)<290,undefined,30000);
 await api(4,1);assert.equal((await data(0))[3],300);assert.equal((await zombie(0))[20],0);await snap('zombie-walked-through');results.walkedThrough=true;
 await api(4,0);await wait(()=>Module._pvz_sandbox_plant_data(1,3)<300,undefined,30000);await api(4,1);
 assert.equal((await data(0))[4],0);assert.equal((await data(0))[3],300);assert.equal((await zombie(0))[20],1);results.backlineStillEdible=true;await snap('recovered-backline-in-danger');
 await api(6);await api(2,0,4,2);await step();assert.equal((await data(0))[4],1);await snap('tucks-again');
 await api(6);await step();assert.equal((await data(0))[4],0);results.repeatedRecovery=true;
 await page.setViewportSize({width:844,height:390});await snap('phone-sandbox');await page.setViewportSize({width:1100,height:750});
 if(!process.env.PVZ_QA_SANDBOX_ONLY){
  await api(15);await page.waitForTimeout(1800);await click(560,135);await page.waitForTimeout(15000);await snap('adventure-chooser');
  for(const base of [0,1,3,4,5,7])await click(47+base*53,163);await click(258,566);await wait(()=>Module._pvz_adventure_power_data(-1,5)===1,undefined,25000);
  results.adventureCards=await page.evaluate(()=>Array.from({length:6},(_,i)=>Module._pvz_adventure_seed_data(i,0)));
  assert.deepEqual(results.adventureCards,[500,520,501,4,5,521]);
  const card=await page.evaluate(()=>{for(let i=0;i<10;++i)if(Module._pvz_adventure_seed_data(i,0)===520)return Array.from({length:7},(_,f)=>Module._pvz_adventure_seed_data(i,f));return null;});
  assert.ok(card);assert.equal(card[1],50);assert.equal(card[2],300);results.adventureCard=true;
  await wait(()=>Module._pvz_adventure_power_data(0,16)>=0,undefined,45000);
  const row=await page.evaluate(()=>Module._pvz_adventure_power_data(0,16));
  await click(card[5]+25,card[6]+35);await click(720,130+row*100);await wait(()=>Module._pvz_adventure_power_data(0,0)===520);
  await wait(()=>Module._pvz_adventure_power_data(0,4)===1,undefined,30000);
  await click(748,14);results.saved=[await adv(0)];assert.equal(results.saved[0][3],300);await snap('adventure-tucked');
  await click(400,401);await page.waitForTimeout(250);await click(305,394);await wait(()=>Module._pvz_adventure_power_data(-1,5)===-1);await page.evaluate(()=>new Promise((r,j)=>Module.FS.syncfs(false,e=>e?j(e):r())));
  await boot();await click(560,135);await wait(()=>Module._pvz_adventure_power_data(-1,5)===1,undefined,30000);results.resumed=[await adv(0)];
  // Coin positions change through ordinary save reconstruction; plant state must not.
  const plantState=a=>a.filter((_,i)=>i<10||i>13);assert.deepEqual(results.resumed.map(plantState),results.saved.map(plantState));await snap('resumed-expression');results.saveResume=true;
  assert.equal(results.resumed[0][8],1); // Restored sunflower keeps its vanilla slot.
  await boot(37);await click(370,455);await click(208,366);await click(94,123);await page.waitForTimeout(600);await snap('tucking-almanac');
  await click(370,123);await page.waitForTimeout(600);await snap('repeater-almanac');
 }
 assert.deepEqual(errors,[]);await writeFile(join(out,'report.json'),JSON.stringify({results,errors},null,2));console.log('Tucking sunflower QA passed',out,results);
}catch(e){await snap('failure');await writeFile(join(out,'failure.json'),JSON.stringify({error:String(e),results,errors,log:await page.evaluate(()=>window.pvzEngineLog)},null,2));throw e;}
finally{await browser.close();}
