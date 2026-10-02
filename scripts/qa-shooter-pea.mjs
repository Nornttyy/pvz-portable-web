// Third plant: native art, actual WASM combat and an independent adventure card.
import assert from 'node:assert/strict';
import {mkdir,writeFile} from 'node:fs/promises';
import {join} from 'node:path';
const {chromium}=await import(process.env.PVZ_PLAYWRIGHT||'playwright-core');
const out=process.env.PVZ_QA_OUT||'/tmp/pvz-shooter-pea-qa';await mkdir(out,{recursive:true});
const browser=await chromium.launch({executablePath:'/Applications/Google Chrome.app/Contents/MacOS/Google Chrome',headless:true,args:['--no-sandbox','--autoplay-policy=no-user-gesture-required']});
const page=await browser.newPage({viewport:{width:1100,height:750},hasTouch:true}),errors=[],results={};page.on('pageerror',e=>errors.push(e.message));
async function click(x,y){const b=await page.locator('#canvas').boundingBox(),s=await page.evaluate(()=>[Module.canvas.width,Module.canvas.height]);await page.mouse.click(b.x+x*b.width/s[0],b.y+y*b.height/s[1]);await page.waitForTimeout(100);}
async function api(...args){return page.evaluate(a=>Module._pvz_sandbox_command(...[...a,0,0,0,0].slice(0,4)),args);}
async function snap(name){await page.screenshot({path:join(out,name+'.png')});}
async function wait(fn,arg,timeout=15000){await page.waitForFunction(fn,arg,{timeout});}
async function bank(){return page.evaluate(()=>Array.from({length:6},(_,i)=>[0,1,2].map(f=>Module._pvz_adventure_seed_data(i,f))));}
async function boot(seed=false){
 await page.goto(process.env.PVZ_QA_URL||'http://127.0.0.1:8097/');await wait(()=>!document.getElementById('start').disabled,undefined,90000);
 if(seed)await page.evaluate(async()=>{const FS=Module.FS;if(!FS.analyzePath('/saves/userdata').exists)FS.mkdir('/saves/userdata');
  const name=new TextEncoder().encode('PeaQA'),users=new Uint8Array(16+name.length),u=new DataView(users.buffer);u.setUint32(0,14,true);u.setUint16(4,1,true);u.setUint16(6,name.length,true);users.set(name,8);u.setUint32(8+name.length,1,true);u.setUint32(12+name.length,1,true);FS.writeFile('/saves/userdata/users.dat',users);
  const profile=new Uint8Array(4096),p=new DataView(profile.buffer);p.setUint32(0,12,true);p.setUint32(4,8,true);FS.writeFile('/saves/userdata/user1.dat',profile);await new Promise((r,j)=>FS.syncfs(false,e=>e?j(e):r()));});
 await page.locator('#start').click();await page.waitForTimeout(12000);await click(400,560);await page.waitForTimeout(4500);
}
try{
 await boot(true);
 await click(719,27);await wait(()=>Module.canvas.width===1024);await api(7);await api(5,1);await page.waitForTimeout(1600);
 for(let id=502;id<=518;++id)assert.equal(await api(1,id,0,0),-2);
 assert.ok(await api(1,519,2,2)>0);assert.ok(await api(1,500,2,1)>0);assert.ok(await api(1,501,2,3)>0);
 await snap('three-plants-and-cards');
 // Backward-shooter enum storage must NOT inherit backward aiming.
 await api(2,0,0,2);await api(4,0);await page.waitForTimeout(1000);await api(4,1);
 assert.equal(await page.evaluate(()=>Module._pvz_projectile_data(0,0)),-1);results.noBackwardFire=true;
 await api(2,4,8,2);await api(4,0);await wait(()=>Module._pvz_projectile_data(0,0)===296);await page.waitForTimeout(450);await api(4,1);
 const projectile=await page.evaluate(()=>Array.from({length:9},(_,f)=>Module._pvz_projectile_data(0,f)));
 assert.equal(projectile[0],296);assert.ok(projectile[3]>0);assert.equal(projectile[4],0);assert.equal(projectile[5],2);results.projectile=projectile;
 assert.equal(await page.evaluate(()=>Module._pvz_sandbox_plant_data(0,0)),519);await snap('pea-head-fires-whole-shooter');
 await page.setViewportSize({width:844,height:390});await snap('phone');await page.setViewportSize({width:1100,height:750});
 await api(4,0);await page.waitForTimeout(2500);await api(4,1);
 const hp=await page.evaluate(()=>Module._pvz_sandbox_zombie_data(1,4)+Module._pvz_sandbox_zombie_data(1,5));assert.ok(hp<1370);results.nativeDamage=true;
 await api(15);await page.waitForTimeout(800);await click(560,135);await page.waitForTimeout(15000);
 // Ninth first-row cell belongs to 519, not a detached overlay.
 for(const base of [0,3,1,5,4])await click(47+base*53,163);await click(471,163);await page.waitForTimeout(600);await snap('adventure-chooser');await click(258,566);
 await wait(()=>Module._pvz_adventure_power_data(-1,5)===1,undefined,25000);
 results.bank=await bank();assert.deepEqual(results.bank.map(x=>x[0]),[500,501,1,5,4,519]);assert.deepEqual(results.bank[5],[519,125,300]);await snap('adventure-bank');
 // Save with 519 selected but NOT planted: bank identity must survive reload.
 await click(748,14);await click(400,401);await page.waitForTimeout(250);await click(305,394);await wait(()=>Module._pvz_adventure_power_data(-1,5)===-1);
 await page.evaluate(()=>new Promise((r,j)=>Module.FS.syncfs(false,e=>e?j(e):r())));await boot();await click(560,135);await wait(()=>Module._pvz_adventure_power_data(-1,5)===1,undefined,30000);
 assert.deepEqual(await bank(),results.bank);results.unplantedCardSurvivesReload=true;await click(280,371);
 const deadline=Date.now()+60000;
 while(await page.evaluate(()=>Module._pvz_adventure_power_data(-1,2))<125){assert.ok(Date.now()<deadline,'sun collection deadline');const sun=await page.evaluate(()=>[Module._pvz_adventure_power_data(0,10),Module._pvz_adventure_power_data(0,11)]);if(sun[0]>=0)await click(...sun);await page.waitForTimeout(150);}
 await wait(()=>Module._pvz_adventure_power_data(0,16)>=0,undefined,40000);
 const lane=await page.evaluate(()=>Module._pvz_adventure_power_data(0,16));results.adventureLane=lane;
 const pos=await page.evaluate(()=>[Module._pvz_adventure_seed_data(5,5)+25,Module._pvz_adventure_seed_data(5,6)+35]);await click(...pos);await click(240,130+100*lane);
 await wait(()=>Module._pvz_adventure_power_data(0,0)===519);assert.equal(await page.evaluate(()=>Module._pvz_adventure_power_data(0,8)),52);
 await wait(()=>Module._pvz_projectile_data(0,0)===296,undefined,50000);await click(748,14);await snap('adventure-firing');results.adventureCombat=true;
 await click(400,401);await page.waitForTimeout(250);await click(305,394);await wait(()=>Module._pvz_adventure_power_data(-1,5)===-1);
 await page.evaluate(()=>new Promise((r,j)=>Module.FS.syncfs(false,e=>e?j(e):r())));await boot();await click(560,135);await wait(()=>Module._pvz_adventure_power_data(-1,5)===1,undefined,30000);
 assert.equal(await page.evaluate(()=>Module._pvz_adventure_power_data(0,0)),519);assert.equal(await page.evaluate(()=>Module._pvz_projectile_data(0,0)),296);results.plantAndShotResume=true;await snap('resumed-third-plant');
 assert.deepEqual(errors,[]);await writeFile(join(out,'report.json'),JSON.stringify({results,errors},null,2));console.log('Shooter pea browser QA passed',out,results);
}catch(e){await snap('failure');await writeFile(join(out,'failure.json'),JSON.stringify({error:String(e),results,errors,log:await page.evaluate(()=>window.pvzEngineLog)},null,2));throw e;}
finally{await browser.close();}
