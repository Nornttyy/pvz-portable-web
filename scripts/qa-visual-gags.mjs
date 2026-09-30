// Actual native engine, disposable profile; no production save files modified.
import assert from 'node:assert/strict';
import {mkdir,writeFile} from 'node:fs/promises';
import {join} from 'node:path';
const {chromium}=await import(process.env.PVZ_PLAYWRIGHT||'playwright-core');
const out=process.env.PVZ_QA_OUT||'/tmp/pvz-visual-gags';await mkdir(out,{recursive:true});
const browser=await chromium.launch({executablePath:'/Applications/Google Chrome.app/Contents/MacOS/Google Chrome',headless:true,args:['--no-sandbox','--autoplay-policy=no-user-gesture-required']});
const page=await browser.newPage({viewport:{width:1100,height:750}}),errors=[],results={};page.on('pageerror',e=>errors.push(e.message));
async function click(x,y){const b=await page.locator('#canvas').boundingBox(),s=await page.evaluate(()=>[Module.canvas.width,Module.canvas.height]);await page.mouse.click(b.x+x*b.width/s[0],b.y+y*b.height/s[1]);await page.waitForTimeout(120);}
const api=(...a)=>page.evaluate(a=>Module._pvz_sandbox_command(...[...a,0,0,0,0].slice(0,4)),a);
const pd=(i,f)=>page.evaluate(([i,f])=>Module._pvz_sandbox_plant_data(i,f),[i,f]);
const zd=(i,f)=>page.evaluate(([i,f])=>Module._pvz_sandbox_zombie_data(i,f),[i,f]);
const wait=(f,a,t=12000)=>page.waitForFunction(f,a,{timeout:t});
const shot=n=>page.screenshot({path:join(out,n+'.png')});
async function fresh(){await api(7);await api(5,1);await page.waitForTimeout(1000);}
try{
 await page.goto(process.env.PVZ_QA_URL||'http://127.0.0.1:8097/');await wait(()=>!document.getElementById('start').disabled,undefined,90000);
 await page.evaluate(async()=>{const FS=Module.FS;if(!FS.analyzePath('/saves/userdata').exists)FS.mkdir('/saves/userdata');const name=new TextEncoder().encode('VisualQA'),users=new Uint8Array(16+name.length),v=new DataView(users.buffer);v.setUint32(0,14,true);v.setUint16(4,1,true);v.setUint16(6,name.length,true);users.set(name,8);v.setUint32(8+name.length,1,true);v.setUint32(12+name.length,1,true);FS.writeFile('/saves/userdata/users.dat',users);const profile=new Uint8Array(4096),p=new DataView(profile.buffer);p.setUint32(0,12,true);p.setUint32(4,37,true);FS.writeFile('/saves/userdata/user1.dat',profile);await new Promise((r,j)=>FS.syncfs(false,e=>e?j(e):r()));});
 await page.locator('#start').click();await page.waitForTimeout(12000);await click(400,560);await page.waitForTimeout(4500);
 results.embedded=await page.evaluate(()=>['/addons/art/squash-headband.png','/addons/art/bucket-glove.png','/addons/audio/rage-scream.wav'].map(p=>Module.FS.stat(p).size));assert.ok(results.embedded.every(n=>n>100));
 await click(370,455);await page.waitForTimeout(1000);await shot('almanac-index');await click(208,366);await page.waitForTimeout(500);await click(416,123);await shot('almanac-extra-card-inside-grid');await click(720,580);await page.waitForTimeout(600);await click(260,348);await wait(()=>Module.canvas.width===1024);

 await fresh();await api(1,517,1,2);await api(2,4,8,2);await api(4,0);
 await wait(()=>Module._pvz_projectile_data(0,0)===290);await page.waitForTimeout(220);await api(4,1);await shot('threepeater-whole-head-turn');await api(4,0);
 await wait(()=>{for(let i=0;i<32;i++){if(Module._pvz_projectile_data(i,0)<0)break;if(Module._pvz_projectile_data(i,0)===290&&Module._pvz_projectile_data(i,3)<0)return true;}return false;});await api(4,1);await shot('threepeater-returning-peas');results.returnPeas=true;

 await fresh();await api(1,518,3,2);await shot('squash-new-rig-and-card');await api(2,23,4,2);const hp=await zd(0,4);await api(4,0);
 await wait(()=>Module._pvz_sandbox_plant_data(0,7)===2);await api(4,1);assert.equal(await zd(0,4),hp-600);await shot('squash-first-return');results.squashFirstReturn=true;
 await api(4,0);await wait(()=>Module._pvz_sandbox_plant_data(0,7)===1);await api(4,1);assert.equal(await zd(0,4),hp-1200);await shot('squash-second-return');results.squashSecondReturn=true;
 await api(4,0);await wait(()=>Module._pvz_sandbox_plant_data(0,0)<0);await api(4,1);assert.equal(await zd(0,4),hp-1800);results.threeStomps=true;

 // Native walkers have randomized speeds; a non-attacking vanilla wall-nut
 // holds the front walker in reach instead of relying on identical walk rolls.
 await fresh();await api(1,3,4,2);await api(2,0,5,2);await api(2,4,6,2);const allyHp=await zd(0,4);await shot('bucket-glove-idle');await api(4,0);
 await wait(()=>Module._pvz_sandbox_zombie_data(1,8)>0,undefined,24000);await page.waitForTimeout(170);await api(4,1);await shot('bucket-swings-connected-arm');await api(4,0);await wait(h=>Module._pvz_sandbox_zombie_data(0,4)<h,allyHp,24000);await api(4,1);assert.equal(await zd(0,4),allyHp-40);results.bucketFriendlyPunch=true;

 await fresh();await api(1,3,6,2);await api(2,3,7,2);await api(4,0);await wait(()=>Module._pvz_sandbox_zombie_data(0,8)>700,undefined,16000);await api(4,1);const landedPhase=await zd(0,6);await shot('pole-vault-landed');await api(4,0);await wait(()=>{const n=Module._pvz_sandbox_zombie_data(0,8);return n>10&&n<40;},undefined,13000);await api(4,1);await shot('pole-reaches-for-backup');await api(4,0);await wait(p=>Module._pvz_sandbox_zombie_data(0,6)!==p,landedPhase);await api(4,1);await shot('pole-rearmed');results.poleReloads=true;

 await fresh();await api(1,517,1,2);await api(1,518,3,2);await api(2,4,7,2);await page.setViewportSize({width:844,height:390});await page.waitForTimeout(300);await shot('mobile-new-native-art');await page.setViewportSize({width:1100,height:750});await page.waitForTimeout(300);
 await api(15);await page.waitForTimeout(700);await click(560,135);await page.waitForTimeout(15000);
 for(const base of [18,17,52,1,0,3]){const slot=base===52?8:base>=8?base+1:base;await click(47+slot%9*53,163+Math.floor(slot/9)*73);}await shot('adventure-new-cards');await click(258,566);await click(305,367);await wait(()=>Module._pvz_adventure_power_data(-1,5)===1,undefined,25000);
 const bank=await page.evaluate(()=>Array.from({length:6},(_,i)=>Module._pvz_adventure_seed_data(i,0)));assert.deepEqual(bank,[517,518,504,503,500,501]);results.adventureBank=bank;await shot('adventure-native-bank');
 assert.deepEqual(errors,[]);await writeFile(join(out,'report.json'),JSON.stringify({results,errors},null,2));console.log(results);
}catch(e){await shot('failure');await writeFile(join(out,'failure.json'),JSON.stringify({error:String(e),results,errors,plants:await page.evaluate(()=>Array.from({length:4},(_,i)=>Array.from({length:11},(_,f)=>Module._pvz_sandbox_plant_data(i,f)))),zombies:await page.evaluate(()=>Array.from({length:3},(_,i)=>Array.from({length:13},(_,f)=>Module._pvz_sandbox_zombie_data(i,f)))),engine:await page.evaluate(()=>window.pvzEngineLog)},null,2));throw e;}finally{await browser.close();}
