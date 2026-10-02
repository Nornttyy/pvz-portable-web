// Isolated Chromium only. Synthetic QA profile never touches a player's browser.
import assert from 'node:assert/strict';
import {mkdir,writeFile} from 'node:fs/promises';
import {join} from 'node:path';
const {chromium}=await import(process.env.PVZ_PLAYWRIGHT||'playwright-core');
const out=process.env.PVZ_QA_OUT||'/tmp/pvz-power-expansion-qa';await mkdir(out,{recursive:true});
const browser=await chromium.launch({executablePath:process.env.PVZ_CHROME||'/Applications/Google Chrome.app/Contents/MacOS/Google Chrome',headless:true,args:['--no-sandbox','--autoplay-policy=no-user-gesture-required']});
const page=await browser.newPage({viewport:{width:1100,height:750},hasTouch:true});const errors=[],logs=[];
const allNative=[];
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
try{
 await boot(8);
 let combat=[];
 if(!process.env.PVZ_QA_SKIP_SANDBOX){
 await click(719,27);await page.waitForFunction(()=>Module.canvas.width===1024,{},{timeout:20000});await shot('three-powers');
 for(let i=0;i<5;++i)await tap(220,571);await shot('last-power-page');
 // The last page includes Cob Cannon; a real pointer click selects and plants it.
 await tap(231,361);await tap(384,230);assert.equal((await data())[0][0],47);await api(7);
 await tap(220,535);await tap(220,571);await shot('recipe-results');
 // Recipe page two, row four is the instant cherry result (legacy eight first).
 await tap(220,386);await tap(384,230);assert.equal((await data())[0][0],302);await api(7);
 for(let type=100;type<120;type++)assert.equal(await api(1,type,1,1),-2);
 for(let type=200;type<212;type++)assert.equal(await api(2,type,8,1),-2);
 await tap(36,282);await tap(384,230);await tap(48,156);await tap(384,230);assert.equal((await data())[0][0],120);
 await api(7);const bases=[0,1,3,5,7,18,40,23];
 for(let p=0;p<3;p++)for(let i=0;i<8;i++){assert.equal(await api(1,bases[i],i,p*2),1);assert.equal(await api(1,180+p,i,p*2),120+p*8+i);}
 assert.equal((await data()).length,24);await shot('all-24-combinations');
 for(let row=0;row<5;row++)await api(2,23,8,row);
 await api(5,2);await api(4,0);await page.waitForTimeout(4500);await api(4,1);await shot('combat-combinations');
 combat=await data();assert.ok(combat.some(p=>p[0]===128&&p[5]>0));
 await page.evaluate(()=>window.dispatchEvent(new CustomEvent('pvz-sandbox-action',{detail:1})));await api(7);
 await page.evaluate(()=>window.dispatchEvent(new CustomEvent('pvz-sandbox-action',{detail:2})));assert.deepEqual((await data()).map(p=>p[0]).sort(),combat.map(p=>p[0]).sort());
 // All 48 definitions use the native placement path. Grave Buster still needs
 // an actual tombstone (not present on the sandbox's grass/pool maps).
 for(let base=0;base<48;++base){
  const water=[16,19,24,43].includes(base);await api(8,water?1:0);
  if(base===35)await api(12,0);
  const cases=[];
  for(let power=180;power<=182;++power){
   const col=1+(power-180)*2,row=2;
   if(base===35)assert.equal(await api(1,8,col,row),1);
   const placed=await api(1,base,col,row);
   if(base===11){assert.equal(placed,-4);cases.push({power,requiresTombstone:true});continue;}
   assert.equal(placed,1,`native placement ${base}/${power}`);
   const id=bases.includes(base)?120+(power-180)*8+bases.indexOf(base):300+(power-180)*48+base;
   assert.equal(await api(1,power,col,row),id,`infusion ${base}/${power}`);
   const plant=(await data()).find(p=>p[0]===id&&p[1]===col&&p[2]===row);
   assert.ok(plant,`live result ${id}`);cases.push({power,id,plant});
  }
  await api(4,0);await page.waitForTimeout(260);await api(4,1);
  if([10,30,32,35,39,43,47].includes(base))await shot('native-base-'+base);
  allNative.push({base,cases});
  if(base%12===11)console.log('Native catalogue checked:',base+1,'/ 48');
 }
 assert.equal(allNative.flatMap(p=>p.cases).filter(p=>p.id).length,141);
 await api(12,1);
 // Main plant, shell and support can each have their own power in one square.
 await api(8,1);for(const base of [16,10,30])assert.equal(await api(1,base,1,2),1);
 for(const [power,id] of [[180,310],[181,378],[182,412]])assert.equal(await api(1,power,1,2),id);
 await api(4,0);await page.waitForTimeout(600);await api(4,1);await shot('three-layer-powers');
 assert.equal((await data()).filter(p=>[310,378,412].includes(p[0])).length,3);
 // Native lobbed, mushroom, star and homing attacks run together with powers.
 await api(8,1);
 for(const [base,col,row,power] of [[10,3,0,180],[26,1,1,181],[29,1,4,182],[32,1,5,182],[34,2,5,181],[39,1,0,181],[43,1,2,182],[44,2,1,180]]){
  assert.equal(await api(1,base,col,row),1);assert.ok(await api(1,power,col,row)>0);
 }
 for(let row=0;row<6;++row)await api(2,row===2||row===3?10:23,7,row);
 await api(5,2);await api(4,0);await page.waitForTimeout(7000);await api(4,1);await shot('native-mixed-combat');
 assert.ok((await data()).filter(p=>p[5]>0).length>=5);
 await api(15);await page.waitForTimeout(1000);
 console.log('Sandbox: 141 live combinations, tombstone restrictions, layers, combat and save passed');
 }
 if(!process.env.PVZ_QA_SKIP_ADVENTURE){
 const files=await page.evaluate(()=>{function walk(root){return Module.FS.readdir(root).filter(n=>n!=='.'&&n!=='..').flatMap(n=>{const p=root+'/'+n;return Module.FS.isDir(Module.FS.stat(p).mode)?walk(p):[p];});}return walk('/saves');});
 await writeFile(join(out,'save-files.json'),JSON.stringify(files,null,2));
 console.log('Synthetic profile prepared');
 const profile=files.find(f=>/\/user\d+\.dat$/.test(f));assert.ok(profile);
 await shot('adventure-menu');await click(560,135);await page.waitForTimeout(15000);await shot('adventure-entry');
 for(const [x,y] of [[48,172],[104,172],[210,172],[310,172],[365,172],[154,172]])await click(x,y);
 await click(232,566);await page.waitForFunction(()=>Module._pvz_adventure_power_data(-1,5)===1,{},{timeout:20000});await shot('adventure-board');
 async function ad(index,field){return page.evaluate(([i,f])=>Module._pvz_adventure_power_data(i,f),[index,field]);}
 async function collectUntil(amount){
  const deadline=Date.now()+65000;
  while(await ad(-1,2)<amount){
   assert.ok(Date.now()<deadline,'sun collection timed out');
   const pos=await page.evaluate(()=>[Module._pvz_adventure_power_data(0,10),Module._pvz_adventure_power_data(0,11)]);
   if(pos[0]>=0)await click(...pos);else await page.waitForTimeout(350);
  }
 }
 // Select/plant through native input; earn and collect real suns, no economy cheats.
 await click(170,45);await click(80,330);assert.equal(await ad(0,0),1);
 await tap(748,61);await shot('adventure-choices');await tap(700,179); // Third power is locked at 1-8.
 assert.equal(await ad(-1,0),180);await page.keyboard.press('Escape');
 await collectUntil(175);await click(110,45);await click(160,330);assert.equal(await ad(1,0),0);
 const before=await ad(-1,2);await tap(748,61);await tap(700,102);await tap(160,330);
 assert.equal(await ad(1,0),120);assert.equal(await ad(-1,2),before-75);assert.ok(await ad(-1,1)>0);
 await shot('adventure-red-heat');
 await click(748,14);const paused=await ad(-1,1);await page.waitForTimeout(600);assert.equal(await ad(-1,1),paused);await shot('adventure-paused');
 // The native options menu is 423 x 498, centered in an 800 x 600 board.
 await click(400,401);await page.waitForTimeout(450);await shot('leave-confirm');
 await click(305,394);await page.waitForFunction(()=>Module._pvz_adventure_power_data(-1,5)===-1,{},{timeout:10000});await shot('saved-menu');
 const saved=await page.evaluate(async()=>{
  const FS=Module.FS;await new Promise((resolve,reject)=>FS.syncfs(false,e=>e?reject(e):resolve()));
  for(const name of FS.readdir('/saves/userdata')){
   const path='/saves/userdata/'+name;if(FS.isDir(FS.stat(path).mode))continue;
   const bytes=FS.readFile(path);if(new TextDecoder().decode(bytes.slice(0,10)).replace(/\0/g,'')!=='PVZP_SAVE4')continue;
   const v=new DataView(bytes.buffer,bytes.byteOffset,bytes.byteLength);
   for(let offset=24;offset+8<=bytes.length;){
    const id=v.getUint32(offset,true),size=v.getUint32(offset+4,true);offset+=8;
    if(id===21)return {path,values:Array.from({length:(size-12)/4},(_,i)=>v.getInt32(offset+12+i*4,true))};
    offset+=size;
   }
  }
  return null;
 });
 assert.ok(saved,'native save must contain the optional powers chunk');
 assert.equal(saved.values[0],180);assert.equal(saved.values[1],paused);assert.equal(saved.values[2],1);assert.equal(saved.values[4],120);
 console.log('Adventure: native placement, unlock, sun cost, pause and save chunk passed');
 // Reload the whole browser page: verify IDB persistence, not just in-memory state.
 await boot();await click(560,135);
 await page.waitForFunction(()=>Module._pvz_adventure_power_data(-1,5)===1,{},{timeout:20000});
 await page.waitForTimeout(250);await shot('continue-dialog');await click(280,371);
 await page.waitForFunction(()=>Module._pvz_adventure_power_data(-1,6)===0,{},{timeout:10000});
 assert.equal(await ad(1,0),120);assert.equal(await ad(-1,4),1);assert.equal(await ad(-1,3),8);
 assert.ok(await ad(-1,1)>0&&await ad(-1,1)<=paused);await shot('adventure-resumed');
 await page.waitForTimeout(3300);assert.equal(await ad(-1,1),0,'power is ready after three game seconds');
 console.log('Adventure: full reload, restored power and three-second cooldown passed');
 await collectUntil(100);
 // Collect overlapping suns first; native coin priority intentionally keeps the
 // power in hand rather than spending it when the player meant to collect sun.
 for(let i=0;i<12;++i){const pos=await page.evaluate(()=>[Module._pvz_adventure_power_data(0,10),Module._pvz_adventure_power_data(0,11)]);if(pos[0]<0)break;await click(...pos);}
 await page.waitForTimeout(800);
 await page.setViewportSize({width:844,height:390});await page.waitForTimeout(300);await shot('mobile-landscape');
 const balance=await ad(-1,2);await tap(748,61);await tap(700,141);await tap(160,330);
 assert.equal(await ad(-1,2),balance,'already infused plants cannot be overwritten or charged again');
 assert.equal(await ad(-1,1),0);await tap(110,370);
 assert.equal(await ad(0,0),129);assert.equal(await ad(-1,2),balance-100);assert.ok(await ad(-1,1)>0);
 await shot('mobile-grind-sunflower');
 await page.setViewportSize({width:390,height:844});await page.waitForTimeout(300);
 const canvas=await page.locator('#canvas').boundingBox();assert.ok(canvas.x>=-1&&canvas.x+canvas.width<=391&&canvas.y>=-1&&canvas.y+canvas.height<=845);
 await tap(748,61);await shot('mobile-portrait-choices');await page.keyboard.press('Escape');
 // Prepare the power while a native instant seed is already in hand.
 await page.setViewportSize({width:1100,height:750});await collectUntil(225);
 for(let i=0;i<16;++i){const pos=await page.evaluate(()=>[Module._pvz_adventure_power_data(0,10),Module._pvz_adventure_power_data(0,11)]);if(pos[0]<0)break;await click(...pos);}
 await page.waitForTimeout(800);const instantBalance=await ad(-1,2);
 await click(410,45);await tap(748,61);await tap(700,102);
 await click(160,330);assert.equal(await ad(-1,2),instantBalance,'invalid occupied square must not charge either cost');
 await click(240,430);
 const instant=await page.evaluate(()=>Array.from({length:50},(_,i)=>Module._pvz_adventure_power_data(i,0)));
 assert.ok(instant.includes(302),'cherry bomb receives the power during native planting');
 assert.equal(await ad(-1,2),instantBalance-225);assert.ok(await ad(-1,1)>0);
 await shot('instant-pre-infused');await page.waitForTimeout(1400);
 assert.equal(await ad(-1,4),2,'consumed instant must not leave a phantom power state');
 assert.deepEqual(errors,[]);assert.ok(!logs.some(s=>/Aborted\(|unreachable|memory access out of bounds/.test(s)));
 await writeFile(join(out,'report.json'),JSON.stringify({allNative,combat,files,saved,paused,canvas,errors,logs},null,2));
 }else{
  assert.deepEqual(errors,[]);assert.ok(!logs.some(s=>/Aborted\(|unreachable|memory access out of bounds/.test(s)));
  await writeFile(join(out,'report.json'),JSON.stringify({allNative,combat,errors,logs},null,2));
 }
 console.log('Power expansion QA passed:',out);
}catch(e){await shot('failure');console.log('ENGINE LOG',await page.evaluate(()=>window.pvzEngineLog));throw e;}
finally{await browser.close();}
