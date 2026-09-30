// Isolated Chromium only. Synthetic QA profile never touches a player's browser.
import assert from 'node:assert/strict';
import {mkdir,writeFile} from 'node:fs/promises';
import {join} from 'node:path';
const {chromium}=await import(process.env.PVZ_PLAYWRIGHT||'playwright-core');
const out=process.env.PVZ_QA_OUT||'/tmp/pvz-adventure-replacements-qa';await mkdir(out,{recursive:true});
const browser=await chromium.launch({executablePath:process.env.PVZ_CHROME||'/Applications/Google Chrome.app/Contents/MacOS/Google Chrome',headless:true,args:['--no-sandbox','--autoplay-policy=no-user-gesture-required']});
const page=await browser.newPage({viewport:{width:1100,height:750},hasTouch:true});const errors=[],logs=[];
page.on('pageerror',e=>errors.push(e.message));page.on('console',m=>logs.push(m.text()));
async function point(x,y){const b=await page.locator('#canvas').boundingBox();const s=await page.evaluate(()=>[Module.canvas.width,Module.canvas.height]);return [b.x+x*b.width/s[0],b.y+y*b.height/s[1]];}
async function click(x,y){await page.mouse.click(...await point(x,y));await page.waitForTimeout(140);}
async function tap(x,y){await page.touchscreen.tap(...await point(x,y));await page.waitForTimeout(140);}
async function shot(name){await page.screenshot({path:join(out,name+'.png')});}
async function api(...args){return page.evaluate(v=>Module._pvz_sandbox_command(...[...v,0,0,0,0].slice(0,4)),args);}
async function data(){return page.evaluate(()=>{const list=[];for(let i=0;i<180;i++){const type=Module._pvz_sandbox_plant_data(i,0);if(type<0)break;list.push(Array.from({length:7},(_,f)=>Module._pvz_sandbox_plant_data(i,f)));}return list;});}
async function boot(seedLevel=0){await page.goto(process.env.PVZ_QA_URL||'http://127.0.0.1:8097/');await page.waitForFunction(()=>!document.getElementById('start').disabled,{},{timeout:90000});
 if(seedLevel)await page.evaluate(async level=>{
  const FS=Module.FS;if(!FS.analyzePath('/saves/userdata').exists)FS.mkdir('/saves/userdata');
  const name=new TextEncoder().encode('PowerQA'),users=new Uint8Array(16+name.length),u=new DataView(users.buffer);
  u.setUint32(0,14,true);u.setUint16(4,1,true);u.setUint16(6,name.length,true);users.set(name,8);u.setUint32(8+name.length,1,true);u.setUint32(12+name.length,1,true);
  FS.writeFile('/saves/userdata/users.dat',users);const profile=new Uint8Array(4096),p=new DataView(profile.buffer);p.setUint32(0,12,true);p.setUint32(4,level,true);FS.writeFile('/saves/userdata/user1.dat',profile);
  await new Promise((resolve,reject)=>FS.syncfs(false,e=>e?reject(e):resolve()));
 },seedLevel);
 await page.locator('#start').click();await page.waitForTimeout(12000);await click(400,560);await page.waitForTimeout(4500);}


