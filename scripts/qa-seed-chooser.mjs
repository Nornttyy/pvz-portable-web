// Isolated profiles only; exercise real selection, return, touch and adventure.
import assert from 'node:assert/strict';
import {mkdir,writeFile} from 'node:fs/promises';
import {join} from 'node:path';
const {chromium}=await import(process.env.PVZ_PLAYWRIGHT||'playwright-core');
const out=process.env.PVZ_QA_OUT||'/tmp/pvz-seed-chooser-qa';await mkdir(out,{recursive:true});
const browser=await chromium.launch({executablePath:process.env.PVZ_CHROME||'/Applications/Google Chrome.app/Contents/MacOS/Google Chrome',headless:true,args:['--no-sandbox']});
async function scenario(level){
 const context=await browser.newContext({viewport:{width:1100,height:750},hasTouch:true}),page=await context.newPage(),errors=[];
 page.on('pageerror',e=>errors.push(e.message));
 const full=level===49,baseY=full?158:163,spacing=full?70:73;
 async function click(x,y,touch=false){const b=await page.locator('#canvas').boundingBox(),s=await page.evaluate(()=>[Module.canvas.width,Module.canvas.height]),p=[b.x+x*b.width/s[0],b.y+y*b.height/s[1]];if(touch)await page.touchscreen.tap(...p);else await page.mouse.click(...p);await page.waitForTimeout(350);}
 async function card(id,touch=false){const slot=id===52?8:id>=8?id+1:id;await click(47+slot%9*53,baseY+Math.floor(slot/9)*spacing,touch);}
 async function shot(name){await page.mouse.move(0,0);await page.waitForTimeout(150);await page.screenshot({path:join(out,`${level}-${name}.png`)});}
 async function cardImage(){await page.mouse.move(0,0);await page.waitForTimeout(200);const b=await page.locator('#canvas').boundingBox();return page.screenshot({clip:{x:b.x+446*b.width/800,y:b.y+(baseY-35)*b.height/600,width:50*b.width/800,height:70*b.height/600}});}
 try{
  await page.goto(process.env.PVZ_QA_URL||'http://127.0.0.1:8097/');await page.waitForFunction(()=>!document.getElementById('start').disabled,undefined,{timeout:90000});
  await page.evaluate(async({level,full})=>{
   const FS=Module.FS;if(!FS.analyzePath('/saves/userdata').exists)FS.mkdir('/saves/userdata');
   const name=new TextEncoder().encode('ChooserQA'),users=new Uint8Array(16+name.length),u=new DataView(users.buffer);
   u.setUint32(0,14,true);u.setUint16(4,1,true);u.setUint16(6,name.length,true);users.set(name,8);u.setUint32(8+name.length,1,true);u.setUint32(12+name.length,1,true);FS.writeFile('/saves/userdata/users.dat',users);
   const profile=new Uint8Array(4096),p=new DataView(profile.buffer);p.setUint32(0,12,true);p.setUint32(4,level,true);
   if(full)for(let item=0;item<9;++item)p.setUint32(416+item*4,1,true);
   FS.writeFile('/saves/userdata/user1.dat',profile);await new Promise((r,j)=>FS.syncfs(false,e=>e?j(e):r()));
  },{level,full});
  await page.locator('#start').click();await page.waitForTimeout(12000);await click(400,560);await page.waitForTimeout(4500);await click(560,135);await page.waitForTimeout(15000);
  await shot('all-cards-inside-panel');
  const before=await cardImage();await card(52);assert.notDeepEqual(await cardImage(),before,'selected card leaves a dim placeholder');await shot('selected-with-grid-placeholder');await click(110,43);assert.deepEqual(await cardImage(),before,'cancel restores the exact card at its grid position');await shot('returned-to-grid');
  await page.setViewportSize({width:844,height:390});await page.waitForTimeout(300);
  const mobileBefore=await cardImage();await card(52,true);assert.notDeepEqual(await cardImage(),mobileBefore);await click(110,43,true);assert.deepEqual(await cardImage(),mobileBefore);await card(52,true);await shot('mobile-select-return-reselect');
  await page.setViewportSize({width:1100,height:750});await page.waitForTimeout(300);
  const choices=full?[1,33,34,47,26]:[0,1,2,3,5];for(const id of choices)await card(id);
  await shot('six-selected');await click(258,566);
  await page.waitForFunction(()=>Module._pvz_adventure_power_data(-1,5)===1,undefined,{timeout:25000});
  const bank=await page.evaluate(()=>Array.from({length:6},(_,i)=>Module._pvz_adventure_seed_data(i,0)));
  assert.deepEqual(bank,full?[504,503,33,512,47,508]:[504,500,503,2,501,505]);
  assert.deepEqual(errors,[]);await shot('adventure-started');
  const result={level,full,bank,mouseReturn:true,touchReturn:true,errors};await writeFile(join(out,`${level}-report.json`),JSON.stringify(result,null,2));console.log('Chooser passed',result);return result;
 }catch(e){await shot('failure');await writeFile(join(out,`${level}-failure.json`),JSON.stringify({error:String(e),errors,logs:await page.evaluate(()=>window.pvzEngineLog)},null,2));throw e;}
 finally{await context.close();}
}
try{const results=await Promise.all([scenario(8),scenario(49)]);await writeFile(join(out,'report.json'),JSON.stringify(results,null,2));}finally{await browser.close();}
