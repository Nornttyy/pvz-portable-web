// Actual WASM combat in an isolated browser profile; no real player save writes.
import assert from 'node:assert/strict';
import {mkdir,writeFile} from 'node:fs/promises';
const {chromium}=await import(process.env.PVZ_PLAYWRIGHT||'playwright-core');
const out=process.env.PVZ_QA_OUT||'/tmp/pvz-sandbox-ranged';await mkdir(out,{recursive:true});
const browser=await chromium.launch({executablePath:'/Applications/Google Chrome.app/Contents/MacOS/Google Chrome',headless:true});
const page=await browser.newPage({viewport:{width:1100,height:750}}),errors=[],report={};page.on('pageerror',e=>errors.push(e.stack||e.message));
const api=(...a)=>page.evaluate(a=>Module._pvz_sandbox_command(...[...a,0,0,0].slice(0,4)),a);
async function data(name,fields){return page.evaluate(({name,fields})=>{const a=[],d=Module[name];for(let i=0;i<2048&&d(i,0)>=0;i++)a.push(fields.map(f=>d(i,f)));return a;},{name,fields});}
const plants=()=>data('_pvz_sandbox_plant_data',[0,1,2,3,13,14,16]);
const zombies=()=>data('_pvz_sandbox_zombie_data',[0,1,2,4,5,30]);
const shots=()=>data('_pvz_projectile_data',[0,1,2,3,4,5,6,8,9,10,11]);
async function click(x,y){const b=await page.locator('#canvas').boundingBox(),s=await page.evaluate(()=>[Module.canvas.width,Module.canvas.height]);await page.mouse.click(b.x+x*b.width/s[0],b.y+y*b.height/s[1]);await page.waitForTimeout(100);}
const snap=async name=>{await page.mouse.move(0,0);await page.screenshot({path:out+'/'+name+'.png'});};
const run=async ms=>{await api(4,0);await page.waitForTimeout(ms);await api(4,1);};
async function reset(){await api(25,0);await api(4,1);await api(5,4);await api(28,0);await api(29,0);}
async function sample(ms=3500){const found=[];await api(4,0);for(let n=0;n<ms;n+=100){await page.waitForTimeout(100);found.push(...await shots());}await api(4,1);return found;}
function moves(samples,direction,label){
 assert.ok(samples.length,label+' fires');assert.ok(samples.every(s=>s[9]===direction),label+' facing');
 const history=new Map();let compared=0;
 for(const s of samples){const prev=history.get(s[10]);if(prev&&s[6]>prev[6]){assert.ok((s[1]-prev[1])*direction>=0,label+' trajectory');++compared;}history.set(s[10],s);}
 assert.ok(compared>0,label+' live movement observed');
}
try{
 await page.goto(process.env.PVZ_QA_URL||'http://127.0.0.1:8097/');await page.waitForFunction(()=>!document.getElementById('start').disabled,undefined,{timeout:90000});
 await page.evaluate(async()=>{const FS=Module.FS;if(!FS.analyzePath('/saves/userdata').exists)FS.mkdir('/saves/userdata');const name=new TextEncoder().encode('RangedQA'),users=new Uint8Array(16+name.length),u=new DataView(users.buffer);u.setUint32(0,14,true);u.setUint16(4,1,true);u.setUint16(6,name.length,true);users.set(name,8);u.setUint32(8+name.length,1,true);u.setUint32(12+name.length,1,true);FS.writeFile('/saves/userdata/users.dat',users);const profile=new Uint8Array(4096),p=new DataView(profile.buffer);p.setUint32(0,12,true);p.setUint32(4,49,true);FS.writeFile('/saves/userdata/user1.dat',profile);await new Promise((r,j)=>FS.syncfs(false,e=>e?j(e):r()));});
 await page.locator('#start').click();await page.waitForTimeout(12000);await click(400,560);await page.waitForTimeout(4500);await click(719,27);await page.waitForFunction(()=>Module.canvas.width===1024);
 report.plants=[];
 if(!process.env.PVZ_QA_ZOMBIES_ONLY){
 for(const type of [5,519,521,522,524,530,32,34,39,44]){
  await reset();await api(27,23,1,2);await api(28,1);await api(27,type,6,2);
  const s=await sample();moves(s,-1,'charmed plant '+type);report.plants.push({type,samples:s.length});await snap('plant-'+type);
 }
 // Backward muzzle: native split pea has both front and rear guns.
 await reset();await api(27,23,0,2);await api(27,23,8,2);await api(28,1);await api(27,28,4,2);const split=await sample();assert.ok(split.some(s=>s[9]===-1)&&split.some(s=>s[9]===1),'split pea retains two opposite guns');await snap('split-pea');
 // Fume effect uses the same reflected axis as the mouth and damage rectangle.
 await reset();await api(27,23,2,2);await api(28,1);await api(27,527,5,2);await run(2700);assert.ok((await plants()).find(p=>p[1]===2)[3]<8000,'leftward fume damage');await snap('fume');
 // Fire peas converted on the way keep their tails behind the projectile.
 await reset();await api(27,23,0,2);await api(28,1);await api(27,18,6,2);await api(27,22,4,2);const fire=await sample(4200);assert.ok(fire.some(s=>s[7]===6&&s[9]===-1),'leftward torchwood conversion');await snap('fire-peas');
 }
 report.catapults=[];
 for(const friendly of [false,true]){
  await reset();await api(28,friendly?0:1);await api(27,23,friendly?4:3,2);await api(28,friendly?1:0);await api(27,23,friendly?7:0,2);
  await api(29,+friendly);await api(2,22,friendly?1:6,2);const s=await sample(6000);moves(s,friendly?1:-1,'catapult '+friendly);
  const p=await plants();assert.equal(p.find(p=>p[1]===(friendly?4:3))[3],8000,'catapult ignores allied plant');assert.ok(p.find(p=>p[1]===(friendly?7:0))[3]<8000,'catapult hits enemy plant');report.catapults.push({friendly,plants:p,shots:s});await snap('catapult-'+friendly);
 }
 // Without any enemy plants, charmed catapults must still shoot enemy zombies.
 await reset();await api(29,1);await api(2,22,1,2);await api(29,0);await api(2,23,8,2);const before=(await zombies()).find(z=>z[0]===23)[3];await sample(6000);const giant=(await zombies()).find(z=>z[0]===23);assert.ok(giant&&giant[3]<before,'friendly catapult hits opposing zombie');await snap('catapult-zombie-target');
 // Pea-head and gatling-head bullets leave the actual mirrored mouth.
 for(const type of [26,29])for(const friendly of [false,true]){
  await reset();await api(28,+friendly);await api(27,23,friendly?7:0,2);await api(29,+friendly);await api(2,type,friendly?2:6,2);
  const s=await sample(2500);moves(s,friendly?1:-1,'zombie shooter '+type+'/'+friendly);assert.ok((await plants())[0][3]<8000,'zombie ranged hits opposing plant');await snap('zombie-'+type+'-'+friendly);
 }
 assert.deepEqual(errors,[]);await writeFile(out+'/report.json',JSON.stringify(report,null,2));console.log('Sandbox ranged direction QA passed');
}catch(e){await snap('failure').catch(()=>{});await writeFile(out+'/failure.json',JSON.stringify({error:e.stack,errors,report,plants:await plants().catch(()=>[]),zombies:await zombies().catch(()=>[]),shots:await shots().catch(()=>[])},null,2));throw e;}finally{await browser.close();}
