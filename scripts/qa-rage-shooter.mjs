import assert from 'node:assert/strict';
import {mkdir,writeFile} from 'node:fs/promises';
import {join} from 'node:path';
const {chromium}=await import(process.env.PVZ_PLAYWRIGHT||'playwright-core');
const out=process.env.PVZ_QA_OUT||'/tmp/pvz-rage-shooter-qa';await mkdir(out,{recursive:true});
const browser=await chromium.launch({executablePath:process.env.PVZ_CHROME||'/Applications/Google Chrome.app/Contents/MacOS/Google Chrome',headless:true,args:['--no-sandbox','--autoplay-policy=no-user-gesture-required']});
const page=await browser.newPage({viewport:{width:1100,height:750},hasTouch:true}),errors=[],results={};page.on('pageerror',e=>errors.push(e.message));
async function point(x,y){const b=await page.locator('#canvas').boundingBox(),s=await page.evaluate(()=>[Module.canvas.width,Module.canvas.height]);return [b.x+x*b.width/s[0],b.y+y*b.height/s[1]];}
async function click(x,y){await page.mouse.click(...await point(x,y));await page.waitForTimeout(100);}
async function api(...v){return page.evaluate(v=>Module._pvz_sandbox_command(...[...v,0,0,0,0].slice(0,4)),v);}
async function pd(f){return page.evaluate(f=>Module._pvz_sandbox_plant_data(0,f),f);}
async function shots(){return page.evaluate(()=>{const a=[];for(let i=0;i<4096;i++){if(Module._pvz_projectile_data(i,0)<0)break;a.push(Array.from({length:8},(_,f)=>Module._pvz_projectile_data(i,f)));}return a;});}
async function shot(name){await page.screenshot({path:join(out,name+'.png')});}
try{
 await page.goto(process.env.PVZ_QA_URL||'http://127.0.0.1:8097/');await page.waitForFunction(()=>!document.getElementById('start').disabled,{},{timeout:90000});
 // Isolated synthetic profile, no real user storage is touched.
 await page.evaluate(async()=>{const FS=Module.FS;if(!FS.analyzePath('/saves/userdata').exists)FS.mkdir('/saves/userdata');
  const name=new TextEncoder().encode('RageQA'),users=new Uint8Array(16+name.length),u=new DataView(users.buffer);
  u.setUint32(0,14,true);u.setUint16(4,1,true);u.setUint16(6,name.length,true);users.set(name,8);u.setUint32(8+name.length,1,true);u.setUint32(12+name.length,1,true);FS.writeFile('/saves/userdata/users.dat',users);
  const profile=new Uint8Array(4096),p=new DataView(profile.buffer);p.setUint32(0,12,true);p.setUint32(4,2,true);FS.writeFile('/saves/userdata/user1.dat',profile);await new Promise((r,j)=>FS.syncfs(false,e=>e?j(e):r()));});
 await page.locator('#start').click();await page.waitForTimeout(12000);await click(400,560);await page.waitForTimeout(4500);await click(260,348);
 await page.waitForFunction(()=>Module.canvas.width===1024,{},{timeout:15000});await api(7);await api(1,500,1,2);await api(2,4,8,2);await api(4,0);
 await page.waitForFunction(()=>{for(let i=0;i<128;++i){const s=Module._pvz_projectile_data(i,0);if(s<0)break;if(s>=32&&s<=287)return true;}return false;},{},{timeout:6500});await api(4,1);
 const firstMiss=(await shots()).find(s=>s[0]>=32&&s[0]<=287);let minY=999,maxY=-999,up=false,down=false,turns=0,lastVy=firstMiss[4];const trace=[];
 for(let i=0;i<130;++i){await api(13);await page.waitForTimeout(20);const s=(await shots()).find(s=>s[0]===firstMiss[0]);assert.ok(s);assert.equal(s[3],3330);up||=s[4]<0;down||=s[4]>0;if(s[4]*lastVy<0)++turns;lastVy=s[4];minY=Math.min(minY,s[2]);maxY=Math.max(maxY,s[2]);trace.push(s);}
 assert.ok(up&&down&&turns>=2&&maxY-minY>50);results.repeatedUpDown=true;results.turns=turns;await writeFile(join(out,'floating-pea-trace.json'),JSON.stringify(trace));await shot('normal-random-amplitude-wave');
 await api(4,0);await page.waitForFunction(()=>Module._pvz_sandbox_plant_data(0,5)===100,{},{timeout:15000});await api(4,1);assert.equal(await pd(3),300);
 await shot('gradual-rage-100');
 await api(6);await click(718,61);await page.setViewportSize({width:844,height:390});await page.waitForTimeout(250);await shot('phone-automatic-rage-bar');
 // Legacy API and real body/bar taps cannot spend the accumulated rage.
 await api(4,0);assert.equal(await api(23,0,1,2),0);await page.touchscreen.tap(...await point(384,362));await page.touchscreen.tap(...await point(384,330));
 await page.waitForTimeout(250);assert.equal(await pd(4),0);assert.equal(await pd(5),100);assert.equal(await page.evaluate(()=>Module._pvz_rage_audio_data(0)),0);results.noManualRelease=true;
 await page.setViewportSize({width:1100,height:750});await page.waitForTimeout(250);
 for(const row of [1,2,3])await api(2,23,8,row);await api(5,2);
 await page.waitForFunction(()=>Module._pvz_sandbox_plant_data(0,5)===200,{},{timeout:14000});await api(4,1);assert.equal(await pd(4),0);await shot('rage-200-no-burst');results.noBurstAt200=true;await api(4,0);
 await page.waitForFunction(()=>Module._pvz_sandbox_plant_data(0,5)===280,{},{timeout:14000});await api(4,1);assert.equal(await pd(4),0);await shot('gradual-rage-280');await api(5,1);
 await page.evaluate(()=>{
  window.ragePacingTrace=[];let started=false;
  const sample=()=>{const phase=Module._pvz_sandbox_plant_data(0,4);started||=phase===1;
   window.ragePacingTrace.push({tick:Module._pvz_sandbox_zombie_data(0,10),phase,remaining:Module._pvz_sandbox_plant_data(0,7)});
   if(!started||phase===1)requestAnimationFrame(sample);
  };requestAnimationFrame(sample);
 });await api(4,0);
 await page.waitForFunction(()=>Module._pvz_sandbox_plant_data(0,4)===1,{},{timeout:22000});
 assert.deepEqual(await page.evaluate(()=>[0,1,2].map(f=>Module._pvz_rage_audio_data(f))),[1,1,1000]);results.releaseAudio=true;results.releaseVolume=1;
 await page.waitForTimeout(600);await api(4,1);const fan=await shots(),spread=fan.filter(s=>s[0]===9);
 assert.ok(spread.length>=7&&spread.some(s=>s[4]<0)&&spread.some(s=>s[4]>0));
 const speeds=spread.map(s=>Math.hypot(s[3],s[4])/1000);assert.ok(Math.max(...speeds)-Math.min(...speeds)>1.0);assert.ok(speeds.every(v=>v>=3.448&&v<=5.702));results.irregularSpread=true;results.slowerBurstPeas=true;
 assert.equal(await pd(3),300);await shot('automatic-50-random-burst');results.automaticBurst=true;
 await page.waitForTimeout(300);assert.deepEqual(await shots(),fan);results.pauseSafe=true;
 await api(4,0);await page.waitForFunction(()=>Module._pvz_sandbox_plant_data(0,4)===0,{},{timeout:4000});await api(4,1);await shot('original-colour-restored');assert.equal(await pd(3),300);
 const pacing=await page.evaluate(()=>window.ragePacingTrace),start=pacing.find(s=>s.phase===1),end=pacing.at(-1);
 assert.equal(start.remaining,50);assert.equal(end.phase,0);assert.equal(end.remaining,0);
 const duration=end.tick-start.tick;assert.ok(duration>=297&&duration<=302,`50 shots should finish in 300 native ticks, measured ${duration}`);
 const active=pacing.filter(s=>s.phase===1);for(let i=1;i<active.length;++i)assert.ok(active[i].remaining<=active[i-1].remaining);
 results.burstDurationTicks=duration;results.all50Paced=true;await writeFile(join(out,'three-second-burst-trace.json'),JSON.stringify(pacing));
 await page.waitForFunction(()=>Module._pvz_rage_audio_data(0)===0,{},{timeout:3000});
 await api(7);await api(1,500,1,1);await api(1,500,1,3);await api(2,23,8,1);await api(2,23,8,3);await api(5,2);await api(4,0);
 await page.waitForFunction(()=>[0,1].every(i=>Module._pvz_sandbox_plant_data(i,5)>=100),{},{timeout:15000});
 assert.equal(await api(23,0,1,1),0);assert.equal(await api(23,0,1,3),0);
 await page.waitForFunction(()=>[0,1].every(i=>Module._pvz_sandbox_plant_data(i,4)===1),{},{timeout:12000});
 assert.deepEqual(await page.evaluate(()=>[0,1,2].map(f=>Module._pvz_rage_audio_data(f))),[1,1,1000]);results.noVoiceStacking=true;
 // Real native support plants, not injected coordinates or synthetic rage.
 for(const [name,map,support,barY] of [['pot',0,33,357],['pool',1,16,332]]){
  await api(8,map);assert.ok(await api(1,support,1,2)>0);assert.ok(await api(1,500,1,2)>0);
  if(map)assert.equal(await api(2,10,8,0),-5,'ducky remains water-only');
  assert.ok(await api(2,map?10:23,8,2)>0,'target must really spawn');await api(5,2);await api(4,0);
  await page.waitForFunction(()=>Module._pvz_sandbox_plant_data(1,5)>=100,{},{timeout:12000});await api(4,1);await api(5,1);await api(6);
  await page.setViewportSize({width:844,height:390});await page.waitForTimeout(250);await click(718,61);await shot(name+'-automatic-bar');
  const heat=await page.evaluate(()=>Module._pvz_sandbox_plant_data(1,5));await api(4,0);await page.touchscreen.tap(...await point(384,barY));await page.waitForTimeout(200);
  assert.equal(await page.evaluate(()=>Module._pvz_sandbox_plant_data(1,4)),0);assert.equal(await page.evaluate(()=>Module._pvz_sandbox_plant_data(1,5)),heat);
  assert.equal(await page.evaluate(()=>Module._pvz_sandbox_plant_data(1,3)),300);results[name+'NoManualRelease']=true;
 }
 await api(8,0);await api(1,0,0,2);await api(2,4,5,2);await api(5,1);
 const helmet=await page.evaluate(()=>Module._pvz_sandbox_zombie_data(0,5));await api(4,0);
 await page.waitForFunction(hp=>Module._pvz_sandbox_zombie_data(0,5)<hp,helmet,{timeout:7000});
 let lastX=await page.evaluate(()=>Module._pvz_sandbox_zombie_data(0,2));
 for(let i=0;i<20;++i){await page.waitForTimeout(50);const [x,timer,speed]=await page.evaluate(()=>[2,8,9].map(f=>Module._pvz_sandbox_zombie_data(0,f)));assert.ok(x<=lastX);assert.equal(timer,0);assert.equal(speed,100);lastX=x;}
 await api(4,1);await shot('ordinary-pea-no-knockback');results.ordinaryHitNoKnockback=true;
 assert.deepEqual(errors,[]);await writeFile(join(out,'report.json'),JSON.stringify({results,errors},null,2));console.log('Rage shooter QA passed',results);
}catch(e){await shot('failure');console.log('DATA',await pd(4),await pd(5),await shots());console.log('ENGINE',await page.evaluate(()=>window.pvzEngineLog));throw e;}
finally{await browser.close();}
