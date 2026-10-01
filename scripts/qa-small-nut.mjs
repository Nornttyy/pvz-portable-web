// Exercise the shipped WASM in an isolated profile, never a player's storage.
import assert from 'node:assert/strict';
import {mkdir,writeFile} from 'node:fs/promises';
import {join} from 'node:path';
const {chromium}=await import(process.env.PVZ_PLAYWRIGHT||'playwright-core');
const out=process.env.PVZ_QA_OUT||'/tmp/pvz-small-nut-qa';await mkdir(out,{recursive:true});
const almanac=process.env.PVZ_QA_ALMANAC==='1';
const browser=await chromium.launch({executablePath:'/Applications/Google Chrome.app/Contents/MacOS/Google Chrome',headless:true,args:['--no-sandbox','--autoplay-policy=no-user-gesture-required']});
const context=await browser.newContext({viewport:{width:1100,height:750},hasTouch:true}),page=await context.newPage(),results={},errors=[];
page.on('pageerror',e=>errors.push(e.message));
const wait=(f,a,t=30000)=>page.waitForFunction(f,a,{timeout:t});
const api=(...a)=>page.evaluate(a=>Module._pvz_sandbox_command(...[...a,0,0,0,0].slice(0,4)),a);
const seed=(i,f)=>page.evaluate(([i,f])=>Module._pvz_adventure_seed_data(i,f),[i,f]);
const snap=async n=>{await page.mouse.move(0,0);await page.waitForTimeout(150);await page.screenshot({path:join(out,n+'.png')});};
async function click(x,y,touch=false){const b=await page.locator('#canvas').boundingBox(),s=await page.evaluate(()=>[Module.canvas.width,Module.canvas.height]);const pos=[b.x+x*b.width/s[0],b.y+y*b.height/s[1]];if(touch)await page.touchscreen.tap(...pos);else await page.mouse.click(...pos);await page.waitForTimeout(150);}
async function card(base,touch=false){const slot=base===52?8:base===53?9:base>=8?base+2:base;await click(47+slot%9*53,163+Math.floor(slot/9)*73,touch);}
async function quit(){await click(748,14);await click(400,401);await page.waitForTimeout(250);await click(305,394);await wait(()=>Module._pvz_adventure_power_data(-1,5)===-1);await page.waitForTimeout(1800);}
try{
 await page.goto(process.env.PVZ_QA_URL||'http://127.0.0.1:8097/');await wait(()=>!document.getElementById('start').disabled,undefined,90000);
 await page.evaluate(async(almanac)=>{
  const FS=Module.FS;if(!FS.analyzePath('/saves/userdata').exists)FS.mkdir('/saves/userdata');
  const name=new TextEncoder().encode('SmallNutQA'),users=new Uint8Array(16+name.length),u=new DataView(users.buffer);
  u.setUint32(0,14,true);u.setUint16(4,1,true);u.setUint16(6,name.length,true);users.set(name,8);u.setUint32(8+name.length,1,true);u.setUint32(12+name.length,1,true);FS.writeFile('/saves/userdata/users.dat',users);
  const profile=new Uint8Array(4096),p=new DataView(profile.buffer);p.setUint32(0,12,true);p.setUint32(4,almanac?49:9,true);
  if(almanac)for(let item=0;item<9;++item)p.setUint32(416+item*4,1,true);
  FS.writeFile('/saves/userdata/user1.dat',profile);
  await new Promise((r,j)=>FS.syncfs(false,e=>e?j(e):r()));
 },almanac);
 await page.locator('#start').click();await page.waitForTimeout(12000);await click(400,560);await page.waitForTimeout(4500);await click(560,135);await page.waitForTimeout(15000);
 await snap('chooser-independent-small-nut');
 if(almanac){
  // Native almanac, including localized title, cost, damaged-free preview.
  await click(605,584);await page.waitForTimeout(500);await click(210,366);await click(47,199);await page.waitForTimeout(500);await snap('almanac-small-nut');
  await page.setViewportSize({width:844,height:390});await page.waitForTimeout(1000);await snap('mobile-almanac');
  results.fullRosterAndAlmanac=true;
 }else{
 await page.setViewportSize({width:844,height:390});await page.waitForTimeout(1000);
 await card(53,true);await snap('mobile-select');await click(110,43,true);await card(53,true);await snap('mobile-return-and-reselect');
 await page.setViewportSize({width:1100,height:750});await page.waitForTimeout(500);
 for(const base of [3,0,1,4,52])await card(base);await click(258,566);await wait(()=>Module._pvz_adventure_power_data(-1,5)===1);
 results.bank=await page.evaluate(()=>Array.from({length:6},(_,i)=>[0,1,2].map(f=>Module._pvz_adventure_seed_data(i,f))));
 assert.deepEqual(results.bank.map(x=>x[0]),[523,501,500,520,4,519]);assert.deepEqual(results.bank[0],[523,25,600]);assert.deepEqual(results.bank[1],[501,50,1200]);
 await wait(()=>Module._pvz_adventure_seed_data(0,4)===1);await click(await seed(0,5)+25,await seed(0,6)+35);await click(480,330);
 await wait(()=>Module._pvz_adventure_power_data(0,0)===523);assert.equal(await page.evaluate(()=>Module._pvz_adventure_power_data(0,3)),800);assert.ok(await seed(0,3)>550);await snap('adventure-small-nut');
 await page.waitForTimeout(2800);assert.ok(await seed(0,3)>200);assert.equal(await seed(0,4),0);
 await click(748,14);results.pausedRemaining=await seed(0,3);await page.waitForTimeout(350);assert.equal(await seed(0,3),results.pausedRemaining);
 await click(400,401);await page.waitForTimeout(250);await click(305,394);await wait(()=>Module._pvz_adventure_power_data(-1,5)===-1);await page.waitForTimeout(1800);
 await click(560,135);await wait(()=>Module._pvz_adventure_power_data(-1,5)===1);
 assert.equal(await seed(0,3),results.pausedRemaining);assert.equal(await seed(0,0),523);assert.equal(await seed(5,0),519);assert.equal(await page.evaluate(()=>Module._pvz_adventure_power_data(0,3)),800);results.exactSaveResume=true;
 await click(280,371);await wait(()=>Module._pvz_adventure_seed_data(0,3)===0);assert.equal(await seed(0,4),1);
 await click(await seed(0,5)+25,await seed(0,6)+35);await click(480,430);await wait(()=>Module._pvz_adventure_power_data(1,0)===523);results.secondPlantHealth=await page.evaluate(()=>Module._pvz_adventure_power_data(1,3));assert.equal(results.secondPlantHealth,800);
 await snap('adventure-two-small-nuts');await quit();await click(260,348);await wait(()=>Module.canvas.width===1024);
 await api(7);await api(4,1);await api(1,501,2,1);await api(1,523,3,1);await api(1,523,5,3);await api(2,0,5,3);
 assert.deepEqual(await page.evaluate(()=>[0,1,2].map(i=>Module._pvz_sandbox_plant_data(i,3))),[4000,800,800]);
 await api(13);await page.waitForTimeout(100);await snap('sandbox-size-comparison');
 await api(5,4);await api(4,0);await wait(()=>Module._pvz_sandbox_plant_data(2,3)<533);await api(4,1);results.firstCrackHP=await page.evaluate(()=>Module._pvz_sandbox_plant_data(2,3));assert.ok(results.firstCrackHP>266);await snap('small-nut-damage-one');
 await api(4,0);await wait(()=>Module._pvz_sandbox_plant_data(2,3)<266);await api(4,1);results.secondCrackHP=await page.evaluate(()=>Module._pvz_sandbox_plant_data(2,3));assert.ok(results.secondCrackHP>0);await snap('small-nut-damage-two');
 assert.equal(await page.evaluate(()=>Module._pvz_sandbox_zombie_data(0,4)),270);results.noRetaliation=true;
 await api(4,0);await wait(()=>Module._pvz_sandbox_plant_data(2,0)===-1);await api(4,1);assert.equal(await page.evaluate(()=>Module._pvz_sandbox_plant_data(1,3)),800);results.eatenNormally=true;
 await page.setViewportSize({width:844,height:390});await page.waitForTimeout(1000);await snap('mobile-sandbox');
 }
 assert.deepEqual(errors,[]);await writeFile(join(out,'report.json'),JSON.stringify({results,errors},null,2));console.log('Small nut QA passed',out,results);
}catch(e){await snap('failure');await writeFile(join(out,'failure.json'),JSON.stringify({error:String(e),results,errors,log:await page.evaluate(()=>window.pvzEngineLog)},null,2));throw e;}
finally{await browser.close();}