const level=Number(process.env.PVZ_QA_LEVEL||11),results={level};
async function ad(i,f){return page.evaluate(([i,f])=>Module._pvz_adventure_power_data(i,f),[i,f]);}
async function seed(i,f){return page.evaluate(([i,f])=>Module._pvz_adventure_seed_data(i,f),[i,f]);}
async function card(i){await tap(await seed(i,5)+25,await seed(i,6)+35);}
async function collect(){const pos=[await ad(0,10),await ad(0,11)];if(pos[0]>=0)await click(...pos);}
async function money(n,timeout=55000){const until=Date.now()+timeout;while(await ad(-1,2)<n){assert.ok(Date.now()<until,'production/sun collection deadline');await collect();await page.waitForTimeout(150);}}
async function play(){await click(560,135);await page.waitForFunction(()=>Module._pvz_adventure_power_data(-1,5)===1,{},{timeout:40000});}
async function leave(){await click(748,14);await click(400,401);await page.waitForTimeout(450);await click(305,394);await page.waitForFunction(()=>Module._pvz_adventure_power_data(-1,5)===-1,{},{timeout:10000});}
async function resume(){await boot();await play();await page.waitForTimeout(250);await click(280,371);await page.waitForFunction(()=>Module._pvz_adventure_power_data(-1,6)===0,{},{timeout:10000});}
try{
 await boot(level);
 if(level===1){
  await play();await page.waitForFunction(()=>Module._pvz_adventure_power_data(-1,8)===1,{},{timeout:15000});await shot('first-level-tutorial');
  assert.equal(await seed(0,0),500);assert.equal(await seed(0,1),100);assert.equal(await seed(0,2),300);
  await card(0);await click(80,330);assert.equal(await ad(0,0),500);assert.equal(await ad(-1,2),50);
  await money(100);await card(0);await click(240,330);assert.equal(await ad(1,0),500);assert.equal(await ad(-1,8),4);
  await shot('first-level-native-slots');const until=Date.now()+160000;
  while(await ad(0,12)<0){assert.ok(Date.now()<until,'first-level victory deadline');await collect();for(let i=0;i<2;++i)if(await ad(i,5)>=100&&await ad(i,4)===0){await click(i===0?80:240,330);results.manualBurst=true;}await page.waitForTimeout(160);}
  await click(await ad(0,12),await ad(0,13));await page.waitForTimeout(7000);await shot('replacement-award');results.firstVictory=true;
 }else if(level===2){
  await play();await page.waitForFunction(()=>Module._pvz_adventure_power_data(-1,8)===5,{},{timeout:15000});
  assert.equal(await seed(1,0),503);assert.equal(await seed(1,1),50);await shot('sunflower-replacement-tutorial');
  for(const [x,y] of [[80,230],[80,330],[80,430]]){await money(50);await page.waitForFunction(()=>Module._pvz_adventure_seed_data(1,3)===0);await card(1);await click(x,y);}
  assert.equal(await ad(-1,8),8);for(let i=0;i<3;++i)assert.equal(await ad(i,0),503);await shot('second-tutorial-complete');results.tutorialComplete=true;
 }else{
  await click(560,135);await page.waitForTimeout(15000);
  // The first night begins with Dave's native introduction before the chooser.
  if(level===11){for(let i=0;i<10;++i){await click(425,80);await page.waitForTimeout(700);}await page.waitForTimeout(3000);}
  await shot('native-seed-chooser');
  const choices=level===11?[8,1,0,3,5,2]:[0,1,3,5,6,2];
  for(const id of choices)await click(47+(id%8)*53,163+Math.floor(id/8)*73);
  await click(232,566);await page.waitForFunction(()=>Module._pvz_adventure_power_data(-1,5)===1,{},{timeout:20000});
  assert.equal(level,11);assert.deepEqual(await Promise.all([0,1,2,3,4,5].map(i=>seed(i,0))),[502,503,500,501,5,2]);
  for(let i=0;i<4;++i)assert.equal(await seed(i,2),300);
  await card(1);await click(80,330);assert.equal(await ad(0,0),503);assert.equal(await ad(-1,2),0);
  assert.ok(await seed(1,3)>0);await card(0);await click(480,330);assert.equal(await ad(1,0),502);assert.equal(await ad(1,9),0);
  assert.equal(await ad(-1,2),0);assert.ok(await seed(0,3)>0);await shot('night-free-shroom-and-producer');
  await click(748,14);const clocks=[await seed(0,3),await seed(1,3)];await page.waitForTimeout(400);assert.deepEqual([await seed(0,3),await seed(1,3)],clocks);
  await click(400,450);await page.waitForFunction(()=>Module._pvz_adventure_power_data(-1,6)===0,{},{timeout:4000});
  await page.waitForFunction(()=>Module._pvz_adventure_power_data(0,10)>=0,{},{timeout:18000});await collect();await page.waitForTimeout(600);
  assert.ok(await ad(-1,2)>=25,'new sunflower produces real collectible sun in a night level');results.production=true;
  await tap(748,61);assert.equal(await ad(-1,7),0);await shot('no-extra-drawer');
  const hp=[await ad(0,3),await ad(1,3)];await leave();await resume();
  assert.equal(await ad(0,0),503);assert.equal(await ad(1,0),502);assert.deepEqual([await ad(0,3),await ad(1,3)],hp);assert.equal(await ad(-1,3),11);await shot('replacements-resumed');
  await page.setViewportSize({width:844,height:390});await page.waitForTimeout(400);await card(0);await tap(160,230);assert.equal(await ad(2,0),502);await shot('phone-native-seed-bank');
  await page.setViewportSize({width:1100,height:750});await page.waitForTimeout(400);await leave();
  await page.evaluate(async()=>{
   const FS=Module.FS,path='/saves/userdata/game1_0.v4',bytes=FS.readFile(path),v=new DataView(bytes.buffer,bytes.byteOffset,bytes.byteLength),parts=[bytes.slice(0,24)];
   for(let off=24;off<bytes.length;){const size=v.getUint32(off+4,true),next=off+8+size;if(v.getUint32(off,true)!==21)parts.push(bytes.slice(off,next));off=next;}
   const out=new Uint8Array(parts.reduce((n,b)=>n+b.length,0));let off=0;for(const b of parts){out.set(b,off);off+=b.length;}
   const h=new DataView(out.buffer);h.setUint32(16,out.length-24,true);let crc=0xffffffff;for(const byte of out.subarray(24)){crc^=byte;for(let bit=0;bit<8;++bit)crc=(crc>>>1)^((crc&1)?0xedb88320:0);}h.setUint32(20,(crc^0xffffffff)>>>0,true);
   FS.writeFile(path,out);await new Promise((resolve,reject)=>FS.syncfs(false,e=>e?reject(e):resolve()));
  });
  await resume();assert.equal(await ad(0,0),503);assert.equal(await ad(1,0),502);assert.equal(await ad(2,0),502);assert.equal(await ad(-1,3),11);assert.deepEqual([await ad(0,3),await ad(1,3)],hp);await shot('pre-mod-save-migrated');results.nativeSaveMigration=true;
 }
 assert.deepEqual(errors,[]);assert.ok(!logs.some(s=>/Aborted\(|unreachable|memory access out of bounds/.test(s)));
 await writeFile(join(out,'report.json'),JSON.stringify({results,errors,logs},null,2));console.log('Adventure replacement QA passed:',out,results);
}catch(e){await shot('failure');console.log('ADVENTURE',await ad(-1,3),await ad(-1,8),await ad(-1,9));console.log('ENGINE',await page.evaluate(()=>window.pvzEngineLog));throw e;}
finally{await browser.close();}
