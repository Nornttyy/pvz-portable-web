// Real WASM smoke test. Synthetic profiles live only in isolated browser contexts.
import assert from 'node:assert/strict';
import {mkdir,writeFile} from 'node:fs/promises';
import {join} from 'node:path';
const {chromium}=await import(process.env.PVZ_PLAYWRIGHT||'playwright-core');
const out=process.env.PVZ_QA_OUT||'/tmp/pvz-coin-plants';await mkdir(out,{recursive:true});
const walletUnits=process.env.PVZ_QA_ADVENTURE?20000:Number(process.env.PVZ_QA_WALLET??0);
const browser=await chromium.launch({executablePath:'/Applications/Google Chrome.app/Contents/MacOS/Google Chrome',headless:true,args:['--autoplay-policy=no-user-gesture-required']});
const page=await browser.newPage({viewport:{width:1100,height:750}}),errors=[],results={};
page.on('pageerror',e=>errors.push(e.stack||e.message));
const api=(...a)=>page.evaluate(a=>Module._pvz_sandbox_command(...[...a,0,0,0,0].slice(0,4)),a);
const wait=(fn,args,timeout=30000)=>page.waitForFunction(fn,args,{timeout});
const snap=async name=>{await page.mouse.move(0,0);await page.screenshot({path:join(out,name+'.png')});};
async function click(x,y){const b=await page.locator('#canvas').boundingBox(),s=await page.evaluate(()=>[Module.canvas.width,Module.canvas.height]);await page.mouse.click(b.x+x*b.width/s[0],b.y+y*b.height/s[1]);await page.waitForTimeout(160);}
const data=()=>page.evaluate(()=>{
 const d=Module._pvz_coinplant_data,states=[];
 for(let i=0;i<40&&d(2,i,0)>=0;++i)states.push(Array.from({length:6},(_,f)=>d(2,i,f)));
 return {wallet:d(0,0,0),fired:[0,1,2].map(i=>d(1,i,0)),hits:[0,1,2].map(i=>d(1,i,1)),states};
});
try{
 await page.goto(process.env.PVZ_QA_URL||'http://127.0.0.1:8097/');await wait(()=>!document.getElementById('start').disabled,undefined,90000);
 await page.evaluate(async(walletUnits)=>{
  const FS=Module.FS;if(!FS.analyzePath('/saves/userdata').exists)FS.mkdir('/saves/userdata');
  const name=new TextEncoder().encode('CoinPlantsQA'),users=new Uint8Array(16+name.length),u=new DataView(users.buffer);
  u.setUint32(0,14,true);u.setUint16(4,1,true);u.setUint16(6,name.length,true);users.set(name,8);u.setUint32(8+name.length,1,true);u.setUint32(12+name.length,1,true);FS.writeFile('/saves/userdata/users.dat',users);
  const profile=new Uint8Array(4096),p=new DataView(profile.buffer);p.setUint32(0,12,true);p.setUint32(4,49,true);p.setUint32(8,walletUnits,true);FS.writeFile('/saves/userdata/user1.dat',profile);
  await new Promise((r,j)=>FS.syncfs(false,e=>e?j(e):r()));
 },walletUnits);
 await page.locator('#start').click();await page.waitForTimeout(12000);await click(400,560);await page.waitForTimeout(4500);
 if(process.env.PVZ_QA_ADVENTURE){
  const seed=(i,f)=>page.evaluate(([i,f])=>Module._pvz_adventure_seed_data(i,f),[i,f]);
  const place=async(i,col,row)=>{await page.mouse.click(500,300,{button:'right'});await click(await seed(i,5)+25,await seed(i,6)+35);await click(80+80*col,110+85*row+Math.max(0,5-col)*20);};
  const collect=async()=>{const sun=await page.evaluate(()=>[Module._pvz_sun_data(0,1),Module._pvz_sun_data(0,2)]);if(sun[0]>=0)await click(...sun);};
  const afford=async(cost)=>{const until=Date.now()+100000;while(Date.now()<until){await collect();if(await page.evaluate(()=>Module._pvz_adventure_power_data(-1,2))>=cost)return;await page.waitForTimeout(150);}throw Error('Could not earn ordinary sun');};
  await snap('main-menu');await click(560,135);await page.waitForTimeout(15000);await snap('chooser-before');
  for(const base of [50,38,1,33,3,26]){const slot=base===50?44:base>=8?base+2:base;await click(47+slot%9*53,163+Math.floor(slot/9)*73);await page.waitForTimeout(400);await snap('selected-'+base);}
  await snap('adventure-chooser');await click(258,566);await wait(()=>Module._pvz_adventure_power_data(-1,5)===1);
  const bank=await page.evaluate(()=>Array.from({length:6},(_,i)=>[0,1,2].map(f=>Module._pvz_adventure_seed_data(i,f))));
  assert.deepEqual(bank[0],[530,150,750]);assert.deepEqual(bank[1],[531,200,750]);results.adventureBank=bank;await snap('adventure-start');
  // Roof levels provide existing pots in the left columns. All sun is earned normally.
  await afford(50);await place(2,0,0);await afford(50);await place(2,1,0);await afford(200);await place(1,1,2);
  await wait(()=>Module._pvz_coinplant_data(2,0,4)>0,undefined,16000);await snap('adventure-orbit');
  await afford(150);await place(0,2,2);await wait(()=>{let n=0;while(Module._pvz_coinplant_data(2,n,0)>=0)++n;return n===2;});
  await page.waitForTimeout(1200);await click(748,14);results.beforeSave=await data();await snap('adventure-paused');
  const slotsBefore=await page.evaluate(()=>Array.from({length:2},(_,i)=>Array.from({length:50},(_,s)=>Module._pvz_coinplant_data(3,i,s))));
  await click(400,401);await click(305,394);await page.waitForTimeout(1800);await click(560,135);await wait(()=>Module._pvz_adventure_power_data(-1,5)===1);
  results.afterSave=await data();assert.deepEqual(results.afterSave.states,results.beforeSave.states);assert.equal(results.afterSave.wallet,results.beforeSave.wallet);
  assert.deepEqual(await page.evaluate(()=>Array.from({length:2},(_,i)=>Array.from({length:50},(_,s)=>Module._pvz_coinplant_data(3,i,s)))),slotsBefore);
  await snap('adventure-restored');results.adventureSave=true;
 }else{
 await click(370,455);await click(208,366);await click(408,580);await click(259,127);await snap('almanac-shooter');
 await click(280,580);await click(363,439);await snap('almanac-flower');await click(690,580);await page.waitForTimeout(800);
 await click(719,27);await wait(()=>Module.canvas.width===1024);await api(8,0);await api(4,1);await api(5,4);
 assert.equal((await data()).wallet,walletUnits*10,'sandbox starts with a disposable copy of actual savings');
 for(let row=0;row<5;row++)for(let col=0;col<3;col++)assert.equal(await api(1,531,col,row),1);
 await api(4,0);await wait(()=>Module._pvz_coinplant_data(1,0,0)+Module._pvz_coinplant_data(1,1,0)+Module._pvz_coinplant_data(1,2,0)>=60,undefined,30000);await api(4,1);
 results.flower=await data();assert.equal(results.flower.wallet,walletUnits*10,'all orbit coins are free, including from a zero wallet');
 assert.equal(results.flower.states.reduce((s,p)=>s+p[4],0),results.flower.fired.reduce((s,n)=>s+n,0));
 const paused=await data();await page.waitForTimeout(500);assert.deepEqual(await data(),paused);await snap('orbit-and-glasses');
 await page.setViewportSize({width:844,height:390});await snap('phone');await page.setViewportSize({width:1100,height:750});
 assert.equal(await api(2,219,3,2),1);await api(4,0);
 await wait(()=>[0,1,2].reduce((n,i)=>n+Module._pvz_coinplant_data(1,i,1),0)>0,undefined,15000);await api(4,1);
 results.flowerContact=await data();assert.equal(results.flowerContact.states.length,15);
 assert.equal(results.flowerContact.states.reduce((s,p)=>s+p[4],0),results.flowerContact.fired.reduce((s,n)=>s+n,0),'orbit money remains after actual zombie impacts');
 await snap('persistent-orbit-contact');await api(6);
 for(let row=0;row<5;row++)for(let col=0;col<3;col++)await api(3,0,col,row);
 const afterFlowers=(await data()).wallet;assert.equal(afterFlowers,results.flowerContact.wallet,'removal does not refund ammunition');
 for(let row=0;row<5;row++)for(let col=0;col<3;col++)assert.equal(await api(1,530,col,row),1);
 const refill=async()=>{await api(6);for(let row=0;row<5;row++)for(const col of [5,7])assert.equal(await api(2,219,col,row),1);};
 await refill();await api(4,0);const end=Date.now()+55000;let next=Date.now()+6000;
 while(Date.now()<end){if(Date.now()>=next){await refill();next=Date.now()+6000;}const d=await data();if(d.fired[2]>results.flower.fired[2]&&d.hits.every(n=>n>0)&&d.fired.reduce((s,n)=>s+n,0)>250)break;await page.waitForTimeout(100);}
 await api(4,1);results.shooter=await data();assert.ok(results.shooter.hits.every(n=>n>0),'all three money ammunition types hit actual zombies');
 results.earned=await page.evaluate(()=>Module._pvz_coin_data(-1,1)*10);
 assert.equal(results.shooter.wallet,walletUnits*10+results.earned,'sandbox debits nothing; native zombie coin drops still credit normally');
 await snap('coin-shooter-combat');
 await click(783,24);await click(726,342);await click(305,366);await wait(()=>Module.canvas.width===800);await page.waitForTimeout(2200);
 const wallet=await page.evaluate(()=>{const b=Module.FS.readFile('/saves/userdata/user1.dat');return new DataView(b.buffer,b.byteOffset,b.byteLength).getUint32(8,true);});assert.equal(wallet,walletUnits,'adventure wallet unchanged by sandbox');
 results.sandboxWalletIsolated=true;
 }
 assert.deepEqual(errors,[]);await writeFile(join(out,'report.json'),JSON.stringify({results,errors},null,2));console.log('Coin plants browser QA passed',JSON.stringify(results));
}catch(e){await snap('failure').catch(()=>{});await writeFile(join(out,'failure.json'),JSON.stringify({error:e.stack,results,errors,data:await data().catch(()=>null)},null,2));throw e;}finally{await browser.close();}
