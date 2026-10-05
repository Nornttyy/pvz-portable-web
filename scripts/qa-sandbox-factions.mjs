// All writes are confined to a disposable Chromium profile, never player saves.
import assert from 'node:assert/strict';
import {mkdir,writeFile} from 'node:fs/promises';
const {chromium}=await import(process.env.PVZ_PLAYWRIGHT||'playwright-core');
const out=process.env.PVZ_QA_OUT||'/tmp/pvz-sandbox-factions';await mkdir(out,{recursive:true});
const browser=await chromium.launch({executablePath:'/Applications/Google Chrome.app/Contents/MacOS/Google Chrome',headless:true});
const page=await browser.newPage({viewport:{width:1100,height:750}}),errors=[],report={};page.on('pageerror',e=>errors.push(e.stack||e.message));
const api=(...a)=>page.evaluate(a=>Module._pvz_sandbox_command(...[...a,0,0,0].slice(0,4)),a);
const plants=()=>page.evaluate(()=>{const a=[],d=Module._pvz_sandbox_plant_data;for(let i=0;i<1024&&d(i,0)>=0;i++)a.push([0,1,2,3,12,16].map(f=>d(i,f)));return a;});
const zombies=()=>page.evaluate(()=>{const a=[],d=Module._pvz_sandbox_zombie_data;for(let i=0;i<1024&&d(i,0)>=0;i++)a.push([0,1,2,4,18,30].map(f=>d(i,f)));return a;});
async function click(x,y){const b=await page.locator('#canvas').boundingBox(),s=await page.evaluate(()=>[Module.canvas.width,Module.canvas.height]);await page.mouse.click(b.x+x*b.width/s[0],b.y+y*b.height/s[1]);await page.waitForTimeout(100);}
const snap=async name=>{await page.mouse.move(0,0);await page.screenshot({path:out+'/'+name+'.png'});};
const run=async ms=>{await api(4,0);await page.waitForTimeout(ms);await api(4,1);};
async function reset(){await api(25,0);await api(4,1);await api(5,4);await api(28,0);await api(29,0);}
try{
 await page.goto(process.env.PVZ_QA_URL||'http://127.0.0.1:8097/');await page.waitForFunction(()=>!document.getElementById('start').disabled,undefined,{timeout:90000});
 await page.evaluate(async()=>{const FS=Module.FS;if(!FS.analyzePath('/saves/userdata').exists)FS.mkdir('/saves/userdata');const name=new TextEncoder().encode('FactionQA'),users=new Uint8Array(16+name.length),u=new DataView(users.buffer);u.setUint32(0,14,true);u.setUint16(4,1,true);u.setUint16(6,name.length,true);users.set(name,8);u.setUint32(8+name.length,1,true);u.setUint32(12+name.length,1,true);FS.writeFile('/saves/userdata/users.dat',users);const profile=new Uint8Array(4096),p=new DataView(profile.buffer);p.setUint32(0,12,true);p.setUint32(4,49,true);FS.writeFile('/saves/userdata/user1.dat',profile);await new Promise((r,j)=>FS.syncfs(false,e=>e?j(e):r()));});
 await page.locator('#start').click();await page.waitForTimeout(12000);await click(400,560);await page.waitForTimeout(4500);await click(719,27);await page.waitForFunction(()=>Module.canvas.width===1024);
 if(!process.env.PVZ_QA_ROSTER_ONLY){
 await reset();await click(603,24);await click(193,535);assert.ok((await api(0))&128,'visible plant charm control');
 await api(1,501,1,2);assert.equal((await plants())[0][5],1);await snap('plant-faction-controls');
 await click(688,24);await click(193,535);assert.ok((await api(0))&256,'visible zombie charm control');await snap('zombie-faction-controls');
 // Native snow peas travel left and hit a real opposing plant; an enemy zombie ignores its own team.
 await reset();await api(1,501,1,2);await api(28,1);await api(1,5,5,2);await api(2,0,7,2);
 const before=await plants();await run(5500);const after=await plants();report.hostileShots=after;
 assert.ok(after.find(p=>p[1]===1)[3]<before[0][3],'charmed plant shoots normal plant');assert.equal(after.find(p=>p[1]===5)[3],300,'same-team zombie does not bite');await snap('hostile-plant-combat');
 // Reverse direction: normal plant fires into a charmed wallnut.
 await reset();await api(1,5,1,2);await api(28,1);await api(1,501,5,2);await run(4200);assert.ok((await plants()).find(p=>p[1]===5)[3]<4000,'normal plant damages charmed plant');
 // A friendly zombie walks right, eats opposing plants, and ignores ordinary ones.
 await reset();await api(1,501,2,2);await api(28,1);await api(1,23,4,2);await api(29,1);await api(2,0,2,2);const zx=(await zombies())[0][2];await run(7500);
 report.friendlyZombie={plants:await plants(),zombies:await zombies()};assert.ok(report.friendlyZombie.zombies[0][2]>zx,'friendly zombie moves right');assert.equal(report.friendlyZombie.plants.find(p=>p[1]===2)[3],4000);assert.ok(report.friendlyZombie.plants.find(p=>p[1]===4)[3]<8000,'friendly zombie eats charmed plant');
 // Opposing zombie teams fight each other, normal plants never shoot allied zombies.
 await reset();await api(1,5,0,1);await api(29,1);await api(2,0,2,1);await run(1500);assert.equal((await zombies())[0][3],270,'friendly snow pea ignores ally');await api(29,0);await api(2,0,3,1);await run(3000);assert.ok((await zombies()).some(z=>z[3]<270),'opposing zombies fight');
 // Both coin shooters and the randomized shooter must target real plants too.
 for(const type of [530,529,500,521]){await reset();await api(1,501,1,2);await api(28,1);await api(1,type,5,2);await run(6500);const ps=await plants();assert.ok(!ps.some(p=>p[1]===1)||ps.find(p=>p[1]===1)[3]<4000,'custom shooter '+type+' hits opposing plant');}
 // Lobbed native shots, penetrating fumes, directional triple shots and homing.
 for(const type of [32,34,39,44,43,18,527]){await reset();await api(27,501,type===527?2:1,2);await api(28,1);assert.equal(await api(27,type,5,2),1);await run(6500);const ps=await plants();assert.ok(!ps.some(p=>p[0]===501)||ps.find(p=>p[0]===501)[3]<4000,'special shooter '+type+' hits opposing plant');}
 // Cherry bombs affect only enemies, including real plant targets.
 await reset();await api(1,501,3,2);await api(28,1);await api(1,501,5,2);await api(1,2,4,2);await run(1000);assert.equal((await plants()).find(p=>p[1]===3)[3],2200);assert.equal((await plants()).find(p=>p[1]===5)[3],4000);
 await reset();await api(1,501,0,2);await api(28,1);await api(1,23,2,2);await api(1,528,4,2);await api(2,23,8,2);await api(29,1);await api(2,23,6,2);
 // Survivable targets, sampled at impact: a bucket can die to the 1200 hit
 // plus the opposing zombie before a fixed wall-clock delay finishes.
 await api(4,0);await page.waitForFunction(()=>[0,1].some(i=>Module._pvz_sandbox_zombie_data(i,30)===1&&Module._pvz_sandbox_zombie_data(i,17)>0),undefined,{timeout:5000,polling:20});await api(4,1);
 assert.equal((await plants()).find(p=>p[0]===501)[3],2800,'hostile ice chili damages ordinary plant');assert.equal((await plants()).find(p=>p[0]===23)[3],8000,'ice chili protects allied plant');
 const frozen=await page.evaluate(()=>[0,1].map(i=>[17,30].map(f=>Module._pvz_sandbox_zombie_data(i,f))));assert.ok(frozen.some(z=>z[1]===1&&z[0]>0),'hostile ice freezes charmed zombie');assert.ok(frozen.some(z=>z[1]===0&&z[0]===0),'ice does not freeze own zombie');
 await reset();await api(1,501,0,4);await api(28,1);await api(1,23,8,4);await api(1,526,4,2);await run(3000);assert.ok(!(await plants()).some(p=>p[0]===501),'nuke pulses keep source team after caster dies');assert.equal((await plants()).find(p=>p[0]===23)[3],8000);
 // Persistent orbit coins attack opposing plants but retain their stock.
 await reset();await api(1,501,3,2);await api(28,1);await api(1,531,4,2);await run(8500);assert.ok(!(await plants()).some(p=>p[0]===501)||(await plants()).find(p=>p[0]===501)[3]<4000);await snap('charmed-coin-flower');
 }
 // Every native roster type accepts charm; linked sled followers inherit it.
 report.roster=[];
 for(let type=0;type<=32;type++){await reset();await api(29,1);assert.equal(await api(2,type,5,2),1);assert.ok((await zombies()).every(z=>z[5]===1),'native charm '+type);await run(type===25?9000:100);if(type===25){assert.ok((await zombies()).length>1,'boss summons');assert.ok((await zombies()).every(z=>z[5]===1),'boss summons inherit charm');await snap('charmed-boss');}report.roster.push(type);}
 await reset();await api(1,501,1,1);await api(28,1);await api(1,5,6,1);const savedBefore=await plants();
 for(const map of [1,2,3,4,5,0]){await api(8,map);for(const original of savedBefore)assert.equal((await plants()).find(p=>p[4]===original[4])[5],original[5]);}
 await page.evaluate(()=>window.dispatchEvent(new CustomEvent('pvz-sandbox-action',{detail:1})));const layout=await page.evaluate(()=>JSON.parse(localStorage.getItem('pvz.sandbox.formation.v1')));assert.ok(layout.plants.some(p=>p.charmed));
 await page.evaluate(()=>window.dispatchEvent(new CustomEvent('pvz-sandbox-action',{detail:2})));assert.equal((await plants()).find(p=>p[0]===5)[5],1);assert.equal((await plants()).find(p=>p[0]===501)[5],0);await snap('restored-teams');
 assert.deepEqual(errors,[]);await writeFile(out+'/report.json',JSON.stringify(report,null,2));console.log('Sandbox factions browser QA passed');
}catch(e){await snap('failure').catch(()=>{});await writeFile(out+'/failure.json',JSON.stringify({error:e.stack,errors,report,plants:await plants().catch(()=>[]),zombies:await zombies().catch(()=>[])},null,2));throw e;}finally{await browser.close();}
