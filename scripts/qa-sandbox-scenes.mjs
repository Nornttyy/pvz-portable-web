// All mutations are confined to a disposable headless browser profile.
import assert from 'node:assert/strict';
import {mkdir,writeFile} from 'node:fs/promises';
const {chromium}=await import(process.env.PVZ_PLAYWRIGHT||'playwright-core');
const out=process.env.PVZ_QA_OUT||'/tmp/pvz-sandbox-scenes';await mkdir(out,{recursive:true});
const browser=await chromium.launch({executablePath:'/Applications/Google Chrome.app/Contents/MacOS/Google Chrome',headless:true});
const page=await browser.newPage({viewport:{width:1100,height:750}}),errors=[],report={};page.on('pageerror',e=>errors.push(e.stack||e.message));
const api=(...a)=>page.evaluate(a=>Module._pvz_sandbox_command(...[...a,0,0,0].slice(0,4)),a);
async function click(x,y){const b=await page.locator('#canvas').boundingBox(),s=await page.evaluate(()=>[Module.canvas.width,Module.canvas.height]);await page.mouse.click(b.x+x*b.width/s[0],b.y+y*b.height/s[1]);await page.waitForTimeout(160);}
const snap=async name=>{await page.mouse.move(0,0);await page.screenshot({path:out+'/'+name+'.png'});};
const plants=()=>page.evaluate(()=>{const result=[];for(let i=0;i<1100;++i){const d=Module._pvz_sandbox_plant_data;if(d(i,0)<0)break;result.push([0,1,2,3,12,13,14,15].map(f=>d(i,f)));}return result;});
const coins=()=>page.evaluate(()=>{const d=Module._pvz_coinplant_data;return [0,1,2,3,4,5].map(f=>d(2,0,f));});
try{
 await page.goto(process.env.PVZ_QA_URL||'http://127.0.0.1:8097/');await page.waitForFunction(()=>!document.getElementById('start').disabled,undefined,{timeout:90000});
 await page.evaluate(async()=>{const FS=Module.FS;if(!FS.analyzePath('/saves/userdata').exists)FS.mkdir('/saves/userdata');const name=new TextEncoder().encode('SceneQA'),users=new Uint8Array(16+name.length),u=new DataView(users.buffer);u.setUint32(0,14,true);u.setUint16(4,1,true);u.setUint16(6,name.length,true);users.set(name,8);u.setUint32(8+name.length,1,true);u.setUint32(12+name.length,1,true);FS.writeFile('/saves/userdata/users.dat',users);const profile=new Uint8Array(4096),p=new DataView(profile.buffer);p.setUint32(0,12,true);p.setUint32(4,49,true);FS.writeFile('/saves/userdata/user1.dat',profile);await new Promise((r,j)=>FS.syncfs(false,e=>e?j(e):r()));});
 await page.locator('#start').click();await page.waitForTimeout(12000);await click(400,560);await page.waitForTimeout(4500);await click(719,27);await page.waitForFunction(()=>Module.canvas.width===1024);
 await api(4,1);assert.equal(await api(1,531,1,2),1);assert.equal(await api(1,520,0,3),1);assert.equal(await api(1,501,2,4),1);
 await api(5,4);await api(4,0);await page.waitForFunction(()=>Module._pvz_coinplant_data(2,0,4)>=2,undefined,{timeout:20000});await api(4,1);
 const original=await plants(),stock=await coins();await click(878,61);await snap('map-thumbnails');await click(644,570);
 report.maps=[];
 for(const map of [1,2,3,4,5,0,1]){
  assert.equal(await api(8,map),1);assert.equal(await api(24),map);await page.waitForTimeout(200);const current=await plants();
  for(const p of original){const q=current.find(q=>q[4]===p[4]);assert.ok(q,'original instance survives switch');assert.equal(q[0],p[0]);assert.equal(q[3],p[3]);assert.equal(q[2],p[2]);}
  assert.deepEqual(await coins(),stock,'money stock, production clock and phase preserved');await snap('map-'+map);report.maps.push({map,plants:current.length});
 }
 assert.equal(await api(1,523,4,5),1);const sixth=(await plants()).find(p=>p[0]===523);
 await api(8,4);assert.equal((await plants()).find(p=>p[4]===sixth[4])[2],4);await api(8,3);assert.equal((await plants()).find(p=>p[4]===sixth[4])[2],5);
 // An actual click in the roof's raised left columns must hit the shown cell.
 await api(8,4);assert.equal(await api(1,33,2,1),1);await click(104,40);await click(224+40+2*80+40,70+3*20+1*85+40);
 assert.ok((await plants()).some(p=>p[0]===500&&p[1]===2&&p[2]===1),'roof mouse placement matches the sloped grid');
 await api(8,1);await api(6);
 for(const [type,col] of [[0,4],[7,5],[23,6],[219,7]])assert.equal(await api(2,type,col,2),1);
 await api(4,0);await page.waitForTimeout(400);await api(4,1);
 report.swimmers=await page.evaluate(()=>Array.from({length:4},(_,i)=>[0,1,21].map(f=>Module._pvz_sandbox_zombie_data(i,f))));assert.ok(report.swimmers.every(z=>z[2]===1));await snap('automatic-swim-rings');
 await api(6);await click(973,61);assert.equal(await api(10),6,'standalone every-lane button includes water rows');await snap('every-lane-button');
 await page.setViewportSize({width:844,height:390});await page.waitForTimeout(600);await click(878,61);await snap('phone-map-picker');await click(644,570);await page.setViewportSize({width:1100,height:750});await page.waitForTimeout(500);
 await page.evaluate(()=>window.dispatchEvent(new CustomEvent('pvz-sandbox-action',{detail:1})));
 const saved=await page.evaluate(()=>JSON.parse(localStorage.getItem('pvz.sandbox.formation.v1')));assert.equal(saved.map,1);assert.equal(saved.adapted,true);report.formationCount=saved.plants.length;
 await page.evaluate(()=>window.dispatchEvent(new CustomEvent('pvz-sandbox-action',{detail:2})));assert.equal((await plants()).length,report.formationCount);
 // All 33 native types remain placeable, including their linked/large rigs.
 await api(25,1);await api(4,1);report.roster=[];
 for(let type=0;type<=32;++type){
  await api(6);assert.equal(await api(2,type,5,2),1,'native type '+type);
  const before=await api(10);assert.equal(before,type===13?4:1,'bobsled team stays linked');
  if(type===13){const xs=await page.evaluate(()=>[0,1,2,3].map(i=>Module._pvz_sandbox_zombie_data(i,2)).sort((a,b)=>a-b));assert.deepEqual(xs,[450,500,550,600]);}
  await api(4,0);await page.waitForTimeout(type===25?4500:160);await api(4,1);assert.equal(await api(0)&1,1);
  if([11,14].includes(type))assert.equal(await page.evaluate(()=>Module._pvz_sandbox_zombie_data(0,21)),1,'native aquatic enters water immediately');
  if(type===25)await snap('boss-in-sandbox');
  report.roster.push(type);
 }
 await api(6);await api(2,11,4,2);await api(2,14,6,3);await api(8,4);
 assert.deepEqual(await page.evaluate(()=>[0,1].map(i=>Module._pvz_sandbox_zombie_data(i,21))),[0,0],'aquatics leave water on roof switch');
 // Catalogue page two and press-and-hold use the real canvas controls.
 await api(25,0);await click(688,24);await click(220,572);await snap('zombie-catalogue-page-two');await click(44,572);await click(30,146);await api(20,1);
 {const b=await page.locator('#canvas').boundingBox();await page.mouse.move(b.x+(224+600)*b.width/1024,b.y+220*b.height/600);await page.mouse.down();await page.waitForTimeout(650);await snap('placement-combo');await page.mouse.up();}
 report.heldCount=await api(10);assert.ok(report.heldCount>=5,'hold places rapidly without a game-speed dependency');
 await api(25,0);await api(19,1);
 assert.equal(await page.evaluate(()=>{let n=0;for(let i=0;i<180;++i)if(Module._pvz_sandbox_command(1,523,3,2)>0)n++;return n;}),180);
 await api(8,1);assert.equal(await api(9),181);
 await page.evaluate(()=>window.dispatchEvent(new CustomEvent('pvz-sandbox-action',{detail:1})));
 assert.equal(await page.evaluate(()=>JSON.parse(localStorage.getItem('pvz.sandbox.formation.v1')).plants.length),181,'automatic lily is included beyond the old snapshot boundary');
 await page.evaluate(()=>window.dispatchEvent(new CustomEvent('pvz-sandbox-action',{detail:2})));assert.equal(await api(9),181,'adapted full formation restores every plant');
 assert.deepEqual(errors,[]);await writeFile(out+'/report.json',JSON.stringify(report,null,2));console.log('Sandbox scenes browser QA passed',JSON.stringify(report));
}catch(e){await snap('failure').catch(()=>{});await writeFile(out+'/failure.json',JSON.stringify({error:e.stack,errors,report,plants:await plants().catch(()=>[])},null,2));throw e;}finally{await browser.close();}
