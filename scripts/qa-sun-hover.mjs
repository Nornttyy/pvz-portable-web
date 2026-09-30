// Isolated browser/profile; never opens or edits a player's real save.
import assert from 'node:assert/strict';
import {mkdir,writeFile} from 'node:fs/promises';
import {join} from 'node:path';
const {chromium}=await import(process.env.PVZ_PLAYWRIGHT||'playwright-core');
const out=process.env.PVZ_QA_OUT||'/tmp/pvz-sun-hover-qa';await mkdir(out,{recursive:true});
const browser=await chromium.launch({executablePath:process.env.PVZ_CHROME||'/Applications/Google Chrome.app/Contents/MacOS/Google Chrome',headless:true,args:['--no-sandbox','--autoplay-policy=no-user-gesture-required']});
const page=await browser.newPage({viewport:{width:1100,height:750},hasTouch:true}),errors=[],results={};
page.on('pageerror',e=>errors.push(e.message));
async function point(x,y){const b=await page.locator('#canvas').boundingBox(),s=await page.evaluate(()=>[Module.canvas.width,Module.canvas.height]);return [b.x+x*b.width/s[0],b.y+y*b.height/s[1]];}
async function click(x,y){await page.mouse.click(...await point(x,y));await page.waitForTimeout(150);}
async function hover(x,y){await page.mouse.move(...await point(x,y));await page.waitForTimeout(160);}
async function shot(name){await page.screenshot({path:join(out,name+'.png')});}
async function api(...v){return page.evaluate(v=>Module._pvz_sandbox_command(...[...v,0,0,0,0].slice(0,4)),v);}
async function ad(i,f){return page.evaluate(([i,f])=>Module._pvz_adventure_power_data(i,f),[i,f]);}
async function sd(i,f){return page.evaluate(([i,f])=>Module._pvz_adventure_seed_data(i,f),[i,f]);}
async function sun(){await hover(780,590);await page.waitForFunction(()=>Module._pvz_adventure_power_data(0,10)>=0,{},{timeout:25000});return [await ad(0,10),await ad(0,11)];}
async function hoverSun(pos=undefined){const xy=pos||await sun(),before=await ad(-1,2),downs=await page.evaluate(()=>window.qaMouseDowns);
 await hover(...xy);await page.waitForFunction(n=>Module._pvz_adventure_power_data(-1,2)===n,before+25,{timeout:4000});assert.equal(await page.evaluate(()=>window.qaMouseDowns),downs);
 return before+25;
}
try{
 await page.goto(process.env.PVZ_QA_URL||'http://127.0.0.1:8097/');await page.waitForFunction(()=>!document.getElementById('start').disabled,{},{timeout:90000});
 await page.evaluate(async()=>{
  const FS=Module.FS;if(!FS.analyzePath('/saves/userdata').exists)FS.mkdir('/saves/userdata');
  const name=new TextEncoder().encode('HoverQA'),users=new Uint8Array(16+name.length),u=new DataView(users.buffer);
  u.setUint32(0,14,true);u.setUint16(4,1,true);u.setUint16(6,name.length,true);users.set(name,8);u.setUint32(8+name.length,1,true);u.setUint32(12+name.length,1,true);FS.writeFile('/saves/userdata/users.dat',users);
  const profile=new Uint8Array(4096),p=new DataView(profile.buffer);p.setUint32(0,12,true);p.setUint32(4,2,true);FS.writeFile('/saves/userdata/user1.dat',profile);
  await new Promise((resolve,reject)=>FS.syncfs(false,e=>e?reject(e):resolve()));
  window.qaMouseDowns=0;document.addEventListener('mousedown',()=>++window.qaMouseDowns);
 });
 await page.locator('#start').click();await page.waitForTimeout(12000);await click(400,560);await page.waitForTimeout(4500);
 await click(260,348);await page.waitForFunction(()=>Module.canvas.width===1024,{},{timeout:20000});
 await api(7);await api(1,503,4,2);await shot('flower-normal');await api(2,0,4,2);await api(4,0);
 await page.waitForFunction(()=>Module._pvz_sandbox_plant_data(0,4)===1,{},{timeout:18000});await page.waitForTimeout(250);await api(4,1);await shot('flower-hiding');
 await api(6);await api(4,0);await page.waitForFunction(()=>Module._pvz_sandbox_plant_data(0,4)===0,{},{timeout:1500});await api(4,1);await shot('flower-recovered');results.flowerRecovery=true;
 await api(15);await page.waitForTimeout(900);await click(560,135);
 await page.waitForFunction(()=>Module._pvz_adventure_power_data(-1,8)===5,{},{timeout:35000});
 assert.equal(await sd(1,0),503);await click(await sd(1,5)+25,await sd(1,6)+35);await click(80,330);assert.equal(await ad(0,0),503);assert.equal(await ad(-1,2),0);
 await hoverSun();results.hoverWithoutClick=true;await shot('adventure-hover-collected');
 const pos=await sun();await click(748,14);const before=await ad(-1,2);assert.equal(await ad(-1,6),1);
 await hover(...pos);await page.waitForTimeout(500);assert.equal(await ad(-1,2),before);assert.deepEqual([await ad(0,10),await ad(0,11)],pos);results.pauseSafe=true;
 await hover(780,590);await click(400,450);await hoverSun();assert.equal(await ad(-1,2),50);
 const heldSun=await sun();await click(await sd(1,5)+25,await sd(1,6)+35);await hoverSun(heldSun);await click(80,230);
 assert.equal(await ad(1,0),503);assert.equal(await ad(-1,2),25);results.heldSeedPreserved=true;
 await page.setViewportSize({width:844,height:390});const mobileSun=await sun(),mobileBefore=await ad(-1,2);
 await page.touchscreen.tap(...await point(...mobileSun));await page.waitForFunction(n=>Module._pvz_adventure_power_data(-1,2)===n,mobileBefore+25,{timeout:4000});results.mobileTap=true;await shot('mobile-tap-collected');
 assert.deepEqual(errors,[]);await writeFile(join(out,'report.json'),JSON.stringify({results,errors},null,2));console.log('Sun hover QA passed',results);
}catch(e){await shot('failure');console.log('ADVENTURE',await ad(-1,2),await ad(-1,8),await ad(-1,6));console.log('ENGINE',await page.evaluate(()=>window.pvzEngineLog));throw e;}
finally{await browser.close();}
