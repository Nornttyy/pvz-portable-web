import test from 'node:test';
import assert from 'node:assert/strict';
import {readFile,writeFile,mkdtemp} from 'node:fs/promises';
import {tmpdir} from 'node:os';
import {join} from 'node:path';
import {execFile} from 'node:child_process';
import {promisify} from 'node:util';
const run=promisify(execFile),root=new URL('../',import.meta.url);
test('hover collection executes native board/coin paths without collecting awards or cancelling tools',async()=>{
 const board=await readFile(new URL('src/Lawn/Board.cpp',root),'utf8'),coin=await readFile(new URL('src/Lawn/Coin.cpp',root),'utf8');
 function method(source,start){const from=source.indexOf(start);assert.ok(from>=0);let depth=0,end=source.indexOf('{',from);do{if(source[end]==='{')++depth;if(source[end]==='}')--depth;++end;}while(depth);return source.slice(from,end);}
 for(const name of ['void Board::MouseMove(','void Board::MouseDrag('])assert.match(method(board,name),/CollectSunAt\(x, y\)/);
 const native=[method(board,'void Board::CollectSunAt('),...['bool Coin::IsSun(','int Coin::GetSunValue(','void Coin::MouseDown(','bool Coin::MouseHitTest('].map(name=>method(coin,name))];
 const dir=await mkdtemp(join(tmpdir(),'pvz-sun-hover-')),source=join(dir,'test.cpp'),binary=join(dir,'test');
 await writeFile(source,await readFile(new URL('tests/sun-hover.cpp',root),'utf8')+'\n'+native.join('\n'));
 await run(process.env.CXX||'c++',['-std=c++20',source,'-o',binary]);assert.match((await run(binary)).stdout,/Sun hover:.*passed/);
});
