// Exercise the retired menu area with actual mouse/touch input, then confirm
// the neighboring menu and reconstructed selector still work normally.
import assert from 'node:assert/strict';
import {mkdir,writeFile} from 'node:fs/promises';
import {join} from 'node:path';
const {chromium}=await import(process.env.PVZ_PLAYWRIGHT||'playwright-core');
const out=process.env.PVZ_QA_OUT||'/tmp/pvz-zombatar-removed';await mkdir(out,{recursive:true});
const browser=await chromium.launch({executablePath:'/Applications/Google Chrome.app/Contents/MacOS/Google Chrome',headless:true,args:['--no-sandbox']});
const page=await browser.newPage({viewport:{width:1100,height:750},hasTouch:true}),errors=[],messages=[];
page.on('pageerror',e=>errors.push(e.message));page.on('console',m=>messages.push(m.text()));
async function click(x,y,touch=false){const b=await page.locator('#canvas').boundingBox(),s=await page.evaluate(()=>[Module.canvas.width,Module.canvas.height]);const px=b.x+x*b.width/s[0],py=b.y+y*b.height/s[1];if(touch)await page.touchscreen.tap(px,py);else await page.mouse.click(px,py);await page.waitForTimeout(250);}
async function snap(name){await page.mouse.move(0,0);await page.screenshot({path:join(out,name+'.png')});}
async function almanac(touch=false){await click(370,455,touch);await click(208,366,touch);await page.waitForFunction(()=>Module._pvz_almanac_data(0,0)===0,undefined,{timeout:5000});await click(690,580,touch);}
try{
 await page.goto(process.env.PVZ_QA_URL||'http://127.0.0.1:8097/');await page.waitForFunction(()=>!document.getElementById('start').disabled,undefined,{timeout:90000});
 await page.evaluate(async()=>{
  const FS=Module.FS;if(!FS.analyzePath('/saves/userdata').exists)FS.mkdir('/saves/userdata');
  const name=new TextEncoder().encode('MenuQA'),users=new Uint8Array(16+name.length),u=new DataView(users.buffer);
  u.setUint32(0,14,true);u.setUint16(4,1,true);u.setUint16(6,name.length,true);users.set(name,8);u.setUint32(8+name.length,1,true);u.setUint32(12+name.length,1,true);FS.writeFile('/saves/userdata/users.dat',users);
  const profile=new Uint8Array(4096),p=new DataView(profile.buffer);p.setUint32(0,12,true);p.setUint32(4,27,true);FS.writeFile('/saves/userdata/user1.dat',profile);
  await new Promise((r,j)=>FS.syncfs(false,e=>e?j(e):r()));
 });
 await page.locator('#start').click();await page.waitForTimeout(12000);await click(400,560);await page.waitForTimeout(4500);await snap('menu');
 // Former woodsign3 was at x32..320, y180..245. No area may open a modal/editor.
 for(const [x,y]of [[60,200],[170,210],[290,220]])await click(x,y);
 await snap('removed-area-clicked');await almanac();
 // Achievements use the same slide/update path that formerly moved the editor.
 await click(85,525);await page.waitForTimeout(1600);await snap('achievements');await click(185,70);await page.waitForTimeout(1600);await almanac();
 await click(260,348);await page.waitForFunction(()=>Module.canvas.width===1024,undefined,{timeout:5000});await snap('sandbox');
 // Recreating the selector can yield through Emscripten Asyncify, so observe
 // completion instead of reading a synchronous return value during the unwind.
 await page.evaluate(()=>Module._pvz_sandbox_command(15,0,0,0));await page.waitForFunction(()=>Module.canvas.width===800);await page.waitForTimeout(2200);
 await snap('menu-after-return');
 for(const [name,width,height]of [['phone-landscape',844,390],['phone-portrait',390,844]]){
  await page.setViewportSize({width,height});await page.waitForTimeout(300);await click(170,210,true);await almanac(true);await snap(name);
 }
 // Adventure must still start after destroying/recreating the menu.
 await page.setViewportSize({width:1100,height:750});await page.waitForTimeout(250);await click(560,135);await page.waitForTimeout(15000);await snap('adventure-chooser');
 assert.ok(await page.evaluate(()=>Module._pvz_adventure_power_data(-1,3)>0),'adventure board exists');
 assert.deepEqual(errors,[]);assert.ok(!messages.some(m=>/unable to load|Aborted|RuntimeError/i.test(m)),messages.join('\n'));
 await writeFile(join(out,'report.json'),JSON.stringify({removedAreaNoAction:true,almanac:true,achievements:true,sandboxReturn:true,mobileTouch:true,adventure:true,errors},null,2));
 console.log('Zombatar removal and preserved menu navigation passed',out);
}catch(e){await snap('failure').catch(()=>{});throw e;}finally{await browser.close();}
