import test from 'node:test';
import assert from 'node:assert/strict';
import {readFile,writeFile,mkdtemp} from 'node:fs/promises';
import {execFile} from 'node:child_process';
import {promisify} from 'node:util';
import {tmpdir} from 'node:os';
import {join} from 'node:path';
import {fileURLToPath} from 'node:url';
const run=promisify(execFile),root=fileURLToPath(new URL('../',import.meta.url));
test('small nut initializes at one fifth health and versioned saves preserve both independent cards',async()=>{
 const source=await readFile(join(root,'src/MemeAdventure.cpp'),'utf8'),plants=await readFile(join(root,'src/Lawn/Plant.cpp'),'utf8');
 const start=source.indexOf(' // Also migrate ordinary plants');
 const migration=source.slice(start,source.indexOf(' if(b->mApp->IsAdventureMode())for(const auto& saved:pending.shots)',start));
 const assign=source.slice(source.indexOf('void OnPlanted('),source.indexOf('void OnZombieSpawned('));
 const init=plants.slice(plants.indexOf('\tcase SeedType::SEED_WALLNUT:'),plants.indexOf('\tcase SeedType::SEED_EXPLODE_O_NUT:'));
 assert.ok(migration&&assign&&init);assert.match(plants,/mPlantMaxHealth = mPlantHealth;/);
 const folder=await mkdtemp(join(tmpdir(),'pvz-small-nut-save-')),cpp=join(folder,'test.cpp'),binary=join(folder,'test');
 await writeFile(cpp,`#include "ConstEnums.h"
#include "MemeAdventure.h"
#include <algorithm>
#include <cassert>
#include <iostream>
namespace Sexy {int Rand(int){return 0;}}
class Plant {public:SeedType mSeedType=SEED_SMALL_NUT,mImitaterType=SEED_NONE;bool mDead=false;int custom=0,mPlantHealth=123,mPlantMaxHealth=800,mBlinkCountdown=0;
 void Initialize(){const auto theSeedType=mSeedType;switch(theSeedType){${init}default:break;}mPlantMaxHealth=mPlantHealth;}};
struct Card{SeedType mPacketType=SEED_NONE,mImitaterType=SEED_NONE;int mRefreshTime=600,mRefreshCounter=275;bool mRefreshing=true;};
struct Bank{int mNumPackets=4;Card mSeedPackets[4];};
class Board{public:std::vector<Plant*> mPlants;Bank* mSeedBank=nullptr;};
namespace MemeCharacters {bool Is(const Plant* p){return p->custom!=0;}void Assign(Plant* p,int id){p->custom=id;}}
namespace SandboxPlants {void RestoreRetired(Plant* p,const PowerSave&){p->mSeedType=SEED_PEASHOOTER;}}
namespace MemeAdventure {
 Save pending;bool RosterEnabled(){return true;}
 const MemeCharacters::Definition* Replacement(int seed,int imitater){return MemeCharacters::ForBase(seed==48?imitater:seed);}
 ${assign}
 void RestoreRoster(Board* b){${migration}}
}
int main(){
 Plant small;small.Initialize();Plant big;big.mSeedType=SEED_WALLNUT;big.Initialize();assert(small.mPlantHealth==800&&small.mPlantMaxHealth==800&&small.mPlantHealth*5==big.mPlantHealth);
 for(int version:{0,51800,51900,52300}){
  Plant shooter,nut,copy;shooter.mSeedType=SEED_LEFTPEATER;copy.mSeedType=SEED_IMITATER;copy.mImitaterType=SEED_SMALL_NUT;
  Bank bank;bank.mSeedPackets[0].mPacketType=SEED_LEFTPEATER;bank.mSeedPackets[1].mPacketType=SEED_SMALL_NUT;
  bank.mSeedPackets[2].mPacketType=SEED_IMITATER;bank.mSeedPackets[2].mImitaterType=SEED_SMALL_NUT;
  bank.mSeedPackets[3].mPacketType=SEED_IMITATER;bank.mSeedPackets[3].mImitaterType=SEED_LEFTPEATER;
  Board b;b.mPlants={&shooter,&nut,&copy};b.mSeedBank=&bank;MemeAdventure::pending.cooldown=version;MemeAdventure::RestoreRoster(&b);
  assert(shooter.mSeedType==(version<51900?SEED_PEASHOOTER:SEED_LEFTPEATER));assert(shooter.custom==(version<51900?500:519));
  assert(nut.mSeedType==(version<52300?SEED_SUNFLOWER:SEED_SMALL_NUT));assert(nut.custom==(version<52300?520:523));
  assert(copy.mImitaterType==nut.mSeedType&&copy.mSeedType==SEED_IMITATER);
  assert(bank.mSeedPackets[0].mPacketType==shooter.mSeedType&&bank.mSeedPackets[3].mImitaterType==shooter.mSeedType);
  assert(bank.mSeedPackets[1].mPacketType==nut.mSeedType&&bank.mSeedPackets[2].mImitaterType==nut.mSeedType);
  assert(nut.mPlantHealth==123&&shooter.mPlantHealth==123); // Never heal during migration.
  if(version==52300){assert(bank.mSeedPackets[1].mRefreshTime==600&&bank.mSeedPackets[1].mRefreshCounter==275);assert(bank.mSeedPackets[2].mRefreshTime==600&&bank.mSeedPackets[2].mRefreshCounter==275);}
 }
 std::cout<<"Small nut native HP and old/current roster save migration passed\\n";
}`);
 await run(process.env.CXX||'c++',['-std=c++20','-Isrc',cpp,'-o',binary],{cwd:root});
 assert.match((await run(binary)).stdout,/Small nut native HP and old\/current roster save migration passed/);
});
test('wall-nut planting takes 12 seconds, including imitater and resumed seed cards, without changing attacks',async()=>{
 const source=await readFile(join(root,'src/MemeAdventure.cpp'),'utf8'),plants=await readFile(join(root,'src/Lawn/Plant.cpp'),'utf8'),packets=await readFile(join(root,'src/Lawn/SeedPacket.cpp'),'utf8');
 const mapping=source.slice(source.indexOf('bool RosterEnabled(){'),source.indexOf('void Reset(){'));
 const recharge=plants.slice(plants.indexOf('int Plant::GetRefreshTime('),plants.indexOf('bool Plant::IsNocturnal('));
 const restore=source.slice(source.indexOf('   if(const auto* replacement=Replacement(int(card.mPacketType)'),source.indexOf('\n  }\n }\n if(b->mApp->IsAdventureMode())for(const auto& saved:pending.shots)'));
 const update=packets.slice(packets.indexOf('\tif (!mActive && mRefreshing)'),packets.indexOf('\n\tif (mSlotMachineCountDown > 0)'));
 assert.ok(mapping&&recharge&&restore&&update);
 const folder=await mkdtemp(join(tmpdir(),'pvz-nut-cooldown-')),cpp=join(folder,'test.cpp'),binary=join(folder,'test');
 await writeFile(cpp,`#include "MemeCharacters.h"
#include <algorithm>
#include <cassert>
#include <iostream>
struct App {bool adventure=true,bowling=false,pots=false,whack=false;
 bool IsAdventureMode(){return adventure;}bool IsWallnutBowlingLevel(){return bowling;}bool IsScaryPotterLevel(){return pots;}bool IsWhackAZombieLevel(){return whack;}
} app;
App* gLawnApp=&app;bool gSandboxEnabled=false;
namespace MemeAdventure {${mapping}}
enum SeedType {SEED_NONE=-1,SEED_WALLNUT=3,SEED_IMITATER=48};
struct PlantDefinition {int mRefreshTime=750;};PlantDefinition definitions[60];
const PlantDefinition& GetPlantDefinition(SeedType type){return definitions[int(type)];}
namespace Challenge {bool IsZombieSeedType(SeedType){return false;}}
struct Plant {static int GetRefreshTime(SeedType,SeedType);};${recharge}
struct Card {int mPacketType=3,mImitaterType=-1,mRefreshTime=1200,mRefreshCounter=0;bool mRefreshing=true,mActive=false;
 void Activate(){mActive=true;}void FlashIfReady(){}void Update(){${update}}};
void RestoreCooldown(Card& card){using namespace MemeAdventure;${restore}}
int main(){
 definitions[3].mRefreshTime=3000;definitions[23].mRefreshTime=3000;
 for(const auto& d:MemeCharacters::Definitions){const int expected=d.id==501?1200:d.id==523?600:d.id==524||d.id==527?750:d.id==525?200:d.id==526?3000:d.id==528?5000:300;assert(Plant::GetRefreshTime(SeedType(d.base),SEED_NONE)==expected);assert(Plant::GetRefreshTime(SEED_IMITATER,SeedType(d.base))==expected);}
 assert(Plant::GetRefreshTime(SeedType(23),SEED_NONE)==3000);
 for(int seed:{3,48}){
  Card fresh;fresh.mPacketType=seed;fresh.mImitaterType=seed==48?3:-1;
  for(int tick=0;tick<1200;++tick){fresh.Update();assert(!fresh.mActive&&fresh.mRefreshing);}fresh.Update();assert(fresh.mActive&&!fresh.mRefreshing);
  Card saved;saved.mPacketType=seed;saved.mImitaterType=fresh.mImitaterType;saved.mRefreshCounter=475;RestoreCooldown(saved);assert(saved.mRefreshTime==1200&&saved.mRefreshCounter==475);
  saved.mRefreshTime=300;saved.mRefreshCounter=100;RestoreCooldown(saved);assert(saved.mRefreshTime==1200&&saved.mRefreshCounter==100);
  saved.mRefreshTime=3000;saved.mRefreshCounter=2900;RestoreCooldown(saved);assert(saved.mRefreshTime==1200&&saved.mRefreshCounter==1100);
  saved.mRefreshing=false;saved.mRefreshTime=300;saved.mRefreshCounter=0;RestoreCooldown(saved);assert(saved.mRefreshTime==300&&!saved.mRefreshing);
 }
 Card pea;pea.mPacketType=0;pea.mRefreshTime=3000;pea.mRefreshCounter=100;RestoreCooldown(pea);assert(pea.mRefreshTime==300&&pea.mRefreshCounter==0);
 for(bool* mode:{&gSandboxEnabled,&app.bowling,&app.pots,&app.whack}){*mode=true;assert(Plant::GetRefreshTime(SEED_WALLNUT,SEED_NONE)==3000);Card native;native.mRefreshTime=3000;RestoreCooldown(native);assert(native.mRefreshTime==3000);*mode=false;}
 app.adventure=false;assert(Plant::GetRefreshTime(SEED_WALLNUT,SEED_NONE)==3000);
 std::cout<<"12-second native planting and save boundaries passed\\n";
}`);
 await run(process.env.CXX||'c++',['-std=c++20','-Isrc',cpp,'-o',binary],{cwd:root});
 assert.match((await run(binary)).stdout,/12-second native planting and save boundaries passed/);
 const combat=await readFile(join(root,'src/MemeCharacters.cpp'),'utf8');
 assert.match(combat,/if\(target\)\{s\.timer=300;s\.pulse=50;/);
});
test('production replacement and localization respect native IDs, special modes and unrelated names',async()=>{
 const source=await readFile(join(root,'src/MemeAdventure.cpp'),'utf8');
 const mapping=source.slice(source.indexOf('bool RosterEnabled(){'),source.indexOf('void Reset(){'));
 const translate=source.slice(source.indexOf('std::string_view Translate('),source.indexOf('Save Capture('));
 const folder=await mkdtemp(join(tmpdir(),'pvz-replacement-')),cpp=join(folder,'test.cpp'),binary=join(folder,'test');
 await writeFile(cpp,`#include "MemeCharacters.h"
#include <map>
#include <string>
#include <string_view>
#include <cassert>
#include <iostream>
struct App {bool adventure=true,bowling=false,pots=false,whack=false;
 bool IsAdventureMode(){return adventure;}bool IsWallnutBowlingLevel(){return bowling;}bool IsScaryPotterLevel(){return pots;}bool IsWhackAZombieLevel(){return whack;}
} app;
App* gLawnApp=&app;bool gSandboxEnabled=false;
namespace MemeAdventure {${mapping}\n${translate}}
int main(){using namespace MemeAdventure;
 for(const auto& d:MemeCharacters::Definitions){assert(Replacement(d.base,-1)->id==d.id);assert(Replacement(48,d.base)->id==d.id);assert(Translate(d.key,"old")==d.name);assert(Translate(std::string(d.key)+"_TOOLTIP","old")==d.hint);assert(Translate(std::string(d.key)+"_DESCRIPTION","old")==d.description);}
 assert(!Replacement(48,-1)&&!Replacement(2,-1)&&!Replacement(503,-1));
 assert(MemeCharacters::Definitions.size()==12);
 assert(Translate("ADVICE_QA","向日葵和双子向日葵") == "向日葵和双子向日葵");
 assert(Translate("SEED_CHOOSER_QA","豌豆射手、小喷菇、坚果墙") == "红温豌豆、真·小喷菇、反咬坚果");
 for(int seed=0;seed<54;++seed)if(seed!=0&&seed!=1&&seed!=3&&seed!=7&&seed!=8&&seed!=10&&seed!=15&&seed!=26&&seed!=40&&seed!=51&&seed!=52&&seed!=53)assert(!Replacement(seed,-1));
 for(const char* key:{"FLAG_ZOMBIE","BUCKETHEAD_ZOMBIE","POLE_VAULTING_ZOMBIE","CONEHEAD_ZOMBIE","ZOMBIE","SCREEN_DOOR_ZOMBIE","FOOTBALL_ZOMBIE","BALLOON_ZOMBIE","NEWSPAPER_ZOMBIE","IMP","LADDER_ZOMBIE"})
  assert(Translate(key,"native")=="native"&&Translate(std::string(key)+"_DESCRIPTION","native")=="native");
 assert(Translate("GOLD_SUNFLOWER_TROPHY","金色向日葵奖杯") == "金色向日葵奖杯");
 for(bool* boundary:{&app.bowling,&app.pots,&app.whack,&gSandboxEnabled}){*boundary=true;for(const auto& d:MemeCharacters::Definitions){assert(!Replacement(d.base,-1));assert(Translate(d.key,"original")=="original");}*boundary=false;}
 app.adventure=false;assert(!Replacement(0,-1));app.adventure=true;gLawnApp=nullptr;assert(!Replacement(0,-1));
 std::cout<<"Native-slot mapping and localized text passed.\\n";
}`);
 await run(process.env.CXX||'c++',['-std=c++20','-Isrc',cpp,'-o',binary],{cwd:root});
 assert.match((await run(binary)).stdout,/Native-slot mapping and localized text passed/);
});

test('native adventure introduces each independent zombie gradually and limits clever variants per wave',async()=>{
 const source=await readFile(join(root,'src/MemeAdventure.cpp'),'utf8');
 const hook=source.slice(source.indexOf('void OnZombieSpawned('),source.indexOf('void Draw(Board*'));
 const folder=await mkdtemp(join(tmpdir(),'pvz-runner-wave-')),cpp=join(folder,'test.cpp'),binary=join(folder,'test');
 await writeFile(cpp,`#include "SandboxZombies.h"
#include <vector>
#include <cassert>
#include <iostream>
class Zombie;
class Board {public:int mLevel=1;std::vector<Zombie*> mZombies;bool pool=false;bool IsPoolSquare(int,int row){return pool&&(row==2||row==3);}};
class Zombie {public:Board* mBoard=nullptr;int mZombieType=0,mFromWave=0,id=0;bool board=true;int mRow=2;bool IsOnBoard(){return board;}};
namespace SandboxZombies {int Type(const Zombie* z){return z->id;}bool IsRunner(const Zombie* z){return z&&z->id==Runner;}bool IsConeWrap(const Zombie* z){return z&&z->id==ConeWrap;}void Assign(Zombie* z,int id){z->id=id;}}
namespace MemeAdventure {bool enabled=true;bool RosterEnabled(){return enabled;} ${hook}}
int main(){
 for(int level:{1,2,3,5,6,8,15,16,17,20,22,23,25,26,27,31,32,33,50})for(int wave=-3;wave<25;++wave)for(int base:{0,1,2,3,4,23,24})for(bool onBoard:{false,true})for(bool enabled:{false,true})for(bool water:{false,true}){
  Board b;b.mLevel=level;b.pool=water;Zombie a{&b,base,wave,0,onBoard},c{&b,base,wave,0,onBoard};b.mZombies={&a,&c};MemeAdventure::enabled=enabled;
  MemeAdventure::OnZombieSpawned(&a);MemeAdventure::OnZombieSpawned(&c);
  const bool eligible=enabled&&onBoard;const bool runner=eligible&&SandboxZombies::RunnerWave(level,base,wave),louis=eligible&&SandboxZombies::LouisWave(level,base,wave);
  const bool cone=eligible&&SandboxZombies::ConeWrapWave(level,base,wave);
  const int clever=eligible?SandboxZombies::CleverWave(level,base,wave):-1;
  const int variant=eligible?SandboxZombies::ConeVariantWave(level,base,wave,water):-1;
  assert(a.id==(variant>=0?variant:clever>=0?clever:cone?214:runner?213:louis?212:0));assert(c.id==(louis?212:0));
  assert(a.id!=219||(!water&&level>=32&&wave%8==1));assert(a.id!=218||(level>=27&&wave%4==0));
  assert(base!=4||(a.id==0&&c.id==0)); // No clever bucket variant.
 }
 MemeAdventure::OnZombieSpawned(nullptr);Zombie preview;MemeAdventure::OnZombieSpawned(&preview);assert(preview.id==0);
 std::cout<<"Runner wave boundaries and at-most-one rule passed\\n";
}`);
 await run(process.env.CXX||'c++',['-std=c++20','-Isrc',cpp,'-o',binary],{cwd:root});
 assert.match((await run(binary)).stdout,/Runner wave boundaries and at-most-one rule passed/);
});
