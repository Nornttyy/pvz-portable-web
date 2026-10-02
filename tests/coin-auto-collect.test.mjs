import test from 'node:test';
import assert from 'node:assert/strict';
import {readFile,writeFile,mkdtemp} from 'node:fs/promises';
import {tmpdir} from 'node:os';
import {join} from 'node:path';
import {execFile} from 'node:child_process';
import {promisify} from 'node:util';
const run=promisify(execFile),root=new URL('../',import.meta.url);
test('native coin update automatically collects silver/gold once with unchanged flight and scoring',async()=>{
 const source=await readFile(new URL('src/Lawn/Coin.cpp',root),'utf8');
 function method(start){const from=source.indexOf(start);assert.ok(from>=0);let depth=0,end=source.indexOf('{',from);do{if(source[end]==='{')depth++;if(source[end]==='}')depth--;end++;}while(depth);return source.slice(from,end);}
 const names=['void Coin::TryAutoCollectCoin(','void Coin::Update()','void Coin::UpdateCollected(','void Coin::ScoreCoin(','bool Coin::IsMoney(CoinType','bool Coin::IsMoney()','bool Coin::IsSun(','int Coin::GetCoinValue(','int Coin::GetSunValue(','float Coin::GetSunScale('];
 const dir=await mkdtemp(join(tmpdir(),'pvz-auto-coins-')),cpp=join(dir,'test.cpp'),binary=join(dir,'test');
 await writeFile(cpp,await readFile(new URL('tests/coin-auto-collect.cpp',root),'utf8')+'\n'+names.map(method).join('\n'));
 await run(process.env.CXX||'c++',['-std=c++20',cpp,'-o',binary]);assert.match((await run(binary)).stdout,/Auto coins:.*passed/);
 const update=method('void Coin::Update()');assert.ok(update.indexOf('TryAutoCollectCoin();')<update.indexOf('if (mFadeCount != 0)'));
 const auto=method('void Coin::TryAutoCollectCoin(');assert.match(auto,/PlayCollectSound\(\);\s*Collect\(\);/);assert.doesNotMatch(auto,/AddCoins|ScoreCoin|MouseDown|Cursor/);
 const collect=method('void Coin::Collect(');assert.match(collect,/mIsBeingCollected = true/);assert.match(collect,/mBoard->ShowCoinBank\(\)/);assert.match(collect,/mFadeCount = 0/);assert.doesNotMatch(collect,/AddCoins/);
});
