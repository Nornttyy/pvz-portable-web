import test from 'node:test';
import assert from 'node:assert/strict';
import {readFile,writeFile,mkdtemp} from 'node:fs/promises';
import {execFile} from 'node:child_process';
import {promisify} from 'node:util';
import {tmpdir} from 'node:os';
import {join} from 'node:path';
import {fileURLToPath} from 'node:url';
const run=promisify(execFile),root=fileURLToPath(new URL('../',import.meta.url));
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
 assert(MemeCharacters::Definitions.size()==6);
 assert(Translate("ADVICE_QA","向日葵和双子向日葵") == "向日葵和双子向日葵");
 assert(Translate("SEED_CHOOSER_QA","豌豆射手、小喷菇、坚果墙") == "红温豌豆、小喷菇、反咬坚果");
 for(int seed=0;seed<54;++seed)if(seed!=0&&seed!=1&&seed!=3&&seed!=7&&seed!=40&&seed!=52)assert(!Replacement(seed,-1));
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

test('native adventure spawn hook limits runners and cone wraps to one per eligible wave, never previews',async()=>{
 const source=await readFile(join(root,'src/MemeAdventure.cpp'),'utf8');
 const hook=source.slice(source.indexOf('void OnZombieSpawned('),source.indexOf('void Draw(Board*'));
 const folder=await mkdtemp(join(tmpdir(),'pvz-runner-wave-')),cpp=join(folder,'test.cpp'),binary=join(folder,'test');
 await writeFile(cpp,`#include "SandboxZombies.h"
#include <vector>
#include <cassert>
#include <iostream>
class Zombie;
class Board {public:int mLevel=1;std::vector<Zombie*> mZombies;};
class Zombie {public:Board* mBoard=nullptr;int mZombieType=0,mFromWave=0,id=0;bool board=true;bool IsOnBoard(){return board;}};
namespace SandboxZombies {bool IsRunner(const Zombie* z){return z&&z->id==Runner;}bool IsConeWrap(const Zombie* z){return z&&z->id==ConeWrap;}void Assign(Zombie* z,int id){z->id=id;}}
namespace MemeAdventure {bool enabled=true;bool RosterEnabled(){return enabled;} ${hook}}
int main(){
 for(int level:{1,2,3,5,6,8,15,16,17,20,50})for(int wave=-3;wave<25;++wave)for(int base:{0,1,2,3,4,23})for(bool onBoard:{false,true})for(bool enabled:{false,true}){
  Board b;b.mLevel=level;Zombie a{&b,base,wave,0,onBoard},c{&b,base,wave,0,onBoard};b.mZombies={&a,&c};MemeAdventure::enabled=enabled;
  MemeAdventure::OnZombieSpawned(&a);MemeAdventure::OnZombieSpawned(&c);
  const bool eligible=enabled&&onBoard;const bool runner=eligible&&SandboxZombies::RunnerWave(level,base,wave),louis=eligible&&SandboxZombies::LouisWave(level,base,wave);
  const bool cone=eligible&&SandboxZombies::ConeWrapWave(level,base,wave);
  assert(a.id==(cone?214:runner?213:louis?212:0));assert(c.id==(louis?212:0));
 }
 MemeAdventure::OnZombieSpawned(nullptr);Zombie preview;MemeAdventure::OnZombieSpawned(&preview);assert(preview.id==0);
 std::cout<<"Runner wave boundaries and at-most-one rule passed\\n";
}`);
 await run(process.env.CXX||'c++',['-std=c++20','-Isrc',cpp,'-o',binary],{cwd:root});
 assert.match((await run(binary)).stdout,/Runner wave boundaries and at-most-one rule passed/);
});
