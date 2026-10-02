import test from 'node:test';
import assert from 'node:assert/strict';
import {readFile,writeFile,mkdtemp} from 'node:fs/promises';
import {execFile} from 'node:child_process';
import {promisify} from 'node:util';
import {tmpdir} from 'node:os';
import {join} from 'node:path';
import {PLANTS} from '../web/sandbox-data.mjs';
const root=new URL('../',import.meta.url).pathname,run=promisify(execFile),read=p=>readFile(join(root,p),'utf8');
test('only Everything Shooter blasts are reduced; native bombs and cannon remain unchanged',async()=>{
 const source=await read('src/Lawn/Board.cpp'),dir=await mkdtemp(join(tmpdir(),'pvz-reduced-blast-')),binary=join(dir,'blast');
 await writeFile(join(dir,'blast-production.inc'),source.slice(source.indexOf('int Board::KillAllZombiesInRadius('),source.indexOf('int Board::GetNumWavesPerSurvivalStage()')));
 await run(process.env.CXX||'c++',['-std=c++20','-I'+dir,'tests/reduced-blast-native.cpp','-o',binary],{cwd:root});
 assert.match((await run(binary)).stdout,/reduced blasts respect armor, area, boss and native 1800 defaults/);
 assert.match(await read('src/Lawn/Projectile.cpp'),/EverythingShooterRules::BlastDamage\(MemeCharacters::ShotStyle\(this\)\)/);
});
test('everything shooter production launch and impact functions preserve all 14 native types and exact rare odds',async()=>{
 const source=await read('src/EverythingShooter.cpp'),dir=await mkdtemp(join(tmpdir(),'pvz-everything-native-')),binary=join(dir,'combat');
 await writeFile(join(dir,'everything-production.inc'),source.slice(source.indexOf('bool IsSlot('),source.indexOf('bool DrawShot(')));
 await run(process.env.CXX||'c++',['-std=c++20','-Isrc','-I'+dir,'tests/everything-shooter-native.cpp','-o',binary],{cwd:root});
 assert.match((await run(binary)).stdout,/all 17 production launches, exact weights, native effects, muzzle anchors, capacity and rare impacts passed/);
});
test('new independent card does not replace bowling; native cold preloading and friendly enemy ammunition are scoped',async()=>{
 assert.equal(PLANTS.find(p=>p.id===529).base,49);
 const plant=await read('src/Lawn/Plant.cpp'),app=await read('src/LawnApp.cpp'),shot=await read('src/Lawn/Projectile.cpp'),source=await read('src/EverythingShooter.cpp');
 assert.match(plant,/switch \(EverythingShooter::IsSlot\(theSeedType\) \? SEED_PEASHOOTER : theSeedType\)/);
 assert.match(plant,/mSubClass=SUBCLASS_NORMAL,.mLaunchRate=0,.mPlantName="EVERYTHING_SHOOTER"/);
 assert.match(plant,/if \(EverythingShooter::Fire\(this,theTargetZombie\)\) return/);
 assert.match(plant,/PreloadPlantResources\(ammo\)/);assert.match(plant,/ZOMBIE_CATAPULT/);
 assert.match(app,/SEED_EXPLODE_O_NUT[\s\S]*?EverythingShooterRules::Unlock/);
 assert.equal((shot.match(/!EverythingShooterRules::Own\(MemeCharacters::ShotStyle\(this\)\)/g)||[]).length,3);
 assert.match(source,/MakeCachedPlantFrame\(style==EverythingShooterRules::Doom\?SEED_DOOMSHROOM:SEED_CHERRYBOMB/);
 assert.match(source,/preview.SetFramesForLayer\("anim_full_idle"\)/);
 assert.doesNotMatch(source,/AddCrater|mPlants|StinkShroom|ApplyChill|HitIceTrap/,'special ammunition adds no unrequested friendly/control/crater effects');
 const png=await readFile(join(root,'addons/images/everything-poop.png'));assert.equal(png.subarray(0,8).toString('hex'),'89504e470d0a1a0a');assert.equal(png[25],6,'real RGBA art, not a drawn placeholder');
});
test('actual new plant definition and 3-7 unlock retain explosive bowling nuts in other modes',async()=>{
 const source=await read('src/Lawn/Plant.cpp'),app=await read('src/LawnApp.cpp');
 const definition=source.slice(source.indexOf('const PlantDefinition& GetPlantDefinition(SeedType theSeedType)\n{'),source.indexOf('int Plant::GetCost('));
 const unlock=app.slice(app.indexOf('\tif (theSeedType == SEED_EXPLODE_O_NUT) return'),app.indexOf('\tif (theSeedType == SEED_SPROUT) return'));
 const dir=await mkdtemp(join(tmpdir(),'pvz-everything-mode-')),cpp=join(dir,'test.cpp'),binary=join(dir,'mode');
 await writeFile(cpp,`#include "ConstEnums.h"
#include "IceChiliRules.h"
#include "EverythingShooterRules.h"
#include <cassert>
#include <initializer_list>
#define PVZP_ASSERT assert
namespace MemeAdventure {bool enabled=false;bool RosterEnabled(){return enabled;}}
bool gSandboxEnabled=false;
namespace EverythingShooter {bool IsSlot(int seed){return seed==49&&(gSandboxEnabled||MemeAdventure::RosterEnabled());}}
enum PlantSubClass {SUBCLASS_NORMAL,SUBCLASS_SHOOTER};
struct PlantDefinition {SeedType mSeedType=SEED_NONE;void* mPlantImage=nullptr;ReanimationType mReanimationType=REANIM_NONE;int mPacketIndex=0,mSeedCost=0,mRefreshTime=0;PlantSubClass mSubClass=SUBCLASS_NORMAL;int mLaunchRate=0;const char* mPlantName="";};
PlantDefinition gPlantDefs[NUM_SEED_TYPES];
${definition}
struct Player {int level=1;int GetLevel(){return level;}};
struct App {Player player;Player* mPlayerInfo=&player;bool complete=false;bool HasFinishedAdventure(){return complete;}bool HasSeedType(SeedType theSeedType){${unlock}return false;}};
int main(){
 App a;gPlantDefs[SEED_EXPLODE_O_NUT].mSeedType=SEED_EXPLODE_O_NUT;gPlantDefs[SEED_EXPLODE_O_NUT].mReanimationType=REANIM_WALLNUT;gPlantDefs[SEED_PEASHOOTER].mSeedType=SEED_PEASHOOTER;
 for(bool mode:{false,true})for(bool sandbox:{false,true}){
  MemeAdventure::enabled=mode;gSandboxEnabled=sandbox;const auto& d=GetPlantDefinition(SEED_EXPLODE_O_NUT);
  if(mode||sandbox){assert(d.mReanimationType==REANIM_PEASHOOTER&&d.mSeedCost==250&&d.mRefreshTime==750&&d.mLaunchRate==0&&d.mSubClass==SUBCLASS_NORMAL);assert(&d!=&gPlantDefs[SEED_EXPLODE_O_NUT]);}
  else assert(&d==&gPlantDefs[SEED_EXPLODE_O_NUT]&&d.mReanimationType==REANIM_WALLNUT&&d.mSeedCost==0);
  assert(&GetPlantDefinition(SEED_PEASHOOTER)==&gPlantDefs[SEED_PEASHOOTER]);
  for(int level=1;level<=50;++level){a.player.level=level;assert(a.HasSeedType(SEED_EXPLODE_O_NUT)==(mode&&level>=27));}
  a.player.level=1;a.complete=true;assert(a.HasSeedType(SEED_EXPLODE_O_NUT)==mode);a.complete=false;
 }
 a.mPlayerInfo=nullptr;assert(!a.HasSeedType(SEED_EXPLODE_O_NUT));
}
`);
 await run(process.env.CXX||'c++',['-std=c++20','-Isrc',cpp,'-o',binary],{cwd:root});await run(binary);
});
