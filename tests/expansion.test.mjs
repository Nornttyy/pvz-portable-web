import test from 'node:test';
import assert from 'node:assert/strict';
import {readFile,mkdtemp,copyFile} from 'node:fs/promises';
import {execFile} from 'node:child_process';
import {promisify} from 'node:util';
import {join} from 'node:path';
import {tmpdir} from 'node:os';
import {fileURLToPath} from 'node:url';
import {ORIGINAL_PLANTS,ORIGINAL_ZOMBIES,ZOMBIES,validateLayout} from '../web/sandbox-data.mjs';
const run=promisify(execFile),root=fileURLToPath(new URL('../',import.meta.url));
const read=name=>readFile(join(root,name));
test('actual production combat modules pass 24 native simulation scenarios',async()=>{
 const dir=await mkdtemp(join(tmpdir(),'pvz-expansion-combat-')),binary=join(dir,'combat');
 // Copy source unmodified only to let the compiler resolve state doubles before engine headers.
 for(const f of ['SandboxPlants.cpp','SandboxZombies.cpp'])await copyFile(join(root,'src',f),join(dir,f));
 await run(process.env.CXX||'c++',['-std=c++20','-Itests/combat-stubs','-Isrc',join(dir,'SandboxPlants.cpp'),join(dir,'SandboxZombies.cpp'),'tests/expansion-combat.cpp','-o',binary],{cwd:root});
 const result=await run(binary);assert.match(result.stdout,/24 production combat scenarios passed/);
});
test('10 new plants plus existing eight and 10 zombies are available in native and web catalogs',async()=>{
 assert.deepEqual(ORIGINAL_PLANTS.map(x=>x.id),Array.from({length:18},(_,i)=>100+i));
 assert.deepEqual(ORIGINAL_ZOMBIES.map(x=>x.id),Array.from({length:10},(_,i)=>200+i));assert.equal(ZOMBIES.length,33);
 for(const [defs,file]of [[ORIGINAL_PLANTS,'SandboxPlants.h'],[ORIGINAL_ZOMBIES,'SandboxZombies.h']]){
  const source=(await read('src/'+file)).toString();for(const d of defs){assert.ok(source.includes(d.name));assert.ok(source.includes(d.note));}
 }
 for(const p of ORIGINAL_PLANTS)assert.equal(validateLayout({schema:1,map:0,plants:[{type:p.id,col:0,row:0}]}).plants[0].type,p.id);
 assert.throws(()=>validateLayout({schema:1,map:0,plants:[{type:118,col:0,row:0}]}));
 assert.doesNotThrow(()=>validateLayout({schema:1,map:0,plants:[{type:111,col:0,row:0},{type:35,col:0,row:0}]}));
});
test('generated production parts preserve native bone canvases and exclude empty sprites',async()=>{
 const parts=JSON.parse(await read('art/expansion/parts.json'));assert.equal(parts.length,84);
 for(const part of parts){const png=await read('art/expansion/parts/'+part.file);assert.equal(png.readUInt32BE(16),part.width);assert.equal(png.readUInt32BE(20),part.height);assert.equal(png[25],6);const[x,y,w,h]=part.ink;assert.ok(w>0&&h>0&&x>=0&&y>=0&&x+w<=part.width&&y+h<=part.height,part.file);}
});
test('actual projectile/effect draw code preserves bone and world coordinate contracts',async()=>{
 const dir=await mkdtemp(join(tmpdir(),'pvz-visual-contracts-')),binary=join(dir,'visual');
 for(const f of ['SandboxPlants.cpp','SandboxZombies.cpp','SandboxArt.cpp'])await copyFile(join(root,'src',f),join(dir,f));
 await run(process.env.CXX||'c++',['-std=c++20','-Itests/combat-stubs','-Isrc',...['SandboxPlants.cpp','SandboxZombies.cpp','SandboxArt.cpp'].map(f=>join(dir,f)),'tests/visual-coordinates.cpp','-o',binary],{cwd:root});
 assert.match((await run(binary)).stdout,/Visual coordinate contracts passed/);
});
test('40 generated VFX sprites ship with transparent alpha and no placeholder drawing',async()=>{
 const parts=JSON.parse(await read('art/expansion/vfx-parts.json'));assert.equal(parts.length,40);
 for(const p of parts){const png=await read('art/expansion/parts/'+p.file);assert.equal(png.readUInt32BE(16),p.width);assert.equal(png.readUInt32BE(20),p.height);assert.equal(png[25],6);}
 for(const f of ['SandboxPlants.cpp','SandboxZombies.cpp'])assert.doesNotMatch((await read('src/'+f)).toString(),/DrawLine\(|FillRect\(/);
 const ui=(await read('src/SandboxUI.cpp')).toString();assert.doesNotMatch(ui,/DrawEffects/);
 assert.match((await read('src/Lawn/Board.cpp')).toString(),/MakeRenderOrder\(RENDER_LAYER_PARTICLE,row,1\)/);
});
test('expansion is sandbox-only and stepping/clear operations clean up special state',async()=>{
 const cpp=(await read('src/Sandbox.cpp')).toString();assert.ok(cpp.indexOf('board->mPaused = paused && !stepOnce')<cpp.indexOf('SandboxPlants::Tick(board)'));
 assert.match(cpp,/SandboxZombies::Reset\(\)/);assert.match(cpp,/SandboxZombies::Base\(type\)/);
 const render=(await read('src/SandboxPlants.cpp')).toString();assert.match(render,/DrawFit/);assert.match(render,/starts_with\("GatlingPea_barrel"\)/);
});
