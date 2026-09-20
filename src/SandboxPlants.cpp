// Original sandbox plants; per-instance skins leave original definitions and saves untouched.
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "SandboxPlants.h"
#include "Sandbox.h"
#include "LawnApp.h"
#include "Resources.h"
#include "Lawn/Board.h"
#include "Lawn/Plant.h"
#include "Lawn/SeedPacket.h"
#include "Lawn/System/ReanimationLawn.h"
#include "PvzpLib/Reanimator.h"
#include "PvzpLib/PvzpCommon.h"
#include "graphics/Graphics.h"
#include "graphics/GLImage.h"
#include <map>
#include <memory>
#include <string>
#include <string_view>

namespace SandboxPlants {
namespace {
struct State { int id,shots=0;bool closed=false; };
std::map<const Plant*,State> states;
std::map<std::string,std::unique_ptr<Sexy::GLImage>> images;
std::map<int,std::unique_ptr<Sexy::MemoryImage>> cards;
Sexy::Image* Image(const std::string& file) {
    auto& image=images[file];
    if(!image)image.reset(gLawnApp->GetImage("images/sandbox/"+file));
    return image.get();
}
void Skin(Reanimation* anim,int id,bool closed) {
    if(!anim)return;
    const auto* def=Find(id);if(!def)return;
    const bool ice=def->element!=Element::Fire;
    const std::string family=ice?"ice":"fire";
    const bool triple=def->base==18;
    auto* head=Image(family+(triple?"-small":"")+(closed?"-blink.png":"-head.png"));
    auto* mouth=Image((def->element==Element::Alternating?"fire":family)+(triple?"-small-mouth.png":"-mouth.png"));
    for(int i=0;i<anim->mDefinition->mTracks.count;++i) {
        const std::string_view name=anim->mDefinition->mTracks.tracks[i].mName;
        auto& track=anim->mTrackInstances[i];
        if(name.starts_with("anim_face"))track.mImageOverride=head;
        else if(name.find("mouth")!=name.npos && def->base!=40)track.mImageOverride=mouth;
        // The generated closed-head frame supplies the eyes; do not overlay green eye patches.
        else if(name.find("blink")!=name.npos)track.mRenderGroup=RENDER_GROUP_HIDDEN;
    }
}
void SkinPlant(Plant* plant,State& state) {
    for(auto id:{plant->mBodyReanimID,plant->mHeadReanimID,plant->mHeadReanimID2,plant->mHeadReanimID3})
        Skin(gLawnApp->ReanimationTryToGet(id),state.id,state.closed);
}
Sexy::MemoryImage* Card(int id) {
    auto& cached=cards[id];if(cached)return cached.get();
    const auto* def=Find(id);
    const auto seed=static_cast<SeedType>(def->base);
    const auto reanim=GetPlantDefinition(seed).mReanimationType;
    cached=gLawnApp->mReanimatorCache->MakeBlankMemoryImage(90,90);
    Sexy::Graphics graphics(cached.get());graphics.SetLinearBlend(true);
    auto frame=[&](const char* layer){
        Reanimation anim;anim.ReanimationInitializeType(5,8,reanim);anim.SetFramesForLayer(layer);
        Skin(&anim,id,false);anim.Draw(&graphics);
    };
    frame("anim_idle");
    if(def->base==18){frame("anim_head_idle1");frame("anim_head_idle3");frame("anim_head_idle2");}
    else frame("anim_head_idle");
    return cached.get();
}
}
void Reset(){states.clear();}
void Forget(Plant* plant){states.erase(plant);}
bool IsCustom(const Plant* plant){return gSandboxEnabled&&states.contains(plant);}
int Type(const Plant* plant){const auto it=states.find(plant);return it==states.end()?static_cast<int>(plant->mSeedType):it->second.id;}
void Assign(Plant* plant,int id){if(!Find(id))return;auto& state=states[plant];state={id};SkinPlant(plant,state);}
int NextShot(Plant* plant){
    if(!gSandboxEnabled)return 0;
    auto it=states.find(plant);if(it==states.end())return 0;
    return ShotElement(it->second.id,it->second.shots++)==Element::Ice?1:2;
}
void Tick(Board* board){
    for(auto* plant:board->mPlants){
        if(plant->mDead){Forget(plant);continue;}
        auto it=states.find(plant);if(it==states.end())continue;
        // Use the real randomized native blink countdown; it freezes with the simulation.
        const bool closed=plant->mBlinkCountdown>0&&plant->mBlinkCountdown<=8&&plant->mShootingCounter==0;
        if(closed!=it->second.closed){it->second.closed=closed;SkinPlant(plant,it->second);}
    }
}
void DrawCard(Sexy::Graphics* g,int x,int y,int id){
    const auto* def=Find(id);
    if(!def){DrawSeedPacket(g,x,y,static_cast<SeedType>(id),SEED_NONE,0,255,false,false);return;}
    PvzpDrawImageCelScaledF(g,Sexy::IMAGE_SEEDS,x,y,2,0,1,1);
    Sexy::Graphics clipped(*g);clipped.SetClipRect(x+3,y+9,44,48);
    clipped.DrawImage(Card(id),x+3,y+9,45,45);
    const char* tag=def->base==18?"3":def->base==40?"4":def->base==7?"2":"1";
    PvzpDrawString(g,tag,x+25,y+65,Sexy::FONT_BRIANNETOD12,Sexy::Color(55,64,23),DS_ALIGN_CENTER);
}
}
