import test from 'node:test';
import assert from 'node:assert/strict';
import {readFile,mkdtemp,copyFile} from 'node:fs/promises';
import {execFile} from 'node:child_process';
import {promisify} from 'node:util';
import {join} from 'node:path';
import {tmpdir} from 'node:os';
import {fileURLToPath} from 'node:url';
import {TECH_PLANTS,TECH_ZOMBIES,ZOMBIES,validateLayout} from '../web/sandbox-data.mjs';

const run=promisify(execFile),root=fileURLToPath(new URL('../',import.meta.url));
const read=name=>readFile(join(root,name));

test('production technology combat modules pass root-network simulation',async()=>{
 const dir=await mkdtemp(join(tmpdir(),'pvz-tech-combat-')),binary=join(dir,'combat');
 for(const f of ['SandboxPlants.cpp','SandboxZombies.cpp'])await copyFile(join(root,'src',f),join(dir,f));
 await run(process.env.CXX||'c++',['-std=c++20','-Itests/combat-stubs','-Isrc',join(dir,'SandboxPlants.cpp'),join(dir,'SandboxZombies.cpp'),'tests/expansion-combat.cpp','-o',binary],{cwd:root});
 assert.match((await run(binary)).stdout,/Tech root-network combat scenarios passed/);
});

test('ten technology plants and ten mechanical enemies agree across web and native catalogs',async()=>{
 assert.deepEqual(TECH_PLANTS.map(x=>x.id),Array.from({length:10},(_,i)=>100+i));
 assert.deepEqual(TECH_ZOMBIES.map(x=>x.id),Array.from({length:10},(_,i)=>200+i));
 assert.equal(ZOMBIES.length,33);
 for(const [defs,file]of [[TECH_PLANTS,'SandboxPlants.h'],[TECH_ZOMBIES,'SandboxZombies.h']]){
  const source=(await read('src/'+file)).toString();for(const d of defs){assert.ok(source.includes(d.name));assert.ok(source.includes(d.note));}
 }
 for(const p of TECH_PLANTS)assert.equal(validateLayout({schema:1,map:0,plants:[{type:p.id,col:0,row:0}]}).plants[0].type,p.id);
 assert.throws(()=>validateLayout({schema:1,map:0,plants:[{type:110,col:0,row:0}]}));
 assert.equal(validateLayout({schema:1,map:0,plants:[{type:109,col:0,row:0}]}).plants[0].type,109);
});

test('technology art contract requires separate rigs, damage states, ammunition and readable root effects',async()=>{
 const spec=JSON.parse(await read('art/tech/production-spec.json'));
 assert.equal(spec.schema,'tech-garden-production-v1');assert.equal(spec.plants.length,10);assert.equal(spec.zombies.length,10);
 assert.match(spec.style.rendering,/no photorealism/i);assert.match(spec.style.rendering,/no 3D/i);
 assert.match(spec.rig_contract.mouth_rule,/dedicated/i);assert.match(spec.rig_contract.mouth_rule,/independently drawn/i);assert.match(spec.rig_contract.damage_rule,/damage1/i);
 for(const unit of [...spec.plants,...spec.zombies]){
  assert.ok(unit.parts.length>=6,unit.slug);assert.ok(unit.damage.includes('body-normal')&&unit.damage.includes('body-damage1')&&unit.damage.includes('body-damage2'),unit.slug);
 }
 for(const zombie of spec.zombies)assert.ok(zombie.parts.includes('head')&&zombie.parts.includes('jaw'),zombie.slug);
 assert.ok(spec.mechanic.visual.includes('root-resonance')&&spec.mechanic.visual.includes('root-sever'));
 const source=(await read('src/SandboxPlants.cpp')).toString()+(await read('src/SandboxZombies.cpp')).toString();
 for(const vfx of ['root-charge-on','root-resonance','prism-beam','hook-line','jam-wave','magnet-field','holo-glitch'])assert.ok(source.includes(vfx),vfx);
});

test('persistent native sidebar fits both rosters and keeps the lawn in original units',async()=>{
 const dir=await mkdtemp(join(tmpdir(),'pvz-sidebar-')),binary=join(dir,'sidebar');
 await run(process.env.CXX||'c++',['-std=c++20','-Isrc','tests/sidebar-layout.cpp','-o',binary],{cwd:root});
 assert.match((await run(binary)).stdout,/99 grass\/pool cell mappings passed/);
 const ui=(await read('src/SandboxUI.cpp')).toString();assert.match(ui,/class SandboxOverlay final : public Widget/);assert.doesNotMatch(ui,/showZombies|OpenPanel\(1\)|OpenPanel\(2\)/);
});

test('actual projectile, rig and effect code preserves bone and world-coordinate contracts',async()=>{
 const dir=await mkdtemp(join(tmpdir(),'pvz-tech-visual-')),binary=join(dir,'visual');
 for(const f of ['SandboxPlants.cpp','SandboxZombies.cpp','SandboxArt.cpp'])await copyFile(join(root,'src',f),join(dir,f));
 await run(process.env.CXX||'c++',['-std=c++20','-Itests/combat-stubs','-Isrc',...['SandboxPlants.cpp','SandboxZombies.cpp','SandboxArt.cpp'].map(f=>join(dir,f)),'tests/visual-coordinates.cpp','-o',binary],{cwd:root});
 assert.match((await run(binary)).stdout,/Visual coordinate contracts passed/);
});

test('root resonance is sandbox-only, uses the existing lawn UI, and never falls back to drawn placeholders',async()=>{
 const sandbox=(await read('src/Sandbox.cpp')).toString();
 assert.ok(sandbox.indexOf('board->mPaused = paused && !stepOnce')<sandbox.indexOf('SandboxPlants::Tick(board)'));
 assert.match(sandbox,/case 21:/);assert.match(sandbox,/SandboxPlants::ActivateNetwork\(board, col, row\)/);assert.match(sandbox,/SandboxZombies::Reset\(\)/);
 const ui=(await read('src/SandboxUI.cpp')).toString();
 assert.match(ui,/Command\(21,0,cell%9,cell\/9\)/);assert.match(ui,/根网共振/);assert.match(ui,/plants=\{100,101,103,104,106,109\}/);
 for(const f of ['SandboxPlants.cpp','SandboxZombies.cpp'])assert.doesNotMatch((await read('src/'+f)).toString(),/DrawLine\(|FillRect\(/);
 const code=(await read('src/SandboxPlants.cpp')).toString()+(await read('src/SandboxZombies.cpp')).toString();
 for(const retired of ['spring-nut','echo-lily','electric-pea','gum-zombie','parcel-zombie'])assert.doesNotMatch(code,new RegExp(retired));
});
