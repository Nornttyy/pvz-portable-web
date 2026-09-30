// Real WASM mechanics, audio channels and native adventure cards in a disposable profile.
import assert from 'node:assert/strict';
import {mkdir,writeFile} from 'node:fs/promises';
import {join} from 'node:path';
const {chromium}=await import(process.env.PVZ_PLAYWRIGHT||'playwright-core');
const out=process.env.PVZ_QA_OUT||'/tmp/pvz-gag-batch-qa';await mkdir(out,{recursive:true});
const browser=await chromium.launch({executablePath:process.env.PVZ_CHROME||'/Applications/Google Chrome.app/Contents/MacOS/Google Chrome',headless:true,args:['--no-sandbox','--autoplay-policy=no-user-gesture-required']});
const page=await browser.newPage({viewport:{width:1100,height:750},hasTouch:true}),errors=[],results={};page.on('pageerror',e=>errors.push(e.message));
async function point(x,y){const b=await page.locator('#canvas').boundingBox(),s=await page.evaluate(()=>[Module.canvas.width,Module.canvas.height]);return[b.x+x*b.width/s[0],b.y+y*b.height/s[1]];}
async function click(x,y){await page.mouse.click(...await point(x,y));await page.waitForTimeout(100);}
async function api(...args){return page.evaluate(a=>Module._pvz_sandbox_command(...[...a,0,0,0,0].slice(0,4)),args);}
async function pd(i,f){return page.evaluate(([i,f])=>Module._pvz_sandbox_plant_data(i,f),[i,f]);}
async function zd(i,f){return page.evaluate(([i,f])=>Module._pvz_sandbox_zombie_data(i,f),[i,f]);}
async function shot(name){await page.screenshot({path:join(out,name+'.png')});}
async function wait(fn,arg,timeout=12000){await page.waitForFunction(fn,arg,{timeout});}
async function fresh(){await api(7);await api(5,1);await page.waitForTimeout(1600);}
try{
 await page.goto(process.env.PVZ_QA_URL||'http://127.0.0.1:8097/');await wait(()=>!document.getElementById('start').disabled,undefined,90000);
 await page.evaluate(async()=>{const FS=Module.FS;if(!FS.analyzePath('/saves/userdata').exists)FS.mkdir('/saves/userdata');
  const name=new TextEncoder().encode('GagQA'),users=new Uint8Array(16+name.length),u=new DataView(users.buffer);u.setUint32(0,14,true);u.setUint16(4,1,true);u.setUint16(6,name.length,true);users.set(name,8);u.setUint32(8+name.length,1,true);u.setUint32(12+name.length,1,true);FS.writeFile('/saves/userdata/users.dat',users);
  const profile=new Uint8Array(4096),p=new DataView(profile.buffer);p.setUint32(0,12,true);p.setUint32(4,37,true);FS.writeFile('/saves/userdata/user1.dat',profile);await new Promise((r,j)=>FS.syncfs(false,e=>e?j(e):r()));});
 await page.locator('#start').click();await page.waitForTimeout(12000);await click(400,560);await page.waitForTimeout(4500);await click(260,348);await wait(()=>Module.canvas.width===1024);
 await page.evaluate(()=>{window.memeAudioTrace=[];window.memeAudioRunning=true;const sample=()=>{if(!window.memeAudioRunning)return;const values=[0,1,2].map(f=>Module._pvz_meme_audio_data(f));if(values[1])window.memeAudioTrace.push(values);requestAnimationFrame(sample);};requestAnimationFrame(sample);});

 await fresh();for(let i=14;i<17;++i){await click(30+i%5*51,153+Math.floor(i/5)*78);await click(344+(i-14)*160,330);assert.equal(await pd(i-14,0),500+i);}await shot('three-new-native-cards');results.nativeCards=true;
 await fresh();await api(1,514,1,2);await api(2,23,4,2);const hp=await zd(0,4);await api(4,0);
 await wait(()=>Module._pvz_sandbox_plant_data(0,5)>=260);await api(4,1);assert.equal(await zd(0,4),hp);await shot('fume-holding-hiccup');
 const state=[await pd(0,4),await pd(0,5),await pd(0,7)];await page.waitForTimeout(300);assert.deepEqual([await pd(0,4),await pd(0,5),await pd(0,7)],state);
 await api(4,0);await wait(n=>Module._pvz_sandbox_zombie_data(0,4)<n,hp);await shot('native-fume-cloud-hiccup');
 await wait(()=>Module._pvz_sandbox_plant_data(0,4)===0);await api(4,1);assert.equal(hp-await zd(0,4),60);results.threeNativeFumes=true;results.pauseCharge=true;

 await fresh();await api(1,515,4,2);await api(2,4,8,2);await api(4,0);await wait(()=>Module._pvz_sandbox_plant_data(0,7)===0&&Module._pvz_projectile_data(5,0)>=0);await api(4,1);
 const peas=await page.evaluate(()=>Array.from({length:6},(_,i)=>[1,5,8].map(f=>Module._pvz_projectile_data(i,f))));
 assert.ok(peas.every(p=>p[1]===2&&p[2]===0));assert.ok(peas.filter(p=>p[0]<380).length===3&&peas.filter(p=>p[0]>380).length===3);await shot('split-pea-argues-in-both-directions');results.alternatingSixPeas=true;

 await fresh();await api(1,516,4,2);await api(2,4,4,2);await api(4,0);await wait(()=>Module._pvz_sandbox_zombie_data(0,11)>0);await api(4,1);assert.ok(await zd(0,11)>0);await shot('hotfoot-native-hop');results.hotfootHop=true;
 await api(4,0);await wait(()=>Module._pvz_sandbox_zombie_data(0,11)===0);await api(4,1);results.nativeLanding=true;

 await fresh();await api(2,24,6,2);await api(4,0);await wait(()=>Module._pvz_sandbox_zombie_data(0,9)<0);const reverseX=await zd(0,2);await page.waitForTimeout(350);await api(4,1);assert.ok(await zd(0,2)>reverseX);await shot('imp-reverse-walk');results.impActuallyReverses=true;

 await fresh();await api(1,3,3,2);await api(2,21,7,2);await api(5,2);await api(4,0);await wait(()=>Module._pvz_sandbox_zombie_data(0,1)!==2,undefined,15000);await api(4,1);assert.equal(await zd(0,1),1);assert.ok(await zd(0,7)>0);await shot('ladder-cuts-to-empty-lane');results.ladderChangesLane=true;
 await api(4,0);await page.waitForTimeout(800);await api(4,1);
 const audio=await page.evaluate(()=>window.memeAudioTrace);assert.ok(audio.length>10);assert.ok(audio.every(a=>a[1]===1&&a[2]===420));assert.deepEqual([...new Set(audio.map(a=>a[0]))].sort(),[0,1,2,3]);results.fourGagSounds=true;results.noVoicePileup=true;
 await page.evaluate(()=>window.memeAudioRunning=false);

 await fresh();for(const [id,col] of [[514,1],[515,3],[516,5]])await api(1,id,col,2);await page.setViewportSize({width:844,height:390});await page.waitForTimeout(300);await shot('mobile-native-layout');await page.setViewportSize({width:1100,height:750});await page.waitForTimeout(300);
 await api(15);assert.equal(await page.evaluate(()=>Module._pvz_meme_audio_data(1)),0);await page.waitForTimeout(800);await click(560,135);await page.waitForTimeout(15000);
 for(const base of [10,28,21,1,0,3]){const slot=base>=8?base+1:base;await click(47+slot%9*53,163+Math.floor(slot/9)*73);}await shot('adventure-native-chooser');await click(258,566);await click(305,367);
 await wait(()=>Module._pvz_adventure_power_data(-1,5)===1,undefined,25000);
 const bank=await page.evaluate(()=>Array.from({length:6},(_,i)=>[0,1,2].map(f=>Module._pvz_adventure_seed_data(i,f))));assert.deepEqual(bank.map(p=>p[0]),[514,515,516,503,500,501]);assert.deepEqual(bank.slice(0,3).map(p=>p[1]),[75,150,125]);assert.ok(bank.every(p=>p[2]===300));results.adventureBank=bank;await shot('adventure-gag-roster');
 assert.deepEqual(errors,[]);await writeFile(join(out,'report.json'),JSON.stringify({results,errors,audioCues:[...new Set(audio.map(a=>a[0]))]},null,2));console.log('Gag batch native QA passed',results);
}catch(e){await shot('failure');await writeFile(join(out,'failure.json'),JSON.stringify({error:String(e),results,errors,plants:await page.evaluate(()=>Array.from({length:4},(_,i)=>Array.from({length:11},(_,f)=>Module._pvz_sandbox_plant_data(i,f)))),zombies:await page.evaluate(()=>Array.from({length:3},(_,i)=>Array.from({length:13},(_,f)=>Module._pvz_sandbox_zombie_data(i,f)))),audio:await page.evaluate(()=>window.memeAudioTrace)},null,2));throw e;}
finally{await browser.close();}
