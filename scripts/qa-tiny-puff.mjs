// Browser exercise of the actual deployed engine; separate disposable profile.
import assert from 'node:assert/strict';
import {mkdir,writeFile} from 'node:fs/promises';
import {join} from 'node:path';
const {chromium}=await import(process.env.PVZ_PLAYWRIGHT||'playwright-core');
const out=process.env.PVZ_QA_OUT||'/tmp/pvz-tiny-puff-qa';await mkdir(out,{recursive:true});
const browser=await chromium.launch({executablePath:'/Applications/Google Chrome.app/Contents/MacOS/Google Chrome',headless:true,args:['--no-sandbox','--autoplay-policy=no-user-gesture-required']});
const context=await browser.newContext({viewport:{width:1100,height:750},hasTouch:true}),page=await context.newPage(),results={},errors=[];
page.on('pageerror',e=>errors.push(e.message));
const wait=(f,a,t=30000)=>page.waitForFunction(f,a,{timeout:t});
const api=(...a)=>page.evaluate(a=>Module._pvz_sandbox_command(...[...a,0,0,0,0].slice(0,4)),a);
const seed=(i,f)=>page.evaluate(([i,f])=>Module._pvz_adventure_seed_data(i,f),[i,f]);
const snap=async n=>{await page.mouse.move(0,0);await page.waitForTimeout(150);await page.screenshot({path:join(out,n+'.png')});};
async function click(x,y,touch=false){const b=await page.locator('#canvas').boundingBox(),s=await page.evaluate(()=>[Module.canvas.width,Module.canvas.height]);const pos=[b.x+x*b.width/s[0],b.y+y*b.height/s[1]];if(touch)await page.touchscreen.tap(...pos);else await page.mouse.click(...pos);await page.waitForTimeout(120);}
async function card(base){const slot=base===52?8:base===53?9:base>=8?base+2:base;await click(47+slot%9*53,163+Math.floor(slot/9)*73);}
const plants=()=>page.evaluate(()=>Array.from({length:10},(_,i)=>[0,1,2,3,9,15].map(f=>Module._pvz_adventure_power_data(i,f))).filter(p=>p[0]>=0));
// Night stages randomize graves in the right half; exercise a safe left tile.
async function place(){await wait(()=>Module._pvz_adventure_seed_data(0,4)===1);await click(await seed(0,5)+25,await seed(0,6)+35);await click(200,330);}
async function quit(){await click(748,14);await page.waitForTimeout(350);await snap('pause-menu');await click(400,401);await page.waitForTimeout(350);await click(305,394);await wait(()=>Module._pvz_adventure_power_data(-1,5)===-1);await page.waitForTimeout(1800);}
try{
 await page.goto(process.env.PVZ_QA_URL||'http://127.0.0.1:8097/');await wait(()=>!document.getElementById('start').disabled,undefined,90000);
 await page.evaluate(async()=>{
  const FS=Module.FS;if(!FS.analyzePath('/saves/userdata').exists)FS.mkdir('/saves/userdata');
  const name=new TextEncoder().encode('TinyPuffQA'),users=new Uint8Array(16+name.length),u=new DataView(users.buffer);
  u.setUint32(0,14,true);u.setUint16(4,1,true);u.setUint16(6,name.length,true);users.set(name,8);u.setUint32(8+name.length,1,true);u.setUint32(12+name.length,1,true);FS.writeFile('/saves/userdata/users.dat',users);
  const profile=new Uint8Array(4096),p=new DataView(profile.buffer);p.setUint32(0,12,true);p.setUint32(4,18,true);FS.writeFile('/saves/userdata/user1.dat',profile);
  await new Promise((r,j)=>FS.syncfs(false,e=>e?j(e):r()));
 });
 await page.locator('#start').click();await page.waitForTimeout(12000);await click(400,560);await page.waitForTimeout(4500);await click(560,135);await page.waitForTimeout(15000);
 for(const base of [8,0,1,3,4,5])await card(base);await snap('chooser');await click(258,566);await wait(()=>Module._pvz_adventure_power_data(-1,5)===1);
 assert.equal(await seed(0,0),525);assert.equal(await seed(0,1),0);assert.equal(await seed(0,2),200);
 for(let i=0;i<5;++i){await place();assert.equal((await plants()).length,i+1);assert.ok(await seed(0,3)>160);}
 results.cluster=await plants();assert.deepEqual(results.cluster.map(p=>p[5]),[0,1,2,3,4]);assert.ok(results.cluster.every(p=>p[3]===300&&p[4]===0));
 await place();assert.equal((await plants()).length,5);assert.equal(await seed(0,3),0);await click(await seed(0,5)+25,await seed(0,6)+35);await snap('adventure-five');
 await page.setViewportSize({width:390,height:844});await page.waitForTimeout(500);await snap('phone');await page.setViewportSize({width:1100,height:750});await page.waitForTimeout(800);
 await quit();await click(560,135);await wait(()=>Module._pvz_adventure_power_data(-1,5)===1);assert.deepEqual(await plants(),results.cluster);results.saveResume=true;await snap('resumed');
 await click(280,371);await click(480,40);await click(200,330);await page.waitForTimeout(250);assert.equal((await plants()).length,4);await place();assert.equal((await plants()).length,5);assert.deepEqual((await plants()).map(p=>p[5]).sort(),[0,1,2,3,4]);results.shovelRefill=true;
 await quit();await click(260,348);await wait(()=>Module.canvas.width===1024);await api(7);await api(8);await api(4,1);
 for(let i=0;i<5;++i)assert.equal(await api(1,525,3,2),1);assert.equal(await api(1,525,3,2),-4);await api(19,1);assert.equal(await api(1,525,3,2),-4);await api(19,0);
 await api(3,0,3,2);assert.equal(await api(9),4);assert.equal(await api(1,525,3,2),1);results.sandboxShovelRefill=true;
 await api(1,8,2,2); // Native-only reference for comparing the exact one-third scale.
 await api(13);await api(4,0);await page.waitForTimeout(300);await api(4,1);await snap('sandbox-scale-and-poses');
 await api(3,0,2,2);for(let i=0;i<4;++i)await api(3,0,3,2);assert.equal(await api(9),1);
 await api(2,0,5,2);await api(4,0);await wait(()=>Module._pvz_sandbox_zombie_data(0,4)<270);await api(4,1);results.hitHP=await page.evaluate(()=>Module._pvz_sandbox_zombie_data(0,4));assert.equal(results.hitHP,260);await snap('native-puff-shots');
 assert.deepEqual(errors,[]);await writeFile(join(out,'report.json'),JSON.stringify({results,errors},null,2));console.log('Tiny puff QA passed',out,results);
}catch(e){await snap('failure');await writeFile(join(out,'failure.json'),JSON.stringify({error:String(e),results,errors,log:await page.evaluate(()=>window.pvzEngineLog)},null,2));throw e;}
finally{await browser.close();}
