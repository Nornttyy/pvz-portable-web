// Observe naturally rolled projectiles in the real engine; never force RNG,
// damage or health. Shovel the shooter after its first shot to isolate the hit.
import assert from 'node:assert/strict';
import {mkdir,writeFile} from 'node:fs/promises';
import {join} from 'node:path';
const {chromium}=await import(process.env.PVZ_PLAYWRIGHT||'playwright-core');
const out=process.env.PVZ_QA_OUT||'/tmp/pvz-reduced-blasts';await mkdir(out,{recursive:true});
const browser=await chromium.launch({executablePath:'/Applications/Google Chrome.app/Contents/MacOS/Google Chrome',headless:true,args:['--no-sandbox']});
const page=await browser.newPage({viewport:{width:1100,height:750}}),errors=[],results=[];
page.on('pageerror',e=>errors.push(e.message));
const api=(...a)=>page.evaluate(a=>Module._pvz_sandbox_command(...[...a,0,0,0,0].slice(0,4)),a);
const hp=()=>page.evaluate(()=>Module._pvz_sandbox_zombie_data(0,4)+Module._pvz_sandbox_zombie_data(0,5));
async function click(x,y){const b=await page.locator('#canvas').boundingBox(),s=await page.evaluate(()=>[Module.canvas.width,Module.canvas.height]);await page.mouse.click(b.x+x*b.width/s[0],b.y+y*b.height/s[1]);await page.waitForTimeout(200);}
try{
 await page.goto(process.env.PVZ_QA_URL||'http://127.0.0.1:8097/');await page.waitForFunction(()=>!document.getElementById('start').disabled,undefined,{timeout:90000});
 await page.evaluate(async()=>{
  const FS=Module.FS;if(!FS.analyzePath('/saves/userdata').exists)FS.mkdir('/saves/userdata');
  const name=new TextEncoder().encode('BlastQA'),users=new Uint8Array(16+name.length),u=new DataView(users.buffer);
  u.setUint32(0,14,true);u.setUint16(4,1,true);u.setUint16(6,name.length,true);users.set(name,8);u.setUint32(8+name.length,1,true);u.setUint32(12+name.length,1,true);FS.writeFile('/saves/userdata/users.dat',users);
  const profile=new Uint8Array(4096),p=new DataView(profile.buffer);p.setUint32(0,12,true);p.setUint32(4,27,true);FS.writeFile('/saves/userdata/user1.dat',profile);
  await new Promise((r,j)=>FS.syncfs(false,e=>e?j(e):r()));
 });
 await page.locator('#start').click();await page.waitForTimeout(12000);await click(400,560);await page.waitForTimeout(4500);
 await click(260,348);await page.waitForFunction(()=>Module.canvas.width===1024);await api(8,0);await api(4,1);await api(5,4);
 const wanted=new Map([[334,600],[335,400],[331,600]]);
 for(let attempt=0;wanted.size&&attempt<240;++attempt){
  await api(6);await api(3,0,1,2);
  assert.equal(await api(2,32,7,2),1);assert.equal(await api(1,529,1,2),1);
  const before=await hp();await api(4,0);
  await page.waitForFunction(()=>Module._pvz_projectile_data(0,0)>=320,undefined,{timeout:5000,polling:10});await api(4,1);
  const style=await page.evaluate(()=>Module._pvz_projectile_data(0,0));await api(3,0,1,2);
  if(!wanted.has(style))continue;
  assert.equal(await page.evaluate(()=>Module._pvz_projectile_data(1,0)),-1,'only the first shot is present');
  assert.equal(await hp(),before,'isolated shot has not hit yet');
  const hits=await page.evaluate(s=>Module._pvz_everything_data(1,s-320),style);await api(4,0);
  await page.waitForFunction(([s,n])=>Module._pvz_everything_data(1,s-320)>n,[style,hits],{timeout:5000,polling:10});await api(4,1);
  const after=await hp();assert.equal(before-after,wanted.get(style));
  results.push({style,before,after,damage:before-after});wanted.delete(style);
  await page.screenshot({path:join(out,'blast-'+style+'.png')});console.log('Verified isolated blast',results.at(-1));
 }
 assert.equal(wanted.size,0,'all three naturally rolled explosive types observed');
 // The original Cherry Bomb still removes 1800 health from a fresh red giant.
 await api(6);assert.equal(await api(2,32,7,2),1);const nativeBefore=await hp();assert.equal(await api(1,2,7,2),1);await api(4,0);
 await page.waitForFunction(()=>Module._pvz_sandbox_command(9,0,0,0)===0,undefined,{timeout:5000,polling:10});await api(4,1);
 assert.equal(nativeBefore-await hp(),1800);assert.deepEqual(errors,[]);
 await writeFile(join(out,'report.json'),JSON.stringify({results,nativeCherryDamage:1800,errors},null,2));console.log('Reduced blasts and unchanged native Cherry Bomb passed',out);
}catch(e){await page.screenshot({path:join(out,'failure.png')}).catch(()=>{});throw e;}
finally{await browser.close();}
