// Isolated browser profile; exercise the shipped WASM, never the player's save.
import assert from 'node:assert/strict';
import {mkdir,writeFile} from 'node:fs/promises';
import {join} from 'node:path';
const {chromium}=await import(process.env.PVZ_PLAYWRIGHT||'playwright-core');
const out=process.env.PVZ_QA_OUT||'/tmp/pvz-clever-qa';await mkdir(out,{recursive:true});
const browser=await chromium.launch({executablePath:'/Applications/Google Chrome.app/Contents/MacOS/Google Chrome',headless:true,args:['--no-sandbox','--autoplay-policy=no-user-gesture-required']});
const page=await browser.newPage({viewport:{width:1100,height:750}}),errors=[],results={};page.on('pageerror',e=>errors.push(e.message));
const api=(...a)=>page.evaluate(a=>Module._pvz_sandbox_command(...[...a,0,0,0,0].slice(0,4)),a);
const snap=n=>page.screenshot({path:join(out,n+'.png')});
const wait=(f,t=30000)=>page.waitForFunction(f,undefined,{timeout:t});
const zombies=()=>page.evaluate(()=>{const out=[];for(let i=0;i<100;++i){const z=Array.from({length:25},(_,f)=>Module._pvz_sandbox_zombie_data(i,f));if(z[0]<0)break;out.push(z);}return out;});
async function click(x,y){const b=await page.locator('#canvas').boundingBox(),s=await page.evaluate(()=>[Module.canvas.width,Module.canvas.height]);await page.mouse.click(b.x+x*b.width/s[0],b.y+y*b.height/s[1]);await page.waitForTimeout(100);}
async function steps(n){for(let i=0;i<n;++i){await api(13);await page.waitForTimeout(35);}}
async function fresh(){await api(7);await api(4,1);await api(5,1);}
try{
 await page.goto(process.env.PVZ_QA_URL||'http://127.0.0.1:8097/');await wait(()=>!document.getElementById('start').disabled,90000);
 await page.evaluate(async()=>{
  const FS=Module.FS;if(!FS.analyzePath('/saves/userdata').exists)FS.mkdir('/saves/userdata');
  const name=new TextEncoder().encode('CleverQA'),users=new Uint8Array(16+name.length),u=new DataView(users.buffer);
  u.setUint32(0,14,true);u.setUint16(4,1,true);u.setUint16(6,name.length,true);users.set(name,8);u.setUint32(8+name.length,1,true);u.setUint32(12+name.length,1,true);FS.writeFile('/saves/userdata/users.dat',users);
  const profile=new Uint8Array(4096),p=new DataView(profile.buffer);p.setUint32(0,12,true);p.setUint32(4,37,true);FS.writeFile('/saves/userdata/user1.dat',profile);await new Promise((r,j)=>FS.syncfs(false,e=>e?j(e):r()));
 });
 await page.locator('#start').click();await page.waitForTimeout(12000);await click(400,560);await page.waitForTimeout(4500);
 if(!process.env.PVZ_QA_ALMANAC_ONLY){
 await click(260,348);await wait(()=>Module.canvas.width===1024);await fresh();
 assert.equal(await api(2,216,6,1),1);assert.equal(await api(2,217,6,3),1);
 results.spawn=await zombies();assert.equal(results.spawn[0][4],270);assert.equal(results.spawn[1][5],370);assert.equal(results.spawn[0][9],125);
 for(let row=0;row<5;++row)await api(1,0,0,row);
 await snap('roster');await api(4,0);
 await wait(()=>{for(let i=0;i<10;++i)if(Module._pvz_sandbox_zombie_data(i,6)===1044)return true;return false;});await api(4,1);
 results.flip=await zombies();await snap('flip-start');
 for(let frame=0;frame<4;++frame){await steps(18);await snap('flip-'+frame);}
 await api(4,0);await page.waitForTimeout(2000);await api(4,1);results.landed=await zombies();await snap('landed');
 // Native target selection steals the main plant, leaving its lily pad intact.
 await api(8,1);await fresh();await api(1,16,4,2);await api(1,3,4,2);await api(2,217,4,2);await api(4,0);
 await wait(()=>Module._pvz_sandbox_zombie_data(0,22)===3);await api(4,1);
 results.carry=await zombies();assert.equal(results.carry[0][21],1);assert.equal(results.carry[0][18],1);assert.equal(results.carry[0][9],300);
 assert.equal(await page.evaluate(()=>Module._pvz_sandbox_plant_data(0,0)),16);assert.equal(await page.evaluate(()=>Module._pvz_sandbox_plant_data(1,0)),-1);
 await snap('pool-carry');const x=results.carry[0][2];await steps(60);assert.ok((await zombies())[0][2]>x);
 await api(5,4);await api(4,0);await wait(()=>Module._pvz_sandbox_zombie_data(0,0)===-1);results.escaped=true;
 // Cover every adjacent lane: a successful lane dodge must not make this
 // assertion accidentally treat a healthy right-edge escape (-1) as damage.
 await api(8,0);await fresh();await api(1,3,4,2);await api(2,216,4,2);await api(4,0);await wait(()=>Module._pvz_sandbox_zombie_data(0,22)===3);await api(4,1);await snap('land-carry');
 for(let row=0;row<5;++row)await api(1,0,3,row);
 const hp=(await zombies())[0][4];await api(4,0);await wait(()=>{const hp=Module._pvz_sandbox_zombie_data(0,4);return hp>=0&&hp<270;});await api(4,1);results.escapeDamage=hp-(await zombies())[0][4];assert.ok(results.escapeDamage>0);
 await page.setViewportSize({width:844,height:390});await page.waitForTimeout(1000);await snap('phone');await page.setViewportSize({width:1100,height:750});
 await api(15);await page.waitForTimeout(1800);
 }
 await click(370,455);await click(590,366);await click(53,517);await page.waitForTimeout(700);await snap('almanac-clever');
 await click(124,517);await page.waitForTimeout(700);await snap('almanac-cone');
 assert.deepEqual(errors,[]);await writeFile(join(out,'report.json'),JSON.stringify({results,errors},null,2));console.log('Clever zombie browser QA passed',out,results);
}catch(e){await snap('failure');await writeFile(join(out,'failure.json'),JSON.stringify({error:String(e),results,errors,zombies:await zombies(),log:await page.evaluate(()=>window.pvzEngineLog)},null,2));throw e;}
finally{await browser.close();}
