// Real WASM and screenshots in an isolated browser profile (never user saves).
import assert from 'node:assert/strict';
import {mkdir,writeFile} from 'node:fs/promises';
import {join} from 'node:path';
const {chromium}=await import(process.env.PVZ_PLAYWRIGHT||'playwright-core');
const out=process.env.PVZ_QA_OUT||'/tmp/pvz-cone-variants';await mkdir(out,{recursive:true});
const browser=await chromium.launch({executablePath:'/Applications/Google Chrome.app/Contents/MacOS/Google Chrome',headless:true,args:['--no-sandbox','--autoplay-policy=no-user-gesture-required']});
const page=await browser.newPage({viewport:{width:1100,height:750}}),errors=[],results={};page.on('pageerror',e=>errors.push(e.message));
const api=(...a)=>page.evaluate(a=>Module._pvz_sandbox_command(...[...a,0,0,0,0].slice(0,4)),a);
const snap=n=>page.screenshot({path:join(out,n+'.png')});
const zombies=()=>page.evaluate(()=>{const out=[];for(let i=0;i<100;++i){const z=Array.from({length:30},(_,f)=>Module._pvz_sandbox_zombie_data(i,f));if(z[0]<0)break;out.push(z);}return out;});
async function click(x,y){const b=await page.locator('#canvas').boundingBox(),s=await page.evaluate(()=>[Module.canvas.width,Module.canvas.height]);await page.mouse.click(b.x+x*b.width/s[0],b.y+y*b.height/s[1]);await page.waitForTimeout(150);}
async function fresh(){await api(7);await api(4,1);await api(5,1);}
try{
 await page.goto(process.env.PVZ_QA_URL||'http://127.0.0.1:8097/');await page.waitForFunction(()=>!document.getElementById('start').disabled,undefined,{timeout:90000});
 await page.evaluate(async()=>{
  const FS=Module.FS;if(!FS.analyzePath('/saves/userdata').exists)FS.mkdir('/saves/userdata');
  const name=new TextEncoder().encode('ConeQA'),users=new Uint8Array(16+name.length),u=new DataView(users.buffer);
  u.setUint32(0,14,true);u.setUint16(4,1,true);u.setUint16(6,name.length,true);users.set(name,8);u.setUint32(8+name.length,1,true);u.setUint32(12+name.length,1,true);FS.writeFile('/saves/userdata/users.dat',users);
  const profile=new Uint8Array(4096),p=new DataView(profile.buffer);p.setUint32(0,12,true);p.setUint32(4,37,true);FS.writeFile('/saves/userdata/user1.dat',profile);await new Promise((r,j)=>FS.syncfs(false,e=>e?j(e):r()));
 });
 await page.locator('#start').click();await page.waitForTimeout(12000);await click(400,560);await page.waitForTimeout(4500);
 await click(260,348);await page.waitForFunction(()=>Module.canvas.width===1024);await fresh();
 for(const [type,col,row] of [[2,2,3],[218,4,3],[219,6,3],[214,8,3]])assert.equal(await api(2,type,col,row),1);
 results.initial=await zombies();assert.deepEqual(results.initial.map(z=>z[5]),[370,740,7400,2590]);assert.deepEqual(results.initial.map(z=>z[9]),[100,100,15,60]);
 await click(710,24);await snap('roster');await api(4,0);await page.waitForTimeout(8000);await api(4,1);results.walked=await zombies();
 const distance=results.initial.map((z,i)=>(z[28]-results.walked[i][28])/1000);assert.ok(distance[0]>8&&distance[2]>0&&distance[2]<distance[0]*.25);results.walkDistance=distance;
 await snap('walking');
 // Real bullets damage the stack, rather than a debug HP setter. Native
 // cherry bombs burn normal-bodied zombies regardless of helmet durability.
 await fresh();await api(2,219,6,3);assert.equal(await api(1,7,1,3),1);assert.equal(await api(1,40,1,3),1);await api(5,4);await api(4,0);
 await page.waitForFunction(()=>{const hp=Module._pvz_sandbox_zombie_data(0,5);return hp>0&&hp<=6000;});await api(4,1);
 results.damaged=await zombies();assert.ok(results.damaged[0][5]<=6000&&results.damaged[0][5]>5000);await snap('damaged-stack');
 await fresh();await api(2,218,6,3);await api(1,7,1,3);await api(1,40,1,3);await api(4,0);
 await page.waitForFunction(()=>{const hp=Module._pvz_sandbox_zombie_data(0,5);return hp>247&&hp<490;});await api(4,1);await snap('green-damage-1');
 await api(4,0);await page.waitForFunction(()=>{const hp=Module._pvz_sandbox_zombie_data(0,5);return hp>0&&hp<240;});await api(4,1);await snap('green-damage-2');
 await api(8,1);await fresh();assert.equal(await api(2,219,7,2),-5);assert.equal(await api(2,219,7,3),-5);assert.deepEqual(await zombies(),[]);
 assert.equal(await api(2,219,7,4),1);assert.equal(await api(2,218,6,2),1);await api(4,0);await page.waitForTimeout(1000);await api(4,1);
 results.pool=await zombies();assert.equal(results.pool[1][21],1);await snap('pool-restriction');
 await page.setViewportSize({width:844,height:390});await page.waitForTimeout(600);await snap('phone');await page.setViewportSize({width:1100,height:750});
 await api(15);await page.waitForTimeout(1800);await click(370,455);await click(590,366);
 for(const [name,x,y] of [['wrap-name',337,437],['green-almanac',194,517],['tower-almanac',265,517]]){await click(x,y);await page.waitForTimeout(500);await snap(name);}
 assert.deepEqual(errors,[]);await writeFile(join(out,'report.json'),JSON.stringify({results,errors},null,2));console.log('Cone variants browser QA passed',out,results.walkDistance);
}catch(e){await snap('failure');await writeFile(join(out,'failure.json'),JSON.stringify({error:String(e),results,errors,zombies:await zombies(),log:await page.evaluate(()=>window.pvzEngineLog)},null,2));throw e;}
finally{await browser.close();}
