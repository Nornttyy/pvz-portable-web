// Optional real-Chrome QA; install playwright-core outside the production site.
// PVZ_PLAYWRIGHT may be an absolute path to its index.mjs. Never uses a personal profile.
import assert from 'node:assert/strict';
import {mkdtemp,mkdir,writeFile} from 'node:fs/promises';
import {tmpdir} from 'node:os';
import {join} from 'node:path';
const {chromium}=await import(process.env.PVZ_PLAYWRIGHT||'playwright-core');
const out=process.env.PVZ_QA_OUT||await mkdtemp(join(tmpdir(),'pvz-meme-browser-'));
await mkdir(out,{recursive:true});
const browser=await chromium.launch({executablePath:process.env.PVZ_CHROME||'/Applications/Google Chrome.app/Contents/MacOS/Google Chrome',headless:true,args:['--no-sandbox','--autoplay-policy=no-user-gesture-required']});
let page;const errors=[],logs=[],phases=[];
try{
 page=await browser.newPage({viewport:{width:1100,height:750},hasTouch:true});
 page.on('pageerror',e=>errors.push(e.message));page.on('console',m=>logs.push(m.text()));
 await page.goto(process.env.PVZ_QA_URL||'http://127.0.0.1:8097/');
 await page.waitForFunction(()=>!document.getElementById('start').disabled,{},{timeout:90000});
 await page.locator('#start').click();await page.waitForTimeout(12000);
 async function point(x,y){const b=await page.locator('#canvas').boundingBox();const size=await page.evaluate(()=>[Module.canvas.width,Module.canvas.height]);return[b.x+x*b.width/size[0],b.y+y*b.height/size[1]];}
 async function click(x,y){await page.mouse.click(...await point(x,y));await page.waitForTimeout(150);}
 async function api(...args){return page.evaluate(v=>Module._pvz_sandbox_command(...[...v,0,0,0,0].slice(0,4)),args);}
 async function data(){return page.evaluate(()=>{const a=[];for(let i=0;i<180;i++){const type=Module._pvz_sandbox_plant_data(i,0);if(type<0)break;a.push(Array.from({length:7},(_,field)=>Module._pvz_sandbox_plant_data(i,field)));}return a;});}
 async function waitPhase(type,phase){await page.waitForFunction(([type,phase])=>{for(let i=0;i<180;i++){const id=Module._pvz_sandbox_plant_data(i,0);if(id<0)return false;if(id===type&&Module._pvz_sandbox_plant_data(i,4)===phase)return true;}return false;},[type,phase],{timeout:60000,polling:30});await api(4,1);phases.push(await data());}
 async function shot(name){await page.waitForTimeout(80);await page.screenshot({path:join(out,name+'.png')});}
 await click(400,560);await page.waitForTimeout(4500);await click(400,312);
 await page.keyboard.type('PowerQA',{delay:50});await page.keyboard.press('Enter');await page.waitForTimeout(2500);
 await click(719,27);await page.waitForFunction(()=>Module.canvas.width===1024,{},{timeout:20000});
 await shot('power-entry');
 // Actual UI, not only API: first ingredient -> lawn, power -> existing plant.
 await click(33,150);await click(384,130);assert.equal((await data())[0][0],0);
 await click(127,150);await page.mouse.move(...await point(384,130));await shot('fusion-preview');
 await click(384,130);assert.equal((await data())[0][0],120);assert.equal(await api(9),1);
 assert.equal(await api(1,180,2,0),-6);assert.equal(await api(1,180,1,0),-6);
 await api(21,0);await api(1,1,2,1);assert.equal(await api(1,180,2,1),-6);await api(21,1);
 for(const [base,result,row] of [[1,121,2],[3,122,3]]){
  assert.equal(await api(1,base,1,row),1);const n=await api(9);assert.equal(await api(22,180,1,row),result);
  assert.equal(await api(1,180,1,row),result);assert.equal(await api(9),n);
 }
 // Support layers survive power application, export/reload and disabled fusion.
 await api(8,1);await api(21,1);
 for(const id of [16,3,30])assert.equal(await api(1,id,2,2),1);
 assert.equal(await api(1,180,2,2),122);assert.deepEqual((await data()).map(p=>p[0]).sort((a,b)=>a-b),[16,30,122]);
 await page.evaluate(()=>window.dispatchEvent(new CustomEvent('pvz-sandbox-action',{detail:1})));
 await api(7);await page.evaluate(()=>window.dispatchEvent(new CustomEvent('pvz-sandbox-action',{detail:2})));
 assert.deepEqual((await data()).map(p=>p[0]).sort((a,b)=>a-b),[16,30,122]);await shot('power-pool-layers');
 await api(8,0);await api(21,1);
 for(const [base,col,row] of [[0,1,0],[1,1,2],[3,5,3]]){await api(1,base,col,row);await api(1,180,col,row);}
 await api(1,0,1,1);await api(2,23,8,0);await api(2,23,8,1);await api(2,0,5,3);
 await api(5,2);await api(4,0);await waitPhase(120,1);await shot('heat-burst');
 const frozen=await data();await page.waitForTimeout(350);assert.deepEqual(await data(),frozen);
 await api(4,0);await waitPhase(120,2);await shot('heat-recovery');
 await api(4,0);await waitPhase(122,2);await shot('nut-push');
 const damaged=(await data()).find(p=>p[0]===122);assert.ok(damaged[3]<4000&&damaged[3]>0);
 await api(5,4);await api(4,0);await waitPhase(121,1);await shot('sun-burst');
 await api(4,0);await waitPhase(121,2);await shot('sun-recovery');
 // Actual native damaged-walnut states, not a static palette preview.
 for(const [threshold,name] of [[2600,'nut-damage-1'],[1250,'nut-damage-2']]){
  await api(4,0);
  await page.waitForFunction(limit=>{for(let i=0;i<180;i++)if(Module._pvz_sandbox_plant_data(i,0)===122)return Module._pvz_sandbox_plant_data(i,3)<=limit;return false;},threshold,{timeout:60000,polling:25});
  await api(4,1);assert.ok((await data()).find(p=>p[0]===122)[3]>0);await shot(name);
 }
 await page.evaluate(()=>window.dispatchEvent(new CustomEvent('pvz-sandbox-action',{detail:1})));
 const saved=await page.evaluate(()=>JSON.parse(localStorage.getItem('pvz.sandbox.formation.v1')));
 assert.ok(saved.plants.some(p=>p.type===121));assert.ok(!saved.plants.some(p=>p.type===180));
 await page.setViewportSize({width:844,height:390});await shot('phone-powers');
 const bounds=await page.locator('#canvas').boundingBox();assert.ok(bounds.x>=0&&bounds.y>=0&&bounds.x+bounds.width<=845&&bounds.y+bounds.height<=391);
 // Touch selection, preview and application use the same native hit targets.
 await page.setViewportSize({width:390,height:844});await shot('portrait-powers');
 const portrait=await page.locator('#canvas').boundingBox();assert.ok(portrait.x>=0&&portrait.y>=0&&portrait.x+portrait.width<=391&&portrait.y+portrait.height<=845);
 await api(8,0);await api(21,1);
 for(const [x,y] of [[33,150],[384,230],[127,150],[384,230]]){await page.touchscreen.tap(...await point(x,y));await page.waitForTimeout(150);}
 assert.equal((await data())[0][0],120);assert.equal(await api(9),1);await shot('phone-touch-fusion');
 assert.deepEqual(errors,[]);assert.equal(await page.locator('#error-detail').textContent(),'');
 assert.ok(!logs.some(x=>/unable to load|failed to load music|Aborted|RuntimeError/i.test(x)));
 await writeFile(join(out,'report.json'),JSON.stringify({url:page.url(),phases,saved,bounds,portrait,errors,logs},null,2));
 console.log('PUBLIC/LOCAL meme-power browser QA passed: '+out);
}catch(error){if(page){await page.screenshot({path:join(out,'failure.png')});console.log(await page.evaluate(()=>({log:window.pvzEngineLog,error:document.getElementById('error-detail')?.textContent})));}throw error;}
finally{await browser.close();}
