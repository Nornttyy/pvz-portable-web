import test from 'node:test';
import assert from 'node:assert/strict';
import {readFile,writeFile,mkdtemp} from 'node:fs/promises';
import {execFile} from 'node:child_process';
import {promisify} from 'node:util';
import {tmpdir} from 'node:os';
import {join} from 'node:path';
import {PLANTS} from '../web/sandbox-data.mjs';
const root=new URL('../',import.meta.url).pathname,run=promisify(execFile),read=p=>readFile(join(root,p),'utf8');
test('ice chili production blast deals exact same-row damage and native freeze without extra effects',async()=>{
 const source=await read('src/IceChili.cpp'),dir=await mkdtemp(join(tmpdir(),'pvz-ice-chili-')),binary=join(dir,'combat');
 await writeFile(join(dir,'ice-production.inc'),source.slice(source.indexOf('namespace IceChili {')));
 await run(process.env.CXX||'c++',['-std=c++20','-Isrc','-I'+dir,'tests/ice-chili-native.cpp','-o',binary],{cwd:root});
 assert.match((await run(binary)).stdout,/same-row 1200 damage, 300-tick freeze, original exclusions and nine native ice bursts passed/);
});
test('independent ice chili keeps original fire pepper and mode-gates the native Zen sprout',async()=>{
 assert.equal(PLANTS.find(p=>p.id===528).base,51);assert.ok(PLANTS.some(p=>p.id===20),'original Jalapeno stays');
 const plant=await read('src/Lawn/Plant.cpp'),app=await read('src/LawnApp.cpp');
 assert.match(plant,/SEED_SPROUT && \(gSandboxEnabled \|\| MemeAdventure::RosterEnabled\(\)\)/);
 assert.match(plant,/case SeedType::SEED_SPROUT:\s*if \(aPlantDef.mReanimationType == REANIM_JALAPENO && IsInPlay\(\)\)/);
 assert.match(plant,/mDoSpecialCountdown = IceChiliRules::Windup/);
 assert.match(plant,/void Plant::DoSpecial\(\)\s*\{\s*if \(IceChili::Detonate\(this\)\) return;/);
 assert.match(plant,/SandboxPlants::DrawIceChiliPreview/);
 assert.match(app,/theSeedType == SEED_SPROUT[\s\S]*?MemeAdventure::RosterEnabled\(\)[\s\S]*?IceChiliRules::Unlock/);
 const save=await read('src/Lawn/System/SaveGame.cpp');
 for(const clock of ['mDoSpecialCountdown','mIceTrapCounter','mChilledCounter'])assert.ok(save.includes(clock),'native saved '+clock);
 const source=await read('src/IceChili.cpp');assert.doesNotMatch(source,/HitIceTrap\(|ApplyChill\(|BurnRow\(|DoFwoosh\(/);
});
test('production plant definition and unlock gate preserve native sprout outside adventure and sandbox',async()=>{
 const source=await read('src/Lawn/Plant.cpp'),app=await read('src/LawnApp.cpp');
 const definition=source.slice(source.indexOf('const PlantDefinition& GetPlantDefinition(SeedType theSeedType)\n{'),source.indexOf('int Plant::GetCost('));
 const unlock=app.slice(app.indexOf('\tif (theSeedType == SEED_SPROUT) return'),app.indexOf('\tif (theSeedType == SEED_SMALL_NUT) return'));
 const dir=await mkdtemp(join(tmpdir(),'pvz-ice-mode-')),cpp=join(dir,'test.cpp'),binary=join(dir,'mode');
 await writeFile(cpp,`#include "ConstEnums.h"
#include "IceChiliRules.h"
#include "EverythingShooterRules.h"
#include "CoinPlantRules.h"
#include <cassert>
#define PVZP_ASSERT assert
namespace MemeAdventure {bool enabled=false;bool RosterEnabled(){return enabled;}}
bool gSandboxEnabled=false;
namespace EverythingShooter {bool IsSlot(int){return false;}}
namespace CoinPlants {bool ShooterSlot(int){return false;}}
enum PlantSubClass {SUBCLASS_NORMAL,SUBCLASS_SHOOTER};
struct PlantDefinition {SeedType mSeedType=SEED_NONE;void* mPlantImage=nullptr;ReanimationType mReanimationType=REANIM_NONE;int mPacketIndex=0,mSeedCost=0,mRefreshTime=0;PlantSubClass mSubClass=SUBCLASS_NORMAL;int mLaunchRate=0;const char* mPlantName="";};
PlantDefinition gPlantDefs[NUM_SEED_TYPES];
${definition}
struct Player {int level=1;int GetLevel(){return level;}};
struct App {Player player;Player* mPlayerInfo=&player;bool complete=false;bool HasFinishedAdventure(){return complete;}bool HasSeedType(SeedType theSeedType){${unlock}return false;}};
int main(){
 App a;gPlantDefs[SEED_SPROUT].mSeedType=SEED_SPROUT;gPlantDefs[SEED_SPROUT].mReanimationType=REANIM_ZENGARDEN_SPROUT;
 gPlantDefs[SEED_JALAPENO].mSeedType=SEED_JALAPENO;
 for(bool mode:{false,true})for(bool sandbox:{false,true}){
  MemeAdventure::enabled=mode;gSandboxEnabled=sandbox;const auto& d=GetPlantDefinition(SEED_SPROUT);
  if(mode||sandbox){assert(d.mReanimationType==REANIM_JALAPENO&&d.mSeedCost==125&&d.mRefreshTime==5000);assert(&d!=&gPlantDefs[SEED_SPROUT]);}
  else assert(&d==&gPlantDefs[SEED_SPROUT]&&d.mReanimationType==REANIM_ZENGARDEN_SPROUT);
  assert(&GetPlantDefinition(SEED_JALAPENO)==&gPlantDefs[SEED_JALAPENO]);
  for(int level=1;level<=50;++level){a.player.level=level;assert(a.HasSeedType(SEED_SPROUT)==(mode&&level>=26));}
  a.player.level=1;a.complete=true;assert(a.HasSeedType(SEED_SPROUT)==mode);a.complete=false;
 }
}
`);
 await run(process.env.CXX||'c++',['-std=c++20','-Isrc','-include','initializer_list',cpp,'-o',binary],{cwd:root});await run(binary);
});
