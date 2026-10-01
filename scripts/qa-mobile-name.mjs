// Real WASM in an isolated touch browser; never touches existing player saves.
import assert from 'node:assert/strict';
import {mkdir,writeFile} from 'node:fs/promises';
import {join} from 'node:path';
const {chromium}=await import(process.env.PVZ_PLAYWRIGHT||'playwright-core');
const out=process.env.PVZ_QA_OUT||'/tmp/pvz-mobile-name';await mkdir(out,{recursive:true});
const browser=await chromium.launch({executablePath:'/Applications/Google Chrome.app/Contents/MacOS/Google Chrome',headless:true,args:['--no-sandbox']});
const context=await browser.newContext({viewport:{width:844,height:390},hasTouch:true,isMobile:true,deviceScaleFactor:1});
const page=await context.newPage(),errors=[],results={};page.on('pageerror',e=>errors.push(String(e)));
const input=page.locator('#pvz-soft-keyboard');
const wait=(fn,arg,timeout=20000)=>page.waitForFunction(fn,arg,{timeout});
const shot=name=>page.screenshot({path:join(out,name+'.png')});
async function tap(x,y){const box=await page.locator('#canvas').boundingBox(),size=await page.evaluate(()=>[Module.canvas.width,Module.canvas.height]);await page.touchscreen.tap(box.x+x*box.width/size[0],box.y+y*box.height/size[1]);}
async function boot(){
 await page.goto(process.env.PVZ_QA_URL||'http://127.0.0.1:8097/');await wait(()=>!document.getElementById('start').disabled,undefined,90000);
 await page.locator('#start').tap();await page.waitForTimeout(12000);await tap(400,560);
}
async function nameIs(text){await wait(text=>Module.pvzTextInput.field?.text===text&&!Module.pvzTextInput.pending,text);assert.equal(await input.inputValue(),text);}
try{
 await boot();await wait(()=>Module.pvzTextInput?.active);await input.waitFor({state:'visible'});await shot('new-name');
 await input.tap();assert.equal(await input.evaluate(el=>document.activeElement===el),true);results.touchFocus=true;
 await input.fill('abc');await nameIs('Abc');await input.press('Backspace');await nameIs('Ab'); // Native title-casing is retained.
 await input.evaluate(el=>el.setSelectionRange(0,1));await page.keyboard.insertText('Z');await nameIs('Zb');results.selectionAndDeletion=true;
 await input.fill('abcdefghijklmnop');await nameIs('abcdefghijkl');results.nativeCharacterLimit=true;
 await input.fill('');await nameIs('');
 await input.evaluate(el=>{el.dispatchEvent(new CompositionEvent('compositionstart',{bubbles:true}));el.value='xiao';el.dispatchEvent(new InputEvent('input',{bubbles:true,data:'xiao',isComposing:true}));});
 await page.waitForTimeout(200);assert.equal(await input.inputValue(),'xiao');assert.equal(await page.evaluate(()=>Module.pvzTextInput.field.text),'');
 await input.evaluate(el=>{el.value='小猫';el.setSelectionRange(2,2);el.dispatchEvent(new CompositionEvent('compositionend',{bubbles:true,data:'小猫'}));el.dispatchEvent(new InputEvent('input',{bubbles:true,data:'小猫'}));});
 await nameIs('小猫');results.chineseComposition=true;await shot('chinese-name');
 await page.setViewportSize({width:390,height:844});await page.waitForTimeout(300);await input.tap();await shot('portrait-name');
 const bounds=await input.boundingBox();assert.ok(bounds.x>=0&&bounds.x+bounds.width<=390&&bounds.y>=0&&bounds.y+bounds.height<=844);await nameIs('小猫');results.rotationKeepsName=true;
 await page.setViewportSize({width:844,height:390});await page.waitForTimeout(200);await input.press('Enter');await wait(()=>!Module.pvzTextInput.active);await page.waitForTimeout(1500);
 const names=await page.evaluate(()=>new TextDecoder().decode(Module.FS.readFile('/saves/userdata/users.dat')));assert.ok(names.includes('小猫'));results.savedName=true;await shot('created-profile');
 await page.evaluate(()=>new Promise((resolve,reject)=>Module.FS.syncfs(false,e=>e?reject(e):resolve())));await boot();await page.waitForTimeout(2000);
 assert.equal(await page.evaluate(()=>Module.pvzTextInput.active),false);assert.ok((await page.evaluate(()=>new TextDecoder().decode(Module.FS.readFile('/saves/userdata/users.dat')))).includes('小猫'));results.reloadKeepsName=true;
 assert.deepEqual(errors,[]);await writeFile(join(out,'report.json'),JSON.stringify({results,errors},null,2));console.log('Mobile name QA passed',results);
}catch(error){await shot('failure');await writeFile(join(out,'failure.json'),JSON.stringify({error:String(error),errors,results,state:await page.evaluate(()=>({active:Module.pvzTextInput?.active,field:Module.pvzTextInput?.field,pending:Module.pvzTextInput?.pending,log:window.pvzEngineLog}))},null,2));throw error;}
finally{await browser.close();}
