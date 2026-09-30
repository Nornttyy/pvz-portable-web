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
 assert(Translate("ADVICE_QA","向日葵和双子向日葵") == "已读不回花和双子向日葵");
 assert(Translate("SEED_CHOOSER_QA","豌豆射手、小喷菇、坚果墙") == "红温豌豆、显眼包蘑菇、反咬坚果");
 assert(Translate("FLAG_ZOMBIE","old")=="催更旗手"&&Translate("BUCKETHEAD_ZOMBIE","铁桶僵尸")=="铁桶僵尸");
 assert(Translate("BUCKETHEAD_ZOMBIE_DESCRIPTION","native")=="native");
 assert(Translate("GOLD_SUNFLOWER_TROPHY","金色向日葵奖杯") == "金色向日葵奖杯");
 for(bool* boundary:{&app.bowling,&app.pots,&app.whack,&gSandboxEnabled}){*boundary=true;assert(!Replacement(0,-1));assert(Translate("PEASHOOTER","original")=="original");*boundary=false;}
 app.adventure=false;assert(!Replacement(0,-1));app.adventure=true;gLawnApp=nullptr;assert(!Replacement(0,-1));
 std::cout<<"Native-slot mapping and localized text passed.\\n";
}`);
 await run(process.env.CXX||'c++',['-std=c++20','-Isrc',cpp,'-o',binary],{cwd:root});
 assert.match((await run(binary)).stdout,/Native-slot mapping and localized text passed/);
});
