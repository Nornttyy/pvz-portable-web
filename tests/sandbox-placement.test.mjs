import test from 'node:test';
import assert from 'node:assert/strict';
import {readFile,mkdtemp,writeFile} from 'node:fs/promises';
import {execFile} from 'node:child_process';
import {promisify} from 'node:util';
import {join} from 'node:path';
import {tmpdir} from 'node:os';
import {fileURLToPath} from 'node:url';
import vm from 'node:vm';
import {validateLayout,requiresStacking,LAYOUT_KEY} from '../web/sandbox-data.mjs';
const root=fileURLToPath(new URL('../',import.meta.url)),run=promisify(execFile);
const read=name=>readFile(join(root,name),'utf8');
const plant=(type=0,col=0,row=0)=>({type,col,row});
test('production placement function and repeat state machine pass native tests',async()=>{
 const src=await read('src/Sandbox.cpp'),dir=await mkdtemp(join(tmpdir(),'pvz-placement-'));
 const start=src.indexOf('static int PlacePlant('),end=src.indexOf('static int Spawn(',start);
 assert.ok(start>0&&end>start);
 await writeFile(join(dir,'placement-under-test.inc'),src.slice(start,end));
 await run('c++',['-std=c++20','-Isrc','-I'+dir,'tests/sandbox-placement-native.cpp','-o',join(dir,'placement')],{cwd:root});
 assert.match((await run(join(dir,'placement'))).stdout,/Production placement.*passed/);
});
test('stacked formations round-trip duplicates, mixed units, shells and two-cell cannons',()=>{
 for(const plants of [[plant(),plant()],[plant(),plant(105),plant(30),plant(30)],[plant(47),plant(0,1)]]){
   const layout={schema:1,map:0,stacked:true,plants};
   assert.equal(requiresStacking(plants),true);
   assert.deepEqual(validateLayout(JSON.parse(JSON.stringify(validateLayout(layout)))),validateLayout(layout));
   assert.throws(()=>validateLayout({...layout,stacked:false}));
 }
 assert.equal(requiresStacking([plant(33),plant(),plant(30)]),false);
 assert.equal(requiresStacking([plant(),plant(0,1)]),false);
 assert.equal(requiresStacking([plant(16,0,2),plant(43,0,2)]),true);
 assert.equal(requiresStacking([plant(47),plant(30,1)]),true);
 assert.equal(validateLayout({schema:1,map:0,plants:[plant()]}).stacked,undefined);
});
test('stack mode still validates bounds, water support, special targets and capacity',()=>{
 const layout=(plants,map=0)=>({schema:1,map,stacked:true,plants});
 for(const value of [layout([plant(47,8)]),layout([plant(0,-1)]),layout(Array(181).fill(plant())),layout([plant(16)]),layout([plant(11)]),layout([plant(35)]),layout([plant(0,0,2)],1),layout([plant(16,0,2),plant(4,0,2)],1),layout([plant(16,0,2),plant(47,0,2)],1),{...layout([]),stacked:'yes'}])assert.throws(()=>validateLayout(value));
 const plants=[plant(0,0,2),plant(0,0,2),plant(16,0,2)];
 assert.deepEqual(validateLayout(layout(plants,1)).plants.map(p=>p.type),[16,0,0]);
 assert.equal(validateLayout(layout(Array(180).fill(plant()))).plants.length,180);
});
test('storage bridge restores stacking before planting and detects stacks even after toggle is off',async()=>{
 const handlers=new Map(),calls=[],storage=new Map();let flags=11,plants=[plant(),plant()];
 const target={addEventListener(){},classList:{toggle(){},remove(){}},click(){}};
 const Module={_pvz_sandbox_command(cmd,type,col,row){
   calls.push([cmd,type,col,row]);
   if(cmd===0)return flags;if(cmd===18)return 1;
   if(cmd===8){plants=[];return 1;}
   if(cmd===19){flags=type?flags|16:flags&~16;return 1;}
   if(cmd===1){assert.ok(flags&16);plants.push({type,col,row});return 1;}
   return 1;
 },_pvz_sandbox_plant_data(i,field){const p=plants[i];return p?[p.type,p.col,p.row][field]:-1;}};
 const context=vm.createContext({Module,validateLayout,requiresStacking,LAYOUT_KEY,
   document:{...target,body:target,getElementById:()=>target},window:{addEventListener:(name,fn)=>handlers.set(name,fn)},
   localStorage:{setItem:(k,v)=>storage.set(k,v),getItem:k=>storage.get(k)},setInterval(){},clearInterval(){},setTimeout(){}});
 vm.runInContext((await read('web/sandbox.mjs')).replace(/^import[^;]+;/m,''),context);
 handlers.get('pvz-sandbox-action')({detail:1});
 assert.equal(JSON.parse(storage.get(LAYOUT_KEY)).stacked,true);
 handlers.get('pvz-sandbox-action')({detail:2});
 assert.deepEqual(plants,[plant(),plant()]);
 const enable=calls.findIndex(([cmd,type])=>cmd===19&&type===1),place=calls.findIndex(([cmd])=>cmd===1);
 assert.ok(enable>=0&&place>enable);
});
