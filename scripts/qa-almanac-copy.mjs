// View all twenty-one original entries in the real game with an isolated save.
import assert from 'node:assert/strict';
import {mkdir,writeFile} from 'node:fs/promises';
import {join} from 'node:path';
const {chromium}=await import(process.env.PVZ_PLAYWRIGHT||'playwright-core');
const out=process.env.PVZ_QA_OUT||'/tmp/pvz-almanac-copy';await mkdir(out,{recursive:true});
const browser=await chromium.launch({executablePath:'/Applications/Google Chrome.app/Contents/MacOS/Google Chrome',headless:true,args:['--no-sandbox']});
const page=await browser.newPage({viewport:{width:1100,height:750},hasTouch:true}),errors=[],pages=[];page.on('pageerror',e=>errors.push(e.message));
async function click(x,y){const b=await page.locator('#canvas').boundingBox(),s=await page.evaluate(()=>[Module.canvas.width,Module.canvas.height]);await page.mouse.click(b.x+x*b.width/s[0],b.y+y*b.height/s[1]);await page.waitForTimeout(220);}
async function tap(x,y){const b=await page.locator('#canvas').boundingBox();await page.touchscreen.tap(b.x+x*b.width/800,b.y+y*b.height/600);await page.waitForTimeout(220);}
const ui=(field,seed=0)=>page.evaluate(([f,s])=>Module._pvz_almanac_data(f,s),[field,seed]);
const extras=[52,53,51,49];
async function plantCard(seed){
 const target=extras.includes(seed)?1:0;
 if(await ui(0)!==target)await click(target?408:280,580);
 const slot=target?extras.indexOf(seed):seed;
 await click(51+slot%8*52,127+Math.floor(slot/8)*78);
 assert.equal(await ui(1),seed,'selected the exact visible card');
}
const snap=name=>page.screenshot({path:join(out,name+'.png')});
async function phoneSnap(name){
 await page.setViewportSize({width:844,height:390});await page.waitForTimeout(250);await snap(name+'-phone');
 await page.setViewportSize({width:1100,height:750});await page.waitForTimeout(250);
}
try{
 await page.goto(process.env.PVZ_QA_URL||'http://127.0.0.1:8097/');await page.waitForFunction(()=>!document.getElementById('start').disabled,undefined,{timeout:90000});
 await page.evaluate(async()=>{
  const FS=Module.FS;if(!FS.analyzePath('/saves/userdata').exists)FS.mkdir('/saves/userdata');
  const name=new TextEncoder().encode('AlmanacQA'),users=new Uint8Array(16+name.length),u=new DataView(users.buffer);
  u.setUint32(0,14,true);u.setUint16(4,1,true);u.setUint16(6,name.length,true);users.set(name,8);u.setUint32(8+name.length,1,true);u.setUint32(12+name.length,1,true);FS.writeFile('/saves/userdata/users.dat',users);
  const profile=new Uint8Array(4096),p=new DataView(profile.buffer);p.setUint32(0,12,true);p.setUint32(4,37,true);
  p.setUint32(416,1,true); // First native store purchase: Gatling upgrade.
  FS.writeFile('/saves/userdata/user1.dat',profile);await new Promise((r,j)=>FS.syncfs(false,e=>e?j(e):r()));
 });
 await page.locator('#start').click();await page.waitForTimeout(12000);await click(400,560);await page.waitForTimeout(4500);
 await click(370,455);await click(208,366);
 assert.equal(await ui(2),2);assert.equal(await ui(0),0);await snap('aligned-first-page');
 for(let bookPage=0;bookPage<2;++bookPage){
  if(await ui(0)!==bookPage)await click(bookPage?408:280,580);
  for(let seed=0;seed<54;++seed)if(seed!==48&&await ui(3,seed))await plantCard(seed);
 }
 await snap('aligned-extra-page');const selected=await ui(1);
 await click(415,517);assert.equal(await ui(1),selected,'hidden first-page card cannot be selected');
 await click(408,580);assert.equal(await ui(0),1,'last-page arrow does not wrap');
 await page.keyboard.press('ArrowLeft');await page.waitForFunction(()=>Module._pvz_almanac_data(0,0)===0);
 await click(280,580);assert.equal(await ui(0),0,'first-page arrow does not wrap');
 await page.keyboard.press('ArrowRight');await page.waitForFunction(()=>Module._pvz_almanac_data(0,0)===1);
 for(const [id,base]of [[500,0],[501,3],[519,52],[520,1],[521,7],[522,40],[523,53],[524,26],[525,8],[526,15],[527,10],[528,51],[529,49]]){
  await plantCard(base);await snap('plant-'+id);pages.push(id);
  if(id===500||id===524)await phoneSnap('plant-'+id);
 }
 for(const [name,width,height]of [['landscape',844,390],['portrait',390,844]]){
  await page.setViewportSize({width,height});await page.waitForTimeout(300);
  await tap(280,580);assert.equal(await ui(0),0);await tap(415,127);assert.equal(await ui(1),7);
  await tap(408,580);assert.equal(await ui(0),1);await tap(207,127);assert.equal(await ui(1),49);
  await snap('touch-'+name);
 }
 await page.setViewportSize({width:1100,height:750});await page.waitForTimeout(250);
 await click(110,580);await click(208,366);assert.equal(await ui(0),1,'return from index preserves selected extra card');
 await click(110,580);await click(590,366);
 for(let i=0;i<8;++i){const slot=26+i;await click(53+slot%6*71,117+Math.floor(slot/6)*80);await snap('zombie-'+(212+i));pages.push(212+i);if(i===4||i===5)await phoneSnap('zombie-'+(212+i));}
 await page.setViewportSize({width:844,height:390});await page.waitForTimeout(500);await snap('phone');
 assert.deepEqual(errors,[]);assert.equal(pages.length,21);
 await writeFile(join(out,'report.json'),JSON.stringify({pages,errors},null,2));console.log('All twenty-one almanac pages captured',out);
}catch(e){await snap('failure');throw e;}
finally{await browser.close();}
