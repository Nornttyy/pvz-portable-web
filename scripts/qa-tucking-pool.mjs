// Actual WASM: reproduce a swimming zombie eating the pad under a tucked
// sunflower. Isolated browser/profile, no player save changes or memory edits.
import assert from 'node:assert/strict';
import {mkdir,writeFile} from 'node:fs/promises';
import {join} from 'node:path';
const {chromium}=await import(process.env.PVZ_PLAYWRIGHT||'playwright-core');
const out=process.env.PVZ_QA_OUT||'/tmp/pvz-tucking-pool-qa';await mkdir(out,{recursive:true});
const expectOrphan=process.env.PVZ_EXPECT_ORPHAN==='1';
const browser=await chromium.launch({executablePath:'/Applications/Google Chrome.app/Contents/MacOS/Google Chrome',headless:true,args:['--no-sandbox','--autoplay-policy=no-user-gesture-required']});
const page=await browser.newPage({viewport:{width:1100,height:750}}),errors=[],results={expectOrphan};page.on('pageerror',e=>errors.push(e.message));
const wait=(f,a,t=30000)=>page.waitForFunction(f,a,{timeout:t});
const api=(...a)=>page.evaluate(a=>Module._pvz_sandbox_command(...[...a,0,0,0,0].slice(0,4)),a);
const snap=n=>page.screenshot({path:join(out,n+'.png')});
const plants=()=>page.evaluate(()=>{
 const all=[];for(let i=0;i<32;++i){if(Module._pvz_sandbox_plant_data(i,0)<0)break;all.push(Array.from({length:12},(_,f)=>Module._pvz_sandbox_plant_data(i,f)));}return all;
});
async function click(x,y){const b=await page.locator('#canvas').boundingBox(),s=await page.evaluate(()=>[Module.canvas.width,Module.canvas.height]);await page.mouse.click(b.x+x*b.width/s[0],b.y+y*b.height/s[1]);await page.waitForTimeout(100);}
try{
 await page.goto(process.env.PVZ_QA_URL||'http://127.0.0.1:8097/');await wait(()=>!document.getElementById('start').disabled,undefined,90000);
 await page.evaluate(async()=>{
  const FS=Module.FS;if(!FS.analyzePath('/saves/userdata').exists)FS.mkdir('/saves/userdata');
  const name=new TextEncoder().encode('PoolQA'),users=new Uint8Array(16+name.length),u=new DataView(users.buffer);
  u.setUint32(0,14,true);u.setUint16(4,1,true);u.setUint16(6,name.length,true);users.set(name,8);u.setUint32(8+name.length,1,true);u.setUint32(12+name.length,1,true);FS.writeFile('/saves/userdata/users.dat',users);
  const profile=new Uint8Array(4096),p=new DataView(profile.buffer);p.setUint32(0,12,true);p.setUint32(4,29,true);FS.writeFile('/saves/userdata/user1.dat',profile);
  await new Promise((r,j)=>FS.syncfs(false,e=>e?j(e):r()));
 });
 await page.locator('#start').click();await page.waitForTimeout(12000);await click(400,560);await page.waitForTimeout(4500);
 await click(260,348);await wait(()=>Module.canvas.width===1024);assert.ok(await api(8,1)>0);await api(4,1);await api(5,4);
 for(const [type,col,row] of [[16,4,2],[520,4,2],[16,4,3],[520,4,3],[520,4,1]])assert.ok(await api(1,type,col,row)>0);
 assert.ok(await api(2,0,5,2)>0);await api(4,0);
 await wait(()=>Array.from({length:8},(_,i)=>i).some(i=>Module._pvz_sandbox_plant_data(i,0)===16&&Module._pvz_sandbox_plant_data(i,2)===2&&Module._pvz_sandbox_plant_data(i,3)<300));
 await api(4,1);results.beingEaten=await plants();
 const hidden=results.beingEaten.find(p=>p[0]===520&&p[2]===2);assert.ok(hidden);assert.equal(hidden[4],1);assert.equal(hidden[3],300);await snap('pad-being-eaten');
 await api(4,0);
 await wait(()=>!Array.from({length:8},(_,i)=>i).some(i=>Module._pvz_sandbox_plant_data(i,0)===16&&Module._pvz_sandbox_plant_data(i,2)===2));
 // Native zombie updates happen after the mod tick; give the next tick time
 // to remove the unsupported flower, then check stability with enemies gone.
 await page.waitForTimeout(150);await api(6);await page.waitForTimeout(300);await api(4,1);
 results.afterPadLost=await plants();
 assert.equal(results.afterPadLost.some(p=>p[0]===520&&p[2]===2),expectOrphan);
 assert.ok(results.afterPadLost.some(p=>p[0]===520&&p[2]===3&&p[3]===300));
 assert.ok(results.afterPadLost.some(p=>p[0]===16&&p[2]===3&&p[3]===300));
 assert.ok(results.afterPadLost.some(p=>p[0]===520&&p[2]===1&&p[3]===300));
 assert.equal(results.afterPadLost.length,expectOrphan?4:3);await snap(expectOrphan?'baseline-orphan':'unsupported-flower-removed');
 await page.setViewportSize({width:844,height:390});await snap('phone-pool');
 assert.deepEqual(errors,[]);await writeFile(join(out,'report.json'),JSON.stringify({results,errors},null,2));console.log('Tucking pool QA passed',out,results);
}catch(e){await snap('failure');await writeFile(join(out,'failure.json'),JSON.stringify({error:String(e),results,errors,plants:await plants(),log:await page.evaluate(()=>window.pvzEngineLog)},null,2));throw e;}
finally{await browser.close();}
