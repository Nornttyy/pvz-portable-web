import test from 'node:test';
import assert from 'node:assert/strict';
import {validateLayout} from '../web/sandbox-data.mjs';
import {readFile,writeFile,mkdtemp} from 'node:fs/promises';
import {execFile} from 'node:child_process';
import {promisify} from 'node:util';
import {tmpdir} from 'node:os';
import {join} from 'node:path';
test('mixed allegiance formations retain each plant team and legacy saves remain ordinary',()=>{
 const legacy={schema:1,map:0,plants:[{type:501,col:1,row:2}]};
 assert.deepEqual(validateLayout(legacy),legacy);
 const mixed={schema:1,map:0,plants:[{type:501,col:1,row:2},{type:5,col:5,row:2,charmed:true}]};
 assert.deepEqual(validateLayout(mixed),mixed);
 for(const charmed of ['true',1,null,{},[]])assert.throws(()=>validateLayout({...legacy,plants:[{...legacy.plants[0],charmed}]}));
 const stacked=validateLayout({schema:1,map:1,adapted:true,stacked:true,plants:[{type:5,col:1,row:2,charmed:true},{type:501,col:1,row:2},{type:16,col:1,row:2,charmed:true}]});
 assert.equal(stacked.plants[0].type,16);assert.equal(stacked.plants[0].charmed,true);
 assert.equal(stacked.plants.find(p=>p.type===5).charmed,true);assert.equal(stacked.plants.find(p=>p.type===501).charmed,undefined);
});
test('production facing follows actual motion, and fume reflection keeps the native mouth pivot',async()=>{
 const source=await readFile(new URL('../src/SandboxFactions.cpp',import.meta.url),'utf8');
 const dir=await mkdtemp(join(tmpdir(),'pvz-ranged-direction-')),run=promisify(execFile);
 const direction=source.slice(source.indexOf('int Direction('),source.indexOf('void SyncShotArt('));
 const particles=source.slice(source.indexOf('void ParticleMatrix('),source.lastIndexOf('\n}'));
 await writeFile(join(dir,'direction.cpp'),`#include <cassert>
#include <map>
enum {MOTION_STRAIGHT,MOTION_BACKWARDS,MOTION_BEE_BACKWARDS,MOTION_STAR,MOTION_HOMING,MOTION_LOBBED,MOTION_PUFF};
struct Projectile {int mMotionType=MOTION_STRAIGHT;float mVelX=0;};
struct Shot {bool hostile;unsigned target;bool reverse;};
bool gSandboxEnabled=true;
std::map<const Projectile*,Shot> shots;
float particleAxis=-1;
namespace Sexy {struct SexyMatrix3 {float m00=1,m01=0,m02=0,m10=0,m11=1,m12=0;};}
${direction}
${particles}
int main(){
 Projectile p;
 assert(!TravelsLeft(&p));shots[&p]={true,0,true};assert(TravelsLeft(&p));
 p.mMotionType=MOTION_BACKWARDS;assert(!TravelsLeft(&p));
 shots[&p]={true,0,false};assert(TravelsLeft(&p)); // normal zombie already moves backwards
 shots[&p]={false,0,false};p.mMotionType=MOTION_STRAIGHT;assert(!TravelsLeft(&p));
 for(int motion:{MOTION_STAR,MOTION_HOMING,MOTION_LOBBED}){p.mMotionType=motion;p.mVelX=-3;assert(TravelsLeft(&p));p.mVelX=3;assert(!TravelsLeft(&p));}
 gSandboxEnabled=false;p.mMotionType=MOTION_STRAIGHT;shots[&p]={true,0,true};assert(!TravelsLeft(&p));
 Sexy::SexyMatrix3 m;m.m00=2;m.m01=.25;m.m02=555;m.m12=37;
 ParticleMatrix(m,224);assert(m.m02==555&&m.m00==2);
 particleAxis=2*400+80;ParticleMatrix(m,224);assert(m.m02==773&&m.m00==-2&&m.m01==-.25&&m.m12==37);
 ParticleMatrix(m,224);assert(m.m02==555&&m.m00==2&&m.m01==.25);
}
`);
 await run(process.env.CXX||'c++',['-std=c++20',join(dir,'direction.cpp'),'-o',join(dir,'direction')]);await run(join(dir,'direction'));
});
