// Real native production and collection in a disposable profile, not a player save.
import assert from 'node:assert/strict';
import {mkdir,writeFile} from 'node:fs/promises';
import {join} from 'node:path';
const {chromium}=await import(process.env.PVZ_PLAYWRIGHT||'playwright-core');
const out=process.env.PVZ_QA_OUT||'/tmp/pvz-sun-production-qa';await mkdir(out,{recursive:true});
const browser=await chromium.launch({executablePath:process.env.PVZ_CHROME||'/Applications/Google Chrome.app/Contents/MacOS/Google Chrome',headless:true,args:['--no-sandbox']});
const page=await browser.newPage({viewport:{width:1100,height:750},hasTouch:true}),errors=[],results={};page.on('pageerror',e=>errors.push(e.message));
async function point(x,y){const b=await page.locator('#canvas').boundingBox(),s=await page.evaluate(()=>[Module.canvas.width,Module.canvas.height]);return[b.x+x*b.width/s[0],b.y+y*b.height/s[1]];}
async function click(x,y){await page.mouse.click(...await point(x,y));await page.waitForTimeout(150);}
async function api(...v){return page.evaluate(v=>Module._pvz_sandbox_command(...[...v,0,0,0,0].slice(0,4)),v);}
async function shot(name){await page.screenshot({path:join(out,name+'.png')});}
async function suns(){return page.evaluate(()=>{const a=[];for(let i=0;i<1000&&Module._pvz_sun_data(i,0)>=0;++i)a.push(Array.from({length:7},(_,f)=>Module._pvz_sun_data(i,f)));return a;});}
async function ad(i,f){return page.evaluate(([i,f])=>Module._pvz_adventure_power_data(i,f),[i,f]);}
async function waitSun(value){await page.waitForFunction(value=>{for(let i=0;i<1000&&Module._pvz_sun_data(i,0)>=0;++i)if(Module._pvz_sun_data(i,0)===value&&Module._pvz_sun_data(i,3)===2)return true;return false;},value,{timeout:18000});return(await suns()).find(s=>s[0]===value&&s[3]===2);}
async function collect(s){const before=await ad(-1,2);await page.mouse.move(...await point(s[1],s[2]));await page.waitForFunction(n=>Module._pvz_adventure_power_data(-1,2)===n,before+s[0],{timeout:4000});return before+s[0];}
try{
 await page.goto(process.env.PVZ_QA_URL||'http://127.0.0.1:8097/');await page.waitForFunction(()=>!document.getElementById('start').disabled,undefined,{timeout:90000});
 await page.evaluate(async()=>{
  const FS=Module.FS;if(!FS.analyzePath('/saves/userdata').exists)FS.mkdir('/saves/userdata');
  const name=new TextEncoder().encode('SunQA'),users=new Uint8Array(16+name.length),u=new DataView(users.buffer);u.setUint32(0,14,true);u.setUint16(4,1,true);u.setUint16(6,name.length,true);users.set(name,8);u.setUint32(8+name.length,1,true);u.setUint32(12+name.length,1,true);FS.writeFile('/saves/userdata/users.dat',users);
  // 2-3 avoids 2-2's one-time shop tutorial before the native seed chooser.
  const profile=new Uint8Array(4096),p=new DataView(profile.buffer);p.setUint32(0,12,true);p.setUint32(4,13,true);FS.writeFile('/saves/userdata/user1.dat',profile);await new Promise((r,j)=>FS.syncfs(false,e=>e?j(e):r()));
 });
 await page.locator('#start').click();await page.waitForTimeout(12000);await click(400,560);await page.waitForTimeout(4500);await click(260,348);await page.waitForFunction(()=>Module.canvas.width===1024,undefined,{timeout:20000});
 await api(7);await api(1,1,0,2);await api(1,503,3,2);await api(1,9,6,2);await api(5,4);await page.mouse.move(0,0);await api(4,0);
 await page.waitForFunction(()=>{const types=new Set();for(let i=0;i<1000&&Module._pvz_sun_data(i,0)>=0;++i){if(Module._pvz_sun_data(i,3)!==2)continue;const x=Module._pvz_sun_data(i,1),v=Module._pvz_sun_data(i,0);if(x<150&&v===50)types.add(1);if(x>230&&x<390&&v===50)types.add(503);if(x>470&&v===15)types.add(9);}return types.size===3;},undefined,{timeout:10000});
 await api(4,1);results.initial=await suns();await shot('native-and-replacement-50-young-shroom-15');
 await api(4,0);await page.waitForFunction(()=>Module._pvz_sandbox_plant_data(2,8)===25,undefined,{timeout:40000});
 await page.waitForFunction(()=>{for(let i=0;i<1000&&Module._pvz_sun_data(i,0)>=0;++i)if(Module._pvz_sun_data(i,0)===50&&Module._pvz_sun_data(i,1)>470&&Module._pvz_sun_data(i,3)===2)return true;return false;},undefined,{timeout:9000});
 await api(4,1);results.mature=await suns();assert.equal(await page.evaluate(()=>Module._pvz_sandbox_plant_data(2,8)),25);await shot('grown-shroom-produces-50');
 await api(15);await page.waitForTimeout(900);await click(560,135);await page.waitForTimeout(15000);
 for(const id of [1,9,8,3,0,2]){const slot=id>=8?id+1:id;await click(47+slot%9*53,163+Math.floor(slot/9)*73);}await click(258,566);
 await page.waitForFunction(()=>Module._pvz_adventure_power_data(-1,5)===1,undefined,{timeout:25000});
 const bank=await page.evaluate(()=>Array.from({length:6},(_,i)=>Module._pvz_adventure_seed_data(i,0)));assert.deepEqual(bank,[503,9,502,501,500,2]);
 await click(110,43);await click(80,330);assert.equal(await ad(-1,2),0);await page.mouse.move(0,0);
 results.adventureFlower=await collect(await waitSun(50));assert.equal(results.adventureFlower,50);
 await click(160,43);await click(240,330);assert.equal(await ad(-1,2),25);await page.mouse.move(0,0);
 results.adventureYoungShroom=await collect(await waitSun(15));assert.equal(results.adventureYoungShroom,40);await shot('adventure-collected-actual-values');
 assert.deepEqual(errors,[]);await writeFile(join(out,'report.json'),JSON.stringify({results,errors},null,2));console.log('Native sun production and adventure collection passed',results);
}catch(e){await shot('failure');await writeFile(join(out,'failure.json'),JSON.stringify({error:String(e),results,suns:await suns(),errors},null,2));throw e;}finally{await browser.close();}
