// Real adventure save/reload and mobile input in an isolated Chromium profile.
// Only the initial QA profile is synthetic; battle, rage and saves use game UI.
import assert from 'node:assert/strict';
import {mkdir,writeFile} from 'node:fs/promises';
import {join} from 'node:path';
const {chromium}=await import(process.env.PVZ_PLAYWRIGHT||'playwright-core');
const out=process.env.PVZ_QA_OUT||'/tmp/pvz-rage-resume-qa';await mkdir(out,{recursive:true});
const browser=await chromium.launch({executablePath:process.env.PVZ_CHROME||'/Applications/Google Chrome.app/Contents/MacOS/Google Chrome',headless:true,args:['--no-sandbox','--autoplay-policy=no-user-gesture-required']});
const page=await browser.newPage({viewport:{width:1100,height:750},hasTouch:true}),errors=[],results={};
page.on('pageerror',e=>errors.push(e.message));
async function point(x,y){const b=await page.locator('#canvas').boundingBox(),s=await page.evaluate(()=>[Module.canvas.width,Module.canvas.height]);return [b.x+x*b.width/s[0],b.y+y*b.height/s[1]];}
async function click(x,y){await page.mouse.click(...await point(x,y));await page.waitForTimeout(80);}
async function tap(x,y){await page.touchscreen.tap(...await point(x,y));await page.waitForTimeout(80);}
async function shot(name){await page.screenshot({path:join(out,name+'.png')});}
async function ad(i,f){return page.evaluate(([i,f])=>Module._pvz_adventure_power_data(i,f),[i,f]);}
async function seed(i,f){return page.evaluate(([i,f])=>Module._pvz_adventure_seed_data(i,f),[i,f]);}
async function snapshot(){return page.evaluate(()=>{
 const ad=Module._pvz_adventure_power_data,plants=[],shots=[];
 // The replaced native firing clock is intentionally disabled (9998/9999).
 // The custom saved delay, rage and remaining count control this character.
 const fields={type:0,col:1,row:2,health:3,phase:4,rage:5,timer:6,base:8,asleep:9,remaining:14};
 for(let i=0;i<180&&ad(i,0)>=0;++i)plants.push(Object.fromEntries(Object.entries(fields).map(([k,f])=>[k,ad(i,f)])));
 for(let i=0;i<4096&&Module._pvz_projectile_data(i,0)>=0;++i)shots.push(Array.from({length:8},(_,f)=>Module._pvz_projectile_data(i,f)));
 return {plants,shots,sun:ad(-1,2),level:ad(-1,3)};
});}
async function boot(seedProfile=false){
 await page.goto(process.env.PVZ_QA_URL||'http://127.0.0.1:8097/');await page.waitForFunction(()=>!document.getElementById('start').disabled,{},{timeout:90000});
 if(seedProfile)await page.evaluate(async()=>{
  const FS=Module.FS;if(!FS.analyzePath('/saves/userdata').exists)FS.mkdir('/saves/userdata');
  const name=new TextEncoder().encode('ResumeQA'),users=new Uint8Array(16+name.length),u=new DataView(users.buffer);
  u.setUint32(0,14,true);u.setUint16(4,1,true);u.setUint16(6,name.length,true);users.set(name,8);u.setUint32(8+name.length,1,true);u.setUint32(12+name.length,1,true);
  FS.writeFile('/saves/userdata/users.dat',users);const profile=new Uint8Array(4096),p=new DataView(profile.buffer);p.setUint32(0,12,true);p.setUint32(4,1,true);FS.writeFile('/saves/userdata/user1.dat',profile);
  await new Promise((r,j)=>FS.syncfs(false,e=>e?j(e):r()));
 });
 await page.locator('#start').click();await page.waitForTimeout(12000);await click(400,560);await page.waitForTimeout(4500);
}
async function play(){await click(560,135);await page.waitForFunction(()=>Module._pvz_adventure_power_data(-1,5)===1,{},{timeout:40000});}
try{
 await boot(true);await play();await page.waitForFunction(()=>Module._pvz_adventure_power_data(-1,8)===1,{},{timeout:15000});
 assert.equal(await seed(0,0),500);await tap(await seed(0,5)+25,await seed(0,6)+35);await click(80,330);
 const deadline=Date.now()+55000;
 while(await ad(-1,2)<100){assert.ok(Date.now()<deadline,'sun collection deadline');const sun=[await ad(0,10),await ad(0,11)];if(sun[0]>=0)await click(...sun);await page.waitForTimeout(150);}
 await tap(await seed(0,5)+25,await seed(0,6)+35);await click(240,330);
 await page.waitForFunction(()=>Module._pvz_adventure_power_data(0,5)>=100,{},{timeout:65000});
 await shot('ready-highlight');await tap(80,330);
 await page.waitForFunction(()=>Module._pvz_adventure_power_data(0,4)===1,{},{timeout:1500});
 assert.equal(await page.evaluate(()=>Module._pvz_rage_audio_data(0)),1);
 await page.waitForTimeout(350);await click(748,14);
 const before=await snapshot(),remaining=before.plants[0].remaining;
 assert.ok(remaining>0&&remaining<150);assert.ok(before.shots.some(s=>s[0]===9));
 // Return while the dedicated release voice is still active.
 assert.equal(await page.evaluate(()=>Module._pvz_rage_audio_data(0)),1);
 await click(400,401);await page.waitForTimeout(250);await click(305,394);
 await page.waitForFunction(()=>Module._pvz_adventure_power_data(-1,5)===-1,{},{timeout:10000});
 assert.equal(await page.evaluate(()=>Module._pvz_rage_audio_data(0)),0);results.exitStopsVoice=true;
 await page.evaluate(()=>new Promise((r,j)=>Module.FS.syncfs(false,e=>e?j(e):r())));
 // Reload the actual browser page, not just the in-memory board.
 await boot();await play();
 const restored=await snapshot();assert.deepEqual(restored,before,'native plants, sun, progress and in-flight projectile states survive exactly');
 assert.equal(await page.evaluate(()=>Module._pvz_rage_audio_data(0)),0);results.exactRestore=true;
 await page.waitForTimeout(250);assert.deepEqual(await snapshot(),restored,'continue dialog freezes the restored volley');
 await page.setViewportSize({width:844,height:390});await shot('restored-on-phone');
 await page.evaluate(()=>{
  window.rageResumeTrace=[];
  const sample=()=>{const ad=Module._pvz_adventure_power_data;window.rageResumeTrace.push({remaining:ad(0,14),phase:ad(0,4),audio:Module._pvz_rage_audio_data(0)});
   if(ad(0,4)===1)requestAnimationFrame(sample);
  };requestAnimationFrame(sample);
 });
 await tap(280,371);await page.waitForFunction(()=>Module._pvz_adventure_power_data(0,4)===0,{},{timeout:5000});
 const trace=await page.evaluate(()=>window.rageResumeTrace),after=await snapshot();
 assert.ok(trace.length>5&&trace.at(-1).remaining===0);
 for(let i=0;i<trace.length;++i){assert.equal(trace[i].audio,0,'resuming is not a new release');assert.ok(trace[i].remaining>=0&&trace[i].remaining<=(i?trace[i-1].remaining:remaining),'remaining volley must not restart');}
 assert.equal(after.plants[0].health,before.plants[0].health);assert.equal(after.sun,before.sun);assert.equal(after.level,before.level);
 assert.equal(after.plants[0].remaining,0);await shot('resumed-volley-complete');
 results.remainingRestored=remaining;results.noExtraVolley=true;results.noRepeatedVoice=true;results.mobileResume=true;
 assert.deepEqual(errors,[]);await writeFile(join(out,'report.json'),JSON.stringify({results,before,restored,after,trace,errors},null,2));console.log('Rage save/resume QA passed',out,results);
}catch(e){await shot('failure');console.log('STATE',await snapshot());console.log('ENGINE',await page.evaluate(()=>window.pvzEngineLog));throw e;}
finally{await browser.close();}
