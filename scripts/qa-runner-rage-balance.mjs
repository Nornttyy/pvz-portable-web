// Native WASM, isolated QA profile: paced movement, burst count and lane limits.
import assert from 'node:assert/strict';
import {mkdir,writeFile} from 'node:fs/promises';
import {join} from 'node:path';
const {chromium}=await import(process.env.PVZ_PLAYWRIGHT||'playwright-core');
const out=process.env.PVZ_QA_OUT||'/tmp/pvz-runner-rage-balance';await mkdir(out,{recursive:true});
const browser=await chromium.launch({executablePath:'/Applications/Google Chrome.app/Contents/MacOS/Google Chrome',headless:true,args:['--no-sandbox','--autoplay-policy=no-user-gesture-required']});
const page=await browser.newPage({viewport:{width:1100,height:750}}),results={},errors=[];page.on('pageerror',e=>errors.push(e.message));
const wait=(f,a,t=20000)=>page.waitForFunction(f,a,{timeout:t});
const api=(...a)=>page.evaluate(a=>Module._pvz_sandbox_command(...[...a,0,0,0,0].slice(0,4)),a);
const snap=n=>page.screenshot({path:join(out,n+'.png')});
async function click(x,y){const b=await page.locator('#canvas').boundingBox(),s=await page.evaluate(()=>[Module.canvas.width,Module.canvas.height]);await page.mouse.click(b.x+x*b.width/s[0],b.y+y*b.height/s[1]);await page.waitForTimeout(100);}
try{
 await page.goto(process.env.PVZ_QA_URL||'http://127.0.0.1:8097/');await wait(()=>!document.getElementById('start').disabled,undefined,90000);
 await page.evaluate(async()=>{const FS=Module.FS;if(!FS.analyzePath('/saves/userdata').exists)FS.mkdir('/saves/userdata');const name=new TextEncoder().encode('BalanceQA'),users=new Uint8Array(16+name.length),u=new DataView(users.buffer);u.setUint32(0,14,true);u.setUint16(4,1,true);u.setUint16(6,name.length,true);users.set(name,8);u.setUint32(8+name.length,1,true);u.setUint32(12+name.length,1,true);FS.writeFile('/saves/userdata/users.dat',users);const profile=new Uint8Array(4096),p=new DataView(profile.buffer);p.setUint32(0,12,true);p.setUint32(4,9,true);FS.writeFile('/saves/userdata/user1.dat',profile);await new Promise((r,j)=>FS.syncfs(false,e=>e?j(e):r()));});
 await page.locator('#start').click();await page.waitForTimeout(12000);await click(400,560);await page.waitForTimeout(4500);await click(719,27);await wait(()=>Module.canvas.width===1024);
 await api(7);await api(4,1);await api(5,1);await api(1,3,0,2);await api(1,3,6,2);await api(2,213,8,2);
 await page.evaluate(()=>{window.runnerTrace=[];const sample=()=>{const z=Module._pvz_sandbox_zombie_data;if(z(0,0)<0)return;window.runnerTrace.push({tick:z(0,10),x:z(0,2),phase:z(0,6)});requestAnimationFrame(sample);};sample();});
 await api(4,0);await wait(()=>Module._pvz_sandbox_zombie_data(0,0)===-1,undefined,6000);
 const runner=await page.evaluate(()=>window.runnerTrace);
 for(const [phase,speed] of [[1040,5.6],[1042,7.2]]){const trace=runner.filter(r=>r.phase===phase),first=trace[0],last=trace.at(-1);assert.ok(trace.length>10);const actual=Math.abs(last.x-first.x)/(last.tick-first.tick);assert.ok(Math.abs(actual-speed)<.08);results[phase===1040?'inboundSpeed':'outboundSpeed']=actual;}
 const brake=runner.filter(r=>r.phase===1041);assert.ok(brake.length>0&&brake.at(-1).tick-brake[0].tick<=8);results.runnerRoundTripTicks=runner.at(-1).tick-runner[0].tick;assert.ok(results.runnerRoundTripTicks<280);
 assert.deepEqual(await page.evaluate(()=>[0,1].map(i=>Module._pvz_sandbox_plant_data(i,3))),[4000,4000]);
 await writeFile(join(out,'runner-trace.json'),JSON.stringify(runner));
 for(const map of [0,1]){
  await api(8,map);await api(7);await api(4,1);if(map)await api(1,16,0,2);await api(1,500,0,2);
  const rows=map?6:5;for(let row=0;row<rows;++row)await api(2,214,8,row);
  const index=await page.evaluate(()=>{for(let i=0;i<10;++i)if(Module._pvz_sandbox_plant_data(i,0)===500)return i;return -1;});assert.ok(index>=0);
  await api(5,4);await api(4,0);await wait(i=>Module._pvz_sandbox_plant_data(i,5)===280,index);await api(4,1);await api(5,1);
  const before=await page.evaluate(rows=>Array.from({length:rows},(_,i)=>Module._pvz_sandbox_zombie_data(i,4)+Module._pvz_sandbox_zombie_data(i,5)),rows);
  await page.evaluate(index=>{window.burstTrace=[];window.burstDone=false;let ended=-1,started=false;const sample=()=>{const p=Module._pvz_sandbox_plant_data,z=Module._pvz_sandbox_zombie_data,q=Module._pvz_projectile_data,phase=p(index,4),tick=z(0,10),shots=[];started||=phase===1;if(started&&phase===0&&ended<0)ended=tick;for(let i=0;i<1000&&q(i,0)>=0;++i)if(q(i,0)>=304&&q(i,0)<=309)shots.push([q(i,0),q(i,5),q(i,4)]);window.burstTrace.push({tick,phase,remaining:p(index,7),shots});if(ended>=0&&tick-ended>=240){window.burstDone=true;return;}requestAnimationFrame(sample);};sample();},index);
  await api(4,0);await wait(i=>Module._pvz_sandbox_plant_data(i,4)===1,index);await page.waitForTimeout(650);await api(4,1);await snap(map?'pool-bounded-burst':'lawn-bounded-burst');await api(4,0);await wait(()=>window.burstDone);await api(4,1);
  const trace=await page.evaluate(()=>window.burstTrace),start=trace.find(t=>t.phase===1),end=trace.find(t=>t.tick>start.tick&&t.phase===0);
  assert.equal(start.remaining,40);assert.equal(end.remaining,0);assert.ok(end.tick-start.tick>=297&&end.tick-start.tick<=302);
  const bullets=trace.flatMap(t=>t.shots);assert.ok(bullets.length>100&&bullets.some(s=>s[2]<0)&&bullets.some(s=>s[2]>0));for(const [style,row] of bullets){assert.equal(style,306);assert.ok(row>=1&&row<=3);}
  const after=await page.evaluate(rows=>Array.from({length:rows},(_,i)=>Module._pvz_sandbox_zombie_data(i,4)+Module._pvz_sandbox_zombie_data(i,5)),rows);
  for(let row=0;row<rows;++row)if(Math.abs(row-2)>1)assert.equal(after[row],before[row]);assert.ok(after[2]<before[2]);
  results[map?'pool':'lawn']={shots:40,ticks:end.tick-start.tick,before,after,threeRowsOnly:true};await writeFile(join(out,map?'pool-trace.json':'lawn-trace.json'),JSON.stringify(trace));
 }
 assert.deepEqual(errors,[]);await writeFile(join(out,'report.json'),JSON.stringify({results,errors},null,2));console.log('Runner and rage balance QA passed',results);
}catch(e){await snap('failure');await writeFile(join(out,'failure.json'),JSON.stringify({error:String(e),results,errors,log:await page.evaluate(()=>window.pvzEngineLog)},null,2));throw e;}
finally{await browser.close();}
