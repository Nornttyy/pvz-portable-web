import test from 'node:test';
import assert from 'node:assert/strict';
import {mkdtemp} from 'node:fs/promises';
import {execFile} from 'node:child_process';
import {promisify} from 'node:util';
import {tmpdir} from 'node:os';
import {join} from 'node:path';
import {fileURLToPath} from 'node:url';
const run=promisify(execFile),root=fileURLToPath(new URL('../',import.meta.url));
test('real native font repair retains bitmap metrics and style at all three sizes',async()=>{
  const dir=await mkdtemp(join(tmpdir(),'pvz-font-repair-')),binary=join(dir,'repair');
  await run(process.env.CXX||'c++',['-std=c++20','-Itests/font-stubs','src/SandboxFonts.cpp','tests/font-repair-native.cpp','-o',binary],{cwd:root});
  const result=await run(binary);assert.match(result.stdout,/idempotence passed/);
});
