// Real shipped WASM, isolated player profile. No changes to the player's saves.
import assert from 'node:assert/strict';
import {mkdir,writeFile} from 'node:fs/promises';
import {join} from 'node:path';
const {chromium}=await import(process.env.PVZ_PLAYWRIGHT||'playwright-core');
const out=process.env.PVZ_QA_OUT||'/tmp/pvz-cactus-palm-qa';await mkdir(out,{recursive:true});
const browser=await chromium.launch({executablePath:'/Applications/Google Chrome.app/Contents/MacOS/Google Chrome',headless:true,args:['--no-sandbox','--autoplay-policy=no-user-gesture-required']});
const page=await browser.newPage({viewport:{width:1100,height:750},hasTouch:true}),errors=[],results={};page.on('pageerror',e=>errors.push(e.message));
const wait=(f,a,t=30000)=>page.waitForFunction(f,a,{timeout:t});
const api=(...a)=>page.evaluate(a=>Module._pvz_sandbox_command(...[...a,0,0,0,0].slice(0,4)),a);
const snap=n=>page.screenshot({path:join(out,n+'.png')});
const zombies=()=>page.evaluate(()=>{const out=[];for(let i=0;i<30;++i){const z=Array.from({length:21},(_,f)=>Module._pvz_sandbox_zombie_data(i,f));if(z[0]<0)break;out.push(z);}return out;});
async function click(x,y){const b=await page.locator('#canvas').boundingBox(),s=await page.evaluate(()=>[Module.canvas.width,Module.canvas.height]);await page.mouse.click(b.x+x*b.width/s[0],b.y+y*b.height/s[1]);await page.waitForTimeout(100);}
async function boot(level=0){
 await page.goto(process.env.PVZ_QA_URL||'http://127.0.0.1:8097/');await wait(()=>!document.getElementById('start').disabled,undefined,90000);
 if(level)await page.evaluate(async level=>{
  const FS=Module.FS;if(!FS.analyzePath('/saves/userdata').exists)FS.mkdir('/saves/userdata');
  const name=new TextEncoder().encode('PalmQA'),users=new Uint8Array(16+name.length),u=new DataView(users.buffer);
  u.setUint32(0,14,true);u.setUint16(4,1,true);u.setUint16(6,name.length,true);users.set(name,8);u.setUint32(8+name.length,1,true);u.setUint32(12+name.length,1,true);FS.writeFile('/saves/userdata/users.dat',users);
  const profile=new Uint8Array(4096),p=new DataView(profile.buffer);p.setUint32(0,12,true);p.setUint32(4,level,true);FS.writeFile('/saves/userdata/user1.dat',profile);await new Promise((r,j)=>FS.syncfs(false,e=>e?j(e):r()));
 },level);
 await page.locator('#start').click();await page.waitForTimeout(12000);await click(400,560);await page.waitForTimeout(4500);
}
async function fresh(){await api(7);await api(4,1);await api(5,1);}
try{
 await boot(33);
 if(!process.env.PVZ_QA_CAMPAIGN_ONLY){
  await click(260,348);await wait(()=>Module.canvas.width===1024);await fresh();
  for(let row=0;row<5;row++)assert.equal(await api(1,524,1,row),1);
  for(const [id,row] of [[2,0],[23,1],[214,2],[32,3],[16,4]])assert.equal(await api(2,id,7,row),1);
  await snap('palm-standing');await api(5,4);await api(4,0);
  await wait(()=>[299,300].includes(Module._pvz_projectile_data(0,0)));await api(4,1);
  results.firstShots=await page.evaluate(()=>Array.from({length:5},(_,i)=>Array.from({length:9},(_,f)=>Module._pvz_projectile_data(i,f))));await snap('palms-low-and-raised');
  assert.ok(results.firstShots.filter(p=>p[0]>0).every(p=>[299,300].includes(p[0])&&p[8]===8));
  results.impacts=[];await api(4,0);let previous=await zombies();
  const deadline=Date.now()+25000;
  while(Date.now()<deadline&&results.impacts.length<16){
   await page.waitForTimeout(15);const current=await zombies();
   for(let i=0;i<Math.min(previous.length,current.length);i++){
    const a=previous[i],b=current[i];if(a[0]!==b[0]||a[1]!==b[1])continue;
    const damage=a[4]+a[5]+a[7]-b[4]-b[5]-b[7],push=b[2]-a[2];
    if((damage===80||damage===110)&&push>0){
     const giant=[23,32].includes(b[15]),expected=(giant?40:120)*(damage===110?1.5:1);
     assert.ok(Math.abs(push-expected)<=12,JSON.stringify({damage,push,expected,type:b[0]}));results.impacts.push({damage,push,expected,type:b[0]});
    }
   }
   previous=current;
  }
  await api(4,1);assert.ok(results.impacts.some(v=>v.damage===80));assert.ok(results.impacts.some(v=>v.damage===110));assert.ok(results.impacts.some(v=>v.expected===40));await snap('palm-knockback');
  await page.setViewportSize({width:844,height:390});await page.waitForTimeout(1000);await snap('palm-phone');await page.setViewportSize({width:1100,height:750});
  console.log('Sandbox palm damage, crit, giant and normal knockback passed',results.impacts);
  await api(15);await page.waitForTimeout(1800);
 }
 if(!process.env.PVZ_QA_SANDBOX_ONLY){
  await click(560,135);await page.waitForTimeout(15000);await snap('palm-adventure-chooser');
  for(const base of [26,1,3,4,5,16]){const slot=base>=8?base+2:base;await click(47+slot%9*53,163+Math.floor(slot/9)*73);}
  await click(258,566);await wait(()=>Module._pvz_adventure_power_data(-1,5)===1);await snap('palm-adventure-bank');
  results.card=await page.evaluate(()=>Array.from({length:7},(_,f)=>Module._pvz_adventure_seed_data(0,f)));assert.equal(results.card[0],524);assert.equal(results.card[1],125);assert.equal(results.card[2],750);
  // Fog has no falling sun: plant the chosen producer with the initial 50.
  const producer=await page.evaluate(()=>[Module._pvz_adventure_seed_data(1,5),Module._pvz_adventure_seed_data(1,6)]);
  await click(producer[0]+25,producer[1]+30);await click(115,230);
  // Native sun collection only; no sandbox mutation on an adventure board.
  const end=Date.now()+75000;
  while(await page.evaluate(()=>Module._pvz_adventure_power_data(-1,2))<125&&Date.now()<end){
   const xy=await page.evaluate(()=>[Module._pvz_adventure_power_data(0,10),Module._pvz_adventure_power_data(0,11)]);if(xy[0]>=0)await click(...xy);await page.waitForTimeout(350);
  }
  assert.ok(await page.evaluate(()=>Module._pvz_adventure_power_data(-1,2))>=125);
  await click(results.card[5]+25,results.card[6]+30);await click(195,130);await page.waitForTimeout(500);
  results.plant=await page.evaluate(()=>Array.from({length:10},(_,f)=>Module._pvz_adventure_power_data(1,f)));assert.equal(results.plant[0],524);assert.equal(results.plant[8],26);await snap('palm-adventure-planted');
  await click(748,14);await click(400,401);await page.waitForTimeout(250);await click(305,394);await wait(()=>Module._pvz_adventure_power_data(-1,5)===-1);
  await page.evaluate(()=>new Promise((r,j)=>Module.FS.syncfs(false,e=>e?j(e):r())));await boot();await click(560,135);await wait(()=>Module._pvz_adventure_power_data(-1,5)===1);
  results.resumed=await page.evaluate(()=>[Module._pvz_adventure_power_data(1,0),Module._pvz_adventure_power_data(1,8)]);assert.deepEqual(results.resumed,[524,26]);await snap('palm-adventure-resumed');
 }
 await boot(37);await click(370,455);await click(208,366);await snap('palm-almanac-index');
 // Native almanac plant grid: original cactus entry, not an extra card.
 await click(94,350);await page.waitForTimeout(500);await snap('palm-almanac');
 assert.deepEqual(errors,[]);await writeFile(join(out,'report.json'),JSON.stringify({results,errors},null,2));console.log('Palm browser QA passed',out);
}catch(e){await snap('failure');await writeFile(join(out,'failure.json'),JSON.stringify({error:String(e),results,errors,zombies:await zombies(),log:await page.evaluate(()=>window.pvzEngineLog)},null,2));throw e;}
finally{await browser.close();}
