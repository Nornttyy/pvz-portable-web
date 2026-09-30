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
 await page.waitForFunction(()=>Module._pvz_sandbox_plant_data(0,5)===20,{},{timeout:10000});await api(4,1);
 const first=(await shots())[0];assert.ok(first[0]>=1&&first[0]<=8);let positive=false,negative=false,minY=999,maxY=-999;
 for(let i=0;i<95;++i){await api(13);await page.waitForTimeout(20);const s=(await shots())[0];assert.ok(s);positive||=s[4]>0;negative||=s[4]<0;minY=Math.min(minY,s[2]);maxY=Math.max(maxY,s[2]);}
 assert.ok(positive&&negative&&maxY-minY>60);results.wobbleRange=maxY-minY;await shot('normal-wavy-pea');
 await api(4,0);await page.waitForFunction(()=>Module._pvz_sandbox_plant_data(0,5)===100,{},{timeout:15000});await api(4,1);assert.equal(await pd(3),300);
 await api(6);await click(718,61);await api(4,0);await page.touchscreen.tap(...await point(384,330));await page.waitForFunction(()=>Module._pvz_sandbox_plant_data(0,4)===1,{},{timeout:1500});
 assert.deepEqual(await page.evaluate(()=>[0,1,2].map(f=>Module._pvz_rage_audio_data(f))),[1,1,600]);results.releaseAudio=true;
 await page.waitForTimeout(850);await api(4,1);const fan=await shots(),spread=fan.filter(s=>s[0]===9);
 assert.ok(spread.length>=10&&spread.some(s=>s[4]<0)&&spread.some(s=>s[4]>0));assert.equal(await pd(3),300);await shot('manual-50-pea-fan');
 await page.waitForTimeout(300);assert.deepEqual(await shots(),fan);results.pauseSafe=true;
 await api(4,0);await page.waitForFunction(()=>Module._pvz_sandbox_plant_data(0,4)===0,{},{timeout:4000});assert.equal(await pd(5),0);assert.equal(await pd(3),300);results.manualBurst=true;
 await page.waitForFunction(()=>Module._pvz_rage_audio_data(0)===0,{},{timeout:3000});
 await api(7);await api(1,500,0,2);for(const row of [1,2,3])await api(2,23,8,row);await api(4,0);await api(5,2);
 await page.waitForFunction(()=>Module._pvz_sandbox_plant_data(0,4)===1,{},{timeout:22000});assert.equal(await page.evaluate(()=>Module._pvz_rage_audio_data(0)),1);await api(5,1);await page.waitForTimeout(700);await api(4,1);
 assert.equal(await pd(3),300);assert.ok((await shots()).some(s=>s[0]===9));await shot('automatic-red-burst');results.automaticBurst=true;
 await api(4,0);await page.waitForFunction(()=>Module._pvz_sandbox_plant_data(0,4)===0,{},{timeout:4000});await api(4,1);await shot('original-colour-restored');assert.equal(await pd(3),300);
 await page.waitForFunction(()=>Module._pvz_rage_audio_data(0)===0,{},{timeout:3000});
 await api(7);await api(1,500,1,1);await api(1,500,1,3);await api(2,23,8,1);await api(2,23,8,3);await api(5,2);await api(4,0);
 await page.waitForFunction(()=>[0,1].every(i=>Module._pvz_sandbox_plant_data(i,5)>=100),{},{timeout:15000});
 await page.evaluate(()=>{Module._pvz_sandbox_command(23,0,1,1);Module._pvz_sandbox_command(23,0,1,3);});
 assert.equal(await page.evaluate(()=>Module._pvz_rage_audio_data(1)),1);results.noVoiceStacking=true;
 assert.deepEqual(errors,[]);await writeFile(join(out,'report.json'),JSON.stringify({results,errors},null,2));console.log('Rage shooter QA passed',results);
}catch(e){await shot('failure');console.log('DATA',await pd(4),await pd(5),await shots());console.log('ENGINE',await page.evaluate(()=>window.pvzEngineLog));throw e;}
finally{await browser.close();}
