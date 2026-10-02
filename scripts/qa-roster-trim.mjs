// Real WASM mechanics, audio channels and native adventure cards in a disposable profile.
import assert from 'node:assert/strict';
import {mkdir,writeFile} from 'node:fs/promises';
import {join} from 'node:path';
const {chromium}=await import(process.env.PVZ_PLAYWRIGHT||'playwright-core');
const out=process.env.PVZ_QA_OUT||'/tmp/pvz-roster-trim-qa';await mkdir(out,{recursive:true});
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
  const name=new TextEncoder().encode('RosterQA'),users=new Uint8Array(16+name.length),u=new DataView(users.buffer);u.setUint32(0,14,true);u.setUint16(4,1,true);u.setUint16(6,name.length,true);users.set(name,8);u.setUint32(8+name.length,1,true);u.setUint32(12+name.length,1,true);FS.writeFile('/saves/userdata/users.dat',users);
  const profile=new Uint8Array(4096),p=new DataView(profile.buffer);p.setUint32(0,12,true);p.setUint32(4,9,true);FS.writeFile('/saves/userdata/user1.dat',profile);await new Promise((r,j)=>FS.syncfs(false,e=>e?j(e):r()));});
 await page.locator('#start').click();await page.waitForTimeout(12000);await click(400,560);await page.waitForTimeout(4500);await click(719,27);await wait(()=>Module.canvas.width===1024);

 await fresh();
 for(let id=502;id<=518;++id)assert.equal(await api(1,id,0,0),-2);
 for(const id of [52,100,119,120,121,122,180,181,182,300,443])assert.equal(await api(1,id,0,0),-2);
 for(let id=200;id<212;++id)assert.equal(await api(2,id,8,2),-2);
 results.retiredIDsRejected=true;
 for(const [id,col] of [[500,1],[501,3],[1,5],[8,7]])assert.ok(await api(1,id,col,2)>0);
 assert.deepEqual(await page.evaluate(()=>Array.from({length:4},(_,i)=>Module._pvz_sandbox_plant_data(i,0))),[500,501,1,8]);
 await shot('only-two-original-cards');results.onlyTwoOriginals=true;
 await click(657,17);await api(2,5,8,1);await api(2,4,7,3);await shot('native-zombie-catalog');
 await page.setViewportSize({width:844,height:390});await page.waitForTimeout(300);await shot('mobile-native-roster');
 await page.setViewportSize({width:1100,height:750});await fresh();

 // Ordinary walkers and imps do not rest, reverse or acquire gag phases.
 await api(2,0,8,1);await api(2,24,8,3);await api(5,4);await api(4,0);
 await wait(()=>Module._pvz_sandbox_zombie_data(0,10)>=650);
 await api(4,1);
 for(let i=0;i<2;++i){assert.equal(await zd(i,9),100);assert.equal(await zd(i,12),0);assert.equal(await zd(i,6),0);}
 results.nativeZombieBehavior=true;
 await fresh();await api(1,501,3,2);await api(2,0,3,2);const hp=await zd(0,4);await api(4,0);
 await wait(()=>Module._pvz_sandbox_plant_data(0,4)===1);await api(4,1);const x=await zd(0,2);
 for(let tick=0;tick<24;++tick){await api(13);await page.waitForTimeout(20);}
 assert.equal(await zd(0,4),hp-80);assert.ok(await zd(0,2)<=x);await shot('retained-walnut-bump');results.walnutKept=true;

 await api(15);await page.waitForTimeout(800);await click(560,135);await page.waitForTimeout(15000);
 for(const base of [0,3,1,5,4,7])await click(47+(base%8)*53,163+Math.floor(base/8)*73);
 await shot('adventure-native-eight-column-chooser');await click(232,566);
 await wait(()=>Module._pvz_adventure_power_data(-1,5)===1,undefined,25000);
 const bank=await page.evaluate(()=>Array.from({length:6},(_,i)=>[0,1,2].map(f=>Module._pvz_adventure_seed_data(i,f))));
 assert.deepEqual(bank.map(p=>p[0]),[500,501,1,5,4,7]);
 assert.deepEqual(bank.map(p=>p[1]),[100,50,50,175,25,200]);
 assert.deepEqual(bank.map(p=>p[2]),[300,300,750,750,3000,750]);
 results.adventureBank=bank;await shot('adventure-two-replacements');
 assert.deepEqual(errors,[]);await writeFile(join(out,'report.json'),JSON.stringify({results,errors},null,2));console.log('Roster trim browser QA passed',results);
}catch(e){await shot('failure');await writeFile(join(out,'failure.json'),JSON.stringify({error:String(e),results,errors,engine:await page.evaluate(()=>window.pvzEngineLog)},null,2));throw e;}
finally{await browser.close();}
