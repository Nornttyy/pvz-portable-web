// Shipped WASM, isolated synthetic profile; player saves are never accessed.
import assert from 'node:assert/strict';
import {mkdir,writeFile} from 'node:fs/promises';
import {join} from 'node:path';
const {chromium}=await import(process.env.PVZ_PLAYWRIGHT||'playwright-core');
const out=process.env.PVZ_QA_OUT||'/tmp/pvz-nut-cooldown-qa';await mkdir(out,{recursive:true});
const browser=await chromium.launch({executablePath:'/Applications/Google Chrome.app/Contents/MacOS/Google Chrome',headless:true,args:['--no-sandbox','--autoplay-policy=no-user-gesture-required']});
const page=await browser.newPage({viewport:{width:1100,height:750}}),errors=[],results={};page.on('pageerror',e=>errors.push(e.message));
const wait=(f,a,t=30000)=>page.waitForFunction(f,a,{timeout:t});
const seed=(i,f)=>page.evaluate(([i,f])=>Module._pvz_adventure_seed_data(i,f),[i,f]);
const snap=n=>page.screenshot({path:join(out,n+'.png')});
async function click(x,y){const b=await page.locator('#canvas').boundingBox(),s=await page.evaluate(()=>[Module.canvas.width,Module.canvas.height]);await page.mouse.click(b.x+x*b.width/s[0],b.y+y*b.height/s[1]);await page.waitForTimeout(100);}
try{
 await page.goto(process.env.PVZ_QA_URL||'http://127.0.0.1:8097/');await wait(()=>!document.getElementById('start').disabled,undefined,90000);
 await page.evaluate(async()=>{
  const FS=Module.FS;if(!FS.analyzePath('/saves/userdata').exists)FS.mkdir('/saves/userdata');
  const name=new TextEncoder().encode('NutQA'),users=new Uint8Array(16+name.length),u=new DataView(users.buffer);
  u.setUint32(0,14,true);u.setUint16(4,1,true);u.setUint16(6,name.length,true);users.set(name,8);u.setUint32(8+name.length,1,true);u.setUint32(12+name.length,1,true);FS.writeFile('/saves/userdata/users.dat',users);
  const profile=new Uint8Array(4096),p=new DataView(profile.buffer);p.setUint32(0,12,true);p.setUint32(4,9,true);FS.writeFile('/saves/userdata/user1.dat',profile);
  await new Promise((r,j)=>FS.syncfs(false,e=>e?j(e):r()));
 });
 await page.locator('#start').click();await page.waitForTimeout(12000);await click(400,560);await page.waitForTimeout(4500);
 await click(560,135);await page.waitForTimeout(15000);
 for(const base of [0,1,3,5,6,7])await click(47+(base%9)*53,158+Math.floor(base/9)*70);
 await click(258,566);await wait(()=>Module._pvz_adventure_power_data(-1,5)===1);
 results.bank=await page.evaluate(()=>Array.from({length:6},(_,i)=>[0,1,2].map(f=>Module._pvz_adventure_seed_data(i,f))));
 assert.equal(results.bank[2][0],501);assert.equal(results.bank[2][2],1200);assert.equal(results.bank[0][2],300);assert.equal(results.bank[1][2],300);
 await wait(()=>Module._pvz_adventure_seed_data(2,4)===1);await click(await seed(2,5)+25,await seed(2,6)+35);await click(480,330);
 assert.equal(await page.evaluate(()=>Module._pvz_adventure_power_data(0,0)),501);
 assert.ok(await seed(2,3)>1100);await snap('planted-12-second-cooldown');
 await page.waitForTimeout(3300);assert.ok(await seed(2,3)>700);assert.equal(await seed(2,4),0);results.notReadyAfterThreeSeconds=true;
 await click(748,14);results.pausedRemaining=await seed(2,3);await page.waitForTimeout(500);assert.equal(await seed(2,3),results.pausedRemaining);
 await click(400,401);await page.waitForTimeout(250);await click(305,394);await wait(()=>Module._pvz_adventure_power_data(-1,5)===-1);
 // The board is disposed before the selector finishes its entrance animation.
 await page.waitForTimeout(1800);await click(560,135);await wait(()=>Module._pvz_adventure_power_data(-1,5)===1);
 assert.equal(await seed(2,2),1200);assert.equal(await seed(2,3),results.pausedRemaining);results.exactSaveResume=true;await snap('resumed-same-cooldown');
 await click(280,371);await wait(()=>Module._pvz_adventure_seed_data(2,3)===0,undefined,20000);assert.equal(await seed(2,4),1);results.readyAfterCooldown=true;await snap('ready');
 assert.deepEqual(errors,[]);await writeFile(join(out,'report.json'),JSON.stringify({results,errors},null,2));console.log('Nut cooldown QA passed',out,results);
}catch(e){await snap('failure');await writeFile(join(out,'failure.json'),JSON.stringify({error:String(e),results,errors,log:await page.evaluate(()=>window.pvzEngineLog)},null,2));throw e;}
finally{await browser.close();}
