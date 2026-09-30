import test from 'node:test';
import assert from 'node:assert/strict';
import {mkdtemp,readFile} from 'node:fs/promises';
import {execFile} from 'node:child_process';
import {promisify} from 'node:util';
import {tmpdir} from 'node:os';
import {join} from 'node:path';
import {fileURLToPath} from 'node:url';
const root=fileURLToPath(new URL('../',import.meta.url)),run=promisify(execFile);
test('extra adventure card shares the native grid at every unlock stage',async()=>{
 const dir=await mkdtemp(join(tmpdir(),'pvz-chooser-layout-')),bin=join(dir,'layout');
 await run(process.env.CXX||'c++',['-std=c++20','-Isrc','tests/seed-chooser-layout.cpp','-o',bin],{cwd:root});
 assert.match((await run(bin)).stdout,/49 full-size cards fit/);
 const source=await readFile(join(root,'src/Lawn/Widget/SeedChooserScreen.cpp'),'utf8');
 assert.match(source,/SeedChooserLayout::Card\(theIndex, mApp->HasSeedType\(SEED_LEFTPEATER\), Has7Rows\(\)\)/);
 assert.doesNotMatch(source,/x\s*=\s*464;\s*y\s*=\s*132|IMITATERADDON,\s*459,\s*120/);
 assert.match(source,/GetSeedPositionInChooser\(theChosenSeed.mSeedType, theChosenSeed.mEndX, theChosenSeed.mEndY\)/);
 assert.match(source,/GetSeedPositionInChooser\(SEED_LEFTPEATER, x, y\)/);
});
