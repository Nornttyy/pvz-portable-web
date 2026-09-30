import test from 'node:test';
import assert from 'node:assert/strict';
import {readFile,writeFile,mkdtemp} from 'node:fs/promises';
import {tmpdir} from 'node:os';
import {join} from 'node:path';
import {execFile} from 'node:child_process';
import {promisify} from 'node:util';
import {fileURLToPath} from 'node:url';
const run=promisify(execFile),root=fileURLToPath(new URL('../',import.meta.url));
test('native production emits 50-value suns only for sunflower and grown sun-shroom',async()=>{
 const plant=await readFile(join(root,'src/Lawn/Plant.cpp'),'utf8'),coin=await readFile(join(root,'src/Lawn/Coin.cpp'),'utf8');
 function method(source,start){const from=source.indexOf(start);assert.ok(from>=0);let depth=0,end=source.indexOf('{',from);do{if(source[end]==='{')++depth;if(source[end]==='}')--depth;++end;}while(depth);return source.slice(from,end);}
 const native=[method(plant,'void Plant::UpdateProductionPlant('),method(plant,'void Plant::UpdateSunShroom('),method(coin,'int Coin::GetSunValue(')];
 const dir=await mkdtemp(join(tmpdir(),'pvz-sun-production-')),source=join(dir,'test.cpp'),binary=join(dir,'test');
 await writeFile(source,await readFile(join(root,'tests/sun-production.cpp'),'utf8')+'\n'+native.join('\n'));
 await run(process.env.CXX||'c++',['-std=c++20','-Isrc',source,'-o',binary],{cwd:root});assert.match((await run(binary)).stdout,/Native sun production:.*unchanged/);
});
