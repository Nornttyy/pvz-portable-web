// Actual native WASM, isolated synthetic save profile; never edits player saves.
import assert from 'node:assert/strict';
import {mkdir,writeFile} from 'node:fs/promises';
import {join} from 'node:path';
const {chromium}=await import(process.env.PVZ_PLAYWRIGHT||'playwright-core');
const out=process.env.PVZ_QA_OUT||'/tmp/pvz-awkward-qa';await mkdir(out,{recursive:true});
const browser=await chromium.launch({executablePath:'/Applications/Google Chrome.app/Contents/MacOS/Google Chrome',headless:true,args:['--no-sandbox','--autoplay-policy=no-user-gesture-required']});
const page=await browser.newPage({viewport:{width:1100,height:750},hasTouch:true}),errors=[],results={};page.on('pageerror',e=>errors.push(e.message));
const wait=(f,a,t=15000)=>page.waitForFunction(f,a,{timeout:t});
async function click(x,y){const b=await page.locator('#canvas').boundingBox(),s=await page.evaluate(()=>[Module.canvas.width,Module.canvas.height]);await page.mouse.click(b.x+x*b.width/s[0],b.y+y*b.height/s[1]);await page.waitForTimeout(100);}
const api=(...a)=>page.evaluate(a=>Module._pvz_sandbox_command(...[...a,0,0,0,0].slice(0,4)),a);
const snap=n=>page.screenshot({path:join(out,n+'.png')});
const data=i=>page.evaluate(i=>Array.from({length:12},(_,f)=>Module._pvz_sandbox_plant_data(i,f)),i);
const adv=i=>page.evaluate(i=>Array.from({length:16},(_,f)=>Module._pvz_adventure_power_data(i,f)),i);
async function boot(level=0){
 await page.goto(process.env.PVZ_QA_URL||'http://127.0.0.1:8097/');await wait(()=>!document.getElementById('start').disabled,undefined,90000);
 if(level)await page.evaluate(async level=>{const FS=Module.FS;if(!FS.analyzePath('/saves/userdata').exists)FS.mkdir('/saves/userdata');const name=new TextEncoder().encode('AwkwardQA'),users=new Uint8Array(16+name.length),u=new DataView(users.buffer);u.setUint32(0,14,true);u.setUint16(4,1,true);u.setUint16(6,name.length,true);users.set(name,8);u.setUint32(8+name.length,1,true);u.setUint32(12+name.length,1,true);FS.writeFile('/saves/userdata/users.dat',users);const profile=new Uint8Array(4096),p=new DataView(profile.buffer);p.setUint32(0,12,true);p.setUint32(4,level,true);FS.writeFile('/saves/userdata/user1.dat',profile);await new Promise((r,j)=>FS.syncfs(false,e=>e?j(e):r()));},level);
 await page.locator('#start').click();await page.waitForTimeout(12000);await click(400,560);await page.waitForTimeout(4500);
}
async function step(){await api(13);await page.waitForTimeout(45);}
try{
 await boot(8);await click(260,348);await wait(()=>Module.canvas.width===1024);await api(7);await api(4,1);
 for(const [id,col,row] of [[520,2,2],[520,1,2],[520,2,1],[520,7,3],[1,4,2]])assert.ok(await api(1,id,col,row)>0);
 assert.equal((await data(0))[0],520);await click(132,535);await click(718,62);await snap('new-card-and-idle');
 await api(4,0);await wait(()=>Module._pvz_sandbox_plant_data(0,4)===1,undefined,30000);await page.waitForTimeout(1200);
 // Move through native sun pickup areas so the produced sun does not obscure
 // the face in visual evidence. These are ordinary clicks, not state writes.
 for(const [x,y] of [[446,286],[446,305],[460,323],[366,305],[446,205]])await click(x,y);
 await api(4,1);
 const watched=await data(0);assert.ok(watched[6]>100&&watched[7]>=1);assert.equal(watched[5],0);assert.equal(watched[10],0);
 const neighbours=[await data(1),await data(2)];assert.ok(neighbours.some(a=>a[4]===0&&a[5]===21&&a[10]>0));
 for(const a of [watched,...neighbours])if(a[4]===1){assert.equal(a[5],0);assert.equal(a[10],0);}
 assert.equal((await data(3))[4],0);results.exclusiveRoles=true;await snap('embarrassed-and-neighbours-look');
 const normal=await data(4);for(let i=0;i<20;++i)await step();
 const after=await data(0),normalAfter=await data(4);assert.equal(watched[11]-after[11],20);assert.equal(normal[11]-normalAfter[11],40);results.halfSpeed=true;results.solo=true;results.neighbours=true;
 await api(4,0);await wait(()=>Module._pvz_sandbox_plant_data(0,4)===0);await api(4,1);await snap('smile-restored');
 const recovered=await data(0);await step();assert.equal(recovered[11]-(await data(0))[11],2);results.recovery=true;
 await page.setViewportSize({width:844,height:390});await snap('phone-sandbox');await page.setViewportSize({width:1100,height:750});
 if(!process.env.PVZ_QA_SANDBOX_ONLY){
  await api(15);await page.waitForTimeout(1800);await click(560,135);await page.waitForTimeout(15000);await snap('adventure-chooser');
  for(const base of [0,1,3,4,5])await click(47+base*53,163);await click(471,236);await click(258,566);await wait(()=>Module._pvz_adventure_power_data(-1,5)===1,undefined,25000);
  const card=await page.evaluate(()=>{for(let i=0;i<10;++i)if(Module._pvz_adventure_seed_data(i,0)===520)return Array.from({length:7},(_,f)=>Module._pvz_adventure_seed_data(i,f));return null;});
  assert.ok(card);assert.equal(card[1],50);assert.equal(card[2],300);results.adventureCard=true;
  async function collect(){const suns=await page.evaluate(()=>Array.from({length:12},(_,i)=>[Module._pvz_adventure_power_data(i,10),Module._pvz_adventure_power_data(i,11)]).filter(p=>p[0]>=0));for(const [x,y] of suns)await click(x,y);}
  await click(card[5]+25,card[6]+35);await click(80,530);await wait(()=>Module._pvz_adventure_power_data(0,0)===520);
  for(let i=0;i<160&&await page.evaluate(()=>Module._pvz_adventure_power_data(-1,2))<50;++i){await collect();await page.waitForTimeout(200);}
  await click(card[5]+25,card[6]+35);await click(160,530);await wait(()=>Module._pvz_adventure_power_data(1,0)===520);
  await wait(()=>Module._pvz_adventure_power_data(0,4)===1||Module._pvz_adventure_power_data(1,4)===1,undefined,20000);
  await click(748,14);results.saved=[await adv(0),await adv(1)];await snap('adventure-embarrassed');
  await click(400,401);await page.waitForTimeout(250);await click(305,394);await wait(()=>Module._pvz_adventure_power_data(-1,5)===-1);await page.evaluate(()=>new Promise((r,j)=>Module.FS.syncfs(false,e=>e?j(e):r())));
  await boot();await click(560,135);await wait(()=>Module._pvz_adventure_power_data(-1,5)===1,undefined,30000);results.resumed=[await adv(0),await adv(1)];
  // Coin positions change through ordinary save reconstruction; plant state must not.
  const plantState=a=>a.filter((_,i)=>i<10||i>13);assert.deepEqual(results.resumed.map(plantState),results.saved.map(plantState));await snap('resumed-expression');results.saveResume=true;
  await boot(37);await click(370,455);await click(208,366);await click(416,199);await page.waitForTimeout(600);await snap('awkward-almanac');
 }
 assert.deepEqual(errors,[]);await writeFile(join(out,'report.json'),JSON.stringify({results,errors},null,2));console.log('Awkward sunflower QA passed',out,results);
}catch(e){await snap('failure');await writeFile(join(out,'failure.json'),JSON.stringify({error:String(e),results,errors,log:await page.evaluate(()=>window.pvzEngineLog)},null,2));throw e;}
finally{await browser.close();}
