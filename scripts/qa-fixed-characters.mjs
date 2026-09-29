// Isolated Chromium only. Synthetic QA profile never touches a player's browser.
import assert from 'node:assert/strict';
import {mkdir,writeFile} from 'node:fs/promises';
import {join} from 'node:path';
const {chromium}=await import(process.env.PVZ_PLAYWRIGHT||'playwright-core');
const out=process.env.PVZ_QA_OUT||'/tmp/pvz-fixed-characters-qa';await mkdir(out,{recursive:true});
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

const results={};
async function zombies(){return page.evaluate(()=>{const out=[];for(let i=0;i<160;i++){const type=Module._pvz_sandbox_zombie_data(i,0);if(type<0)break;out.push(Array.from({length:6},(_,f)=>Module._pvz_sandbox_zombie_data(i,f)));}return out;});}
async function waitPlant(field,value){await page.waitForFunction(([f,v])=>Module._pvz_sandbox_plant_data(0,f)===v,[field,value],{timeout:20000});}
try{
 await boot(8);
 if(!process.env.PVZ_QA_SKIP_SANDBOX){
 await click(260,348);await page.waitForFunction(()=>Module.canvas.width===1024,{},{timeout:20000});await shot('fixed-cards');
 for(const id of [120,143,180,181,182,302,443])assert.equal(await api(1,id,1,2),-2);
 await api(21,1);assert.equal((await api(0))&64,0);
 await click(36,155);await click(460,330);assert.equal((await data())[0][0],500);
 for(const [id,col] of [[501,3],[502,4],[503,5]])assert.equal(await api(1,id,col,2),1);
 await page.evaluate(()=>window.dispatchEvent(new CustomEvent('pvz-sandbox-action',{detail:1})));await api(7);
 await page.evaluate(()=>window.dispatchEvent(new CustomEvent('pvz-sandbox-action',{detail:2})));
 assert.deepEqual((await data()).map(p=>p[0]),[500,501,502,503]);await shot('four-characters');

 await api(7);await api(1,500,2,2);await api(2,23,8,2);await api(4,0);
 await page.waitForFunction(()=>Module._pvz_sandbox_plant_data(0,5)>=400,{},{timeout:15000});
 await click(710,61);await click(460,330);assert.ok((await data())[0][5]<200);await shot('manual-vent');
 await waitPlant(4,2);await api(4,1);assert.equal((await data())[0][3],180);
 const frozen=(await data())[0];await page.waitForTimeout(400);assert.deepEqual((await data())[0],frozen);results.overheat=frozen;await shot('overheated');

 await api(7);await api(1,501,3,2);await api(2,0,4,2);await api(4,0);
 await click(710,61);await click(540,300);assert.equal((await zombies())[0][1],1);
 await page.waitForTimeout(700);results.redirect=await zombies();await shot('redirect-up');
 await api(2,0,4,2);await click(540,350);assert.equal((await zombies())[1][1],2,'cooldown prevents repeated lane changes');
 await api(5,4);await waitPlant(6,0);await click(540,350);assert.equal((await zombies())[1][1],3);await shot('redirect-down');
 await api(8,1);await api(1,501,3,1);await api(2,0,4,1);await api(4,0);
 await api(23,1,3,1);assert.equal((await zombies())[0][1],1,'cannot send a land walker into pool');

 await api(8,0);await api(1,502,3,2);await api(2,0,5,1);await api(2,23,4,3);await api(4,0);
 await page.waitForFunction(()=>Module._pvz_sandbox_zombie_data(0,1)===2,{},{timeout:10000});
 assert.equal((await zombies())[1][1],3);results.lure=await zombies();await shot('lure-neighbor');

 await api(7);await api(1,503,4,2);await api(2,0,5,2);await api(5,2);await api(4,0);
 await waitPlant(4,1);await shot('fake-death');
 await waitPlant(4,2);await shot('ambush-rise');await page.waitForTimeout(1700);
 assert.equal((await data())[0][3],300,'walker did not bite the hidden flower');
 const victims=await zombies();assert.ok(!victims.length||victims[0][4]<200,'backward volley actually damages the passing zombie');
 results.ambush=victims;await shot('ambush-hit');
 await api(4,1);await page.setViewportSize({width:844,height:390});await page.waitForTimeout(350);await shot('mobile-fixed-cards');
 await page.setViewportSize({width:1100,height:750});await api(15);await page.waitForTimeout(900);
 console.log('Sandbox fixed-character mechanics, pointer actions, safety, save and mobile passed');
 }
 if(!process.env.PVZ_QA_SKIP_ADVENTURE){
 await click(560,135);await page.waitForTimeout(15000);
 for(const [x,y] of [[48,172],[104,172],[210,172],[310,172],[365,172],[154,172]])await click(x,y);
 await click(232,566);await page.waitForFunction(()=>Module._pvz_adventure_power_data(-1,5)===1,{},{timeout:20000});
 async function ad(i,f){return page.evaluate(([i,f])=>Module._pvz_adventure_power_data(i,f),[i,f]);}
 await tap(748,61);await shot('adventure-seed-drawer');await page.keyboard.press('Escape');
 await click(170,45);await click(80,330);
 const deadline=Date.now()+85000;
 while(await ad(-1,2)<150){assert.ok(Date.now()<deadline,'sun collection deadline');const pos=await page.evaluate(()=>[Module._pvz_adventure_power_data(0,10),Module._pvz_adventure_power_data(0,11)]);if(pos[0]>=0)await click(...pos);else await page.waitForTimeout(350);}
 const money=await ad(-1,2);
 await tap(748,61);await tap(577,121);await tap(80,330);assert.equal(await ad(-1,2),money);
 await tap(160,330);assert.equal(await ad(1,0),500);assert.equal(await ad(-1,2),money-150);assert.ok(await ad(-1,1)>0);await shot('adventure-fixed-placement');
 await click(748,14);const paused=await ad(-1,1);await page.waitForTimeout(500);assert.equal(await ad(-1,1),paused);
 await click(400,401);await page.waitForTimeout(450);await click(305,394);await page.waitForFunction(()=>Module._pvz_adventure_power_data(-1,5)===-1,{},{timeout:10000});
 await boot();await click(560,135);await page.waitForFunction(()=>Module._pvz_adventure_power_data(-1,5)===1,{},{timeout:20000});await page.waitForTimeout(250);await click(280,371);
 await page.waitForFunction(()=>Module._pvz_adventure_power_data(-1,6)===0,{},{timeout:10000});
 assert.equal(await ad(1,0),500);assert.equal(await ad(-1,3),8);await page.waitForTimeout(3300);assert.equal(await ad(-1,1),0);
 await shot('adventure-fixed-resumed');
 await page.setViewportSize({width:390,height:844});await page.waitForTimeout(350);await tap(748,61);assert.equal(await ad(-1,7),1);await shot('portrait-seed-drawer');
 const canvas=await page.locator('#canvas').boundingBox();assert.ok(canvas.x>=-1&&canvas.x+canvas.width<=391);results.canvas=canvas;
 console.log('Adventure direct cards, sun cost, pause, full reload and three-second cooldown passed');
 // Exercise migration with an isolated synthetic V4 record, never a user save.
 await page.keyboard.press('Escape');await page.setViewportSize({width:1100,height:750});await page.waitForTimeout(350);
 await click(748,14);const oldHP=await ad(1,3);await click(400,401);await page.waitForTimeout(450);await click(305,394);
 await page.waitForFunction(()=>Module._pvz_adventure_power_data(-1,5)===-1,{},{timeout:10000});
 await page.evaluate(async()=>{
  const FS=Module.FS,path='/saves/userdata/game1_0.v4',bytes=FS.readFile(path),v=new DataView(bytes.buffer,bytes.byteOffset,bytes.byteLength);let changed=false;
  for(let off=24;off<bytes.length;){const type=v.getUint32(off,true),size=v.getUint32(off+4,true),body=off+8;
   if(type===21){const fields=body+12;if(v.getInt32(fields+8,true)!==1)throw Error('Expected exactly one QA character');v.setInt32(fields,180,true);v.setInt32(fields+16,120,true);changed=true;}
   off=body+size;
  }
  if(!changed)throw Error('Missing QA optional chunk');let crc=0xffffffff;
  for(const byte of bytes.subarray(24)){crc^=byte;for(let bit=0;bit<8;++bit)crc=(crc>>>1)^((crc&1)?0xedb88320:0);}
  v.setUint32(20,(crc^0xffffffff)>>>0,true);FS.writeFile(path,bytes);await new Promise((resolve,reject)=>FS.syncfs(false,e=>e?reject(e):resolve()));
 });
 await boot();await click(560,135);await page.waitForFunction(()=>Module._pvz_adventure_power_data(-1,5)===1,{},{timeout:20000});await page.waitForTimeout(250);await click(280,371);
 assert.equal(await ad(1,0),0);assert.equal(await ad(1,3),oldHP);assert.equal(await ad(-1,3),8);assert.equal(await ad(-1,4),0);await shot('legacy-save-migrated');results.legacyMigration=true;
 console.log('Legacy fusion save migrates to native pea without resetting HP or progress');
 }
 assert.deepEqual(errors,[]);assert.ok(!logs.some(s=>/Aborted\(|unreachable|memory access out of bounds/.test(s)));
 await writeFile(join(out,'report.json'),JSON.stringify({results,errors,logs},null,2));console.log('Fixed characters QA passed:',out);
}catch(e){await shot('failure');console.log('DIAGNOSTICS',await data(),await zombies());console.log('ENGINE LOG',await page.evaluate(()=>window.pvzEngineLog));throw e;}
finally{await browser.close();}
