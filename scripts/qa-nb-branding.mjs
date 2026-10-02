import assert from 'node:assert/strict';
import {mkdir,writeFile} from 'node:fs/promises';
import {join} from 'node:path';
const {chromium}=await import(process.env.PVZ_PLAYWRIGHT||'playwright-core');
const out=process.env.PVZ_QA_OUT||'/tmp/pvz-nb-branding';await mkdir(out,{recursive:true});
const browser=await chromium.launch({executablePath:'/Applications/Google Chrome.app/Contents/MacOS/Google Chrome',headless:true});
const page=await browser.newPage({viewport:{width:1100,height:750},hasTouch:true});
const errors=[],badResponses=[];page.on('pageerror',e=>errors.push(e.stack||e.message));
page.on('response',r=>{if(r.status()>=400)badResponses.push(r.url()+':'+r.status());});
async function snap(name){await page.mouse.move(0,0);await page.screenshot({path:join(out,name+'.png')});}
async function click(x,y,touch=false){const box=await page.locator('#canvas').boundingBox(),s=await page.evaluate(()=>[Module.canvas.width,Module.canvas.height]);const px=box.x+x*box.width/s[0],py=box.y+y*box.height/s[1];if(touch)await page.touchscreen.tap(px,py);else await page.mouse.click(px,py);await page.waitForTimeout(250);}
async function sandbox(touch=false){await click(719,27,touch);await page.waitForFunction(()=>Module.canvas.width===1024,undefined,{timeout:5000});}
async function back(){await click(783,24);await click(726,342);await page.waitForFunction(()=>Module.canvas.width===800);await page.waitForTimeout(2200);}
try{
 await page.goto(process.env.PVZ_QA_URL||'http://127.0.0.1:8097/');await page.waitForFunction(()=>!document.getElementById('start').disabled,undefined,{timeout:90000});
 assert.equal(await page.title(),'植物大战僵尸 NB版');
 assert.ok(await page.locator('.game-brand img').evaluateAll(images=>images.every(i=>i.complete&&i.naturalWidth>0)));
 await snap('web-logo');
 await page.setViewportSize({width:390,height:844});await snap('web-logo-phone');await page.setViewportSize({width:1100,height:750});
 await page.evaluate(async()=>{
  const FS=Module.FS;if(!FS.analyzePath('/saves/userdata').exists)FS.mkdir('/saves/userdata');
  const name=new TextEncoder().encode('BrandQA'),users=new Uint8Array(16+name.length),u=new DataView(users.buffer);
  u.setUint32(0,14,true);u.setUint16(4,1,true);u.setUint16(6,name.length,true);users.set(name,8);u.setUint32(8+name.length,1,true);u.setUint32(12+name.length,1,true);FS.writeFile('/saves/userdata/users.dat',users);
  const profile=new Uint8Array(4096),p=new DataView(profile.buffer);p.setUint32(0,12,true);p.setUint32(4,27,true);FS.writeFile('/saves/userdata/user1.dat',profile);
  await new Promise((r,j)=>FS.syncfs(false,e=>e?j(e):r()));
 });
 await page.locator('#start').click();await page.waitForTimeout(12000);await snap('native-title-logo');
 assert.ok(await page.evaluate(()=>Module.FS.readFile('/addons/images/nb-edition.png').length>1000));
 await click(400,560);await page.waitForTimeout(4500);await snap('menu-desktop');
 // Decorative branding must not block username, almanac, or the sliding achievements page.
 await click(170,150);await snap('change-user-dialog');await page.keyboard.press('Escape');await page.waitForTimeout(400);
 await click(370,455);await click(208,366);await page.waitForFunction(()=>Module._pvz_almanac_data(0,0)===0,undefined,{timeout:5000});await click(690,580);
 await click(85,525);await page.waitForTimeout(1600);await snap('achievements-no-logo-overlay');await click(185,70);await page.waitForTimeout(1600);await snap('menu-after-achievements');
 await click(190,230);assert.equal(await page.evaluate(()=>Module.canvas.width),800,'logo is decorative, not a button');
 await click(260,348);assert.equal(await page.evaluate(()=>Module.canvas.width),800,'old menu location must not open sandbox');
 await sandbox();await snap('sandbox-desktop');await back();
 for(const [name,width,height]of [['landscape',844,390],['portrait',390,844]]){
  await page.setViewportSize({width,height});await page.waitForTimeout(300);await snap('menu-phone-'+name);
  await sandbox(true);await snap('sandbox-phone-'+name);await back();
 }
 await page.setViewportSize({width:1100,height:750});await click(560,135);await page.waitForTimeout(15000);
 assert.ok(await page.evaluate(()=>Module._pvz_adventure_power_data(-1,3)>0),'adventure still starts');
 assert.deepEqual(errors,[]);assert.deepEqual(badResponses,[]);
 await writeFile(join(out,'report.json'),JSON.stringify({webAndNativeBrand:true,mainMenuBrand:true,usernameDialog:true,almanac:true,achievementsSlide:true,embeddedBadge:true,desktopSandbox:true,phoneLandscapeTouch:true,phonePortraitTouch:true,oldHitboxRemoved:true,adventure:true,errors,badResponses},null,2));
 console.log('NB logo and compact sandbox navigation passed',out);
}catch(e){await snap('failure').catch(()=>{});await writeFile(join(out,'failure.json'),JSON.stringify({errors,badResponses,error:e.stack},null,2));throw e;}finally{await browser.close();}
