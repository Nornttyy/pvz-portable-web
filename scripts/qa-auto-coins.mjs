// Native coin drops from marigolds in an isolated sandbox, with no coin clicks.
import assert from 'node:assert/strict';
import {mkdir,writeFile} from 'node:fs/promises';
import {join} from 'node:path';
const {chromium}=await import(process.env.PVZ_PLAYWRIGHT||'playwright-core');
const out=process.env.PVZ_QA_OUT||'/tmp/pvz-auto-coins';await mkdir(out,{recursive:true});
const browser=await chromium.launch({executablePath:'/Applications/Google Chrome.app/Contents/MacOS/Google Chrome',headless:true});
const page=await browser.newPage({viewport:{width:1100,height:750},hasTouch:true}),errors=[],results={};
page.on('pageerror',e=>errors.push(e.stack||e.message));
const api=(...a)=>page.evaluate(a=>Module._pvz_sandbox_command(...[...a,0,0,0,0].slice(0,4)),a);
const coin=(i,f)=>page.evaluate(([i,f])=>Module._pvz_coin_data(i,f),[i,f]);
async function click(x,y){const b=await page.locator('#canvas').boundingBox(),s=await page.evaluate(()=>[Module.canvas.width,Module.canvas.height]);await page.mouse.click(b.x+x*b.width/s[0],b.y+y*b.height/s[1]);await page.waitForTimeout(200);}
const snap=async n=>{await page.mouse.move(0,0);await page.screenshot({path:join(out,n+'.png')});};
const live=()=>page.evaluate(()=>{const a=[];for(let i=0;i<300;i++){const type=Module._pvz_coin_data(i,0);if(type<0)break;a.push(Array.from({length:8},(_,f)=>Module._pvz_coin_data(i,f)));}return a;});
try{
 await page.goto(process.env.PVZ_QA_URL||'http://127.0.0.1:8097/');await page.waitForFunction(()=>!document.getElementById('start').disabled,undefined,{timeout:90000});
 await page.evaluate(async()=>{
  const FS=Module.FS;if(!FS.analyzePath('/saves/userdata').exists)FS.mkdir('/saves/userdata');
  const name=new TextEncoder().encode('AutoCoinQA'),users=new Uint8Array(16+name.length),u=new DataView(users.buffer);
  u.setUint32(0,14,true);u.setUint16(4,1,true);u.setUint16(6,name.length,true);users.set(name,8);u.setUint32(8+name.length,1,true);u.setUint32(12+name.length,1,true);FS.writeFile('/saves/userdata/users.dat',users);
  const profile=new Uint8Array(4096),p=new DataView(profile.buffer);p.setUint32(0,12,true);p.setUint32(4,27,true);p.setUint32(8,123,true);FS.writeFile('/saves/userdata/user1.dat',profile);
  await new Promise((r,j)=>FS.syncfs(false,e=>e?j(e):r()));
  window.coinClicks=0;document.addEventListener('mousedown',()=>window.coinClicks++);document.addEventListener('touchstart',()=>window.coinClicks++);
 });
 await page.locator('#start').click();await page.waitForTimeout(12000);await click(400,560);await page.waitForTimeout(4500);await click(719,27);
 await page.waitForFunction(()=>Module.canvas.width===1024);await api(4,1);
 for(let row=0;row<5;row++)for(let col=0;col<8;col++)assert.equal(await api(1,38,col,row),1);
 const before=await coin(-1,0),clicks=await page.evaluate(()=>window.coinClicks);await api(5,4);await api(4,0);
 const ledger=new Map(),types=new Set();let flying=false;
 const sample=async()=>{const a=await live();for(const c of a)if(c[0]===1||c[0]===2){ledger.set(c[7],c[6]);types.add(c[0]);if(c[2]){flying=true;assert.ok(c[1]>=60);}}return a;};
 const deadline=Date.now()+25000;
 while(Date.now()<deadline){await sample();if(types.size===2&&ledger.size>=30&&flying)break;await page.waitForTimeout(20);}
 assert.equal(types.size,2,'real silver and gold drops both observed');assert.ok(flying,'normal collection flight occurs');
 await api(4,1);await sample();const paused=await live(),money=await coin(-1,0);await page.waitForTimeout(700);assert.deepEqual(await live(),paused);assert.equal(await coin(-1,0),money);results.pause=true;
 await snap('paused-coin-flight');
 // Remove producers only, preserving dropped coins, then account for every coin exactly once.
 for(let row=0;row<5;row++)for(let col=0;col<8;col++)assert.equal(await api(3,0,col,row),1);
 await page.setViewportSize({width:844,height:390});await api(5,1);await api(4,0);
 await page.waitForFunction(()=>{for(let i=0;i<300;i++){const t=Module._pvz_coin_data(i,0);if(t<0)break;if(t===1||t===2)return false;}return true;},undefined,{timeout:12000});
 const expected=[...ledger.values()].reduce((a,b)=>a+b,0);assert.equal(await coin(-1,0)-before,expected);assert.equal(await coin(-1,1),expected);assert.equal(await coin(-1,2),ledger.size);
 assert.equal(await page.evaluate(()=>window.coinClicks),clicks,'no mouse/touch input collected coins');
 results.silverAndGold=true;results.normalFlight=true;results.exactCredit={drops:ledger.size,value:expected};results.mobileNoTap=true;await snap('phone-coins-credited');
 // Sandbox deliberately disables sky drops, so use a real sunflower's production.
 assert.equal(await api(1,520,8,2),1);
 await page.waitForFunction(()=>Module._pvz_sun_data(0,0)===25,undefined,{timeout:15000});
 await page.waitForTimeout(800);const sun=(await live()).find(c=>c[0]===4);assert.ok(sun&&!sun[2],'sunflower suns still wait for their existing collection action');results.sunUnchanged=true;
 await click(783,24);await click(726,342);await click(305,366);await page.waitForFunction(()=>Module.canvas.width===800);await page.waitForTimeout(2200);
 const saved=await page.evaluate(()=>{const b=Module.FS.readFile('/saves/userdata/user1.dat');return new DataView(b.buffer,b.byteOffset,b.byteLength).getUint32(8,true);});assert.equal(saved,123,'sandbox test money must not alter adventure savings');results.adventureSavingsPreserved=true;
 assert.deepEqual(errors,[]);await writeFile(join(out,'report.json'),JSON.stringify({results,errors},null,2));console.log('Native automatic coin pickup passed',results);
}catch(e){await snap('failure').catch(()=>{});await writeFile(join(out,'failure.json'),JSON.stringify({error:e.stack,results,errors},null,2));throw e;}finally{await browser.close();}
