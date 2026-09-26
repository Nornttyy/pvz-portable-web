// Garden-tech enemy roster. The native walk/eat skeleton stays in charge of
// motion; each technology enemy supplies its own rig parts, break states and
// a deliberately narrow interaction with the root-network system.
#include "SandboxZombies.h"
#include "SandboxPlants.h"
#include "SandboxCombatRules.h"
#include "SandboxArt.h"
#include "Sandbox.h"
#include "LawnApp.h"
#include "Resources.h"
#include "Lawn/Board.h"
#include "Lawn/Plant.h"
#include "Lawn/Zombie.h"
#include "Lawn/Projectile.h"
#include "Lawn/System/ReanimationLawn.h"
#include "PvzpLib/Reanimator.h"
#include "PvzpLib/PvzpCommon.h"
#include "graphics/Graphics.h"
#include "graphics/MemoryImage.h"
#include <algorithm>
#include <cmath>
#include <map>
#include <memory>
#include <string_view>
#include <vector>

namespace SandboxZombies {
namespace {
struct State {
    int id=0,age=0,burst=0,hookCooldown=0,repairFlash=0,windup=0,reload=100,field=0,decoyCooldown=0;
    bool hadArmor=true,decoyReady=true;
};
struct Shot { int id=0; };
struct Effect { float x=0,y=0; int row=0,kind=0,ticks=18; };

std::map<const Zombie*,State> states;
std::map<const Zombie*,int> resin;
std::map<const Projectile*,Shot> shots;
std::vector<Effect> effects;
std::map<int,std::unique_ptr<Sexy::MemoryImage>> portraits;

bool Alive(Zombie* z){return z&&!z->mMindControlled&&!z->IsDeadOrDying()&&z->mHasHead;}
bool Targetable(Plant* p){return p&&!p->mDead&&p->mPlantHealth>0&&int(p->mSeedType)!=16&&int(p->mSeedType)!=33&&int(p->mSeedType)!=35;}
void EffectAt(float x,float y,int row,int kind,int ticks=18){
    if(effects.size()>=180)effects.erase(effects.begin());
    effects.push_back({x,y,row,kind,ticks});
}
void Point(Zombie* z,const char* bone,float w,float h,float& x,float& y){
    float px=0,py=0;
    if(SandboxArt::TrackPoint(gLawnApp->ReanimationTryToGet(z->mBodyReanimID),bone,w,h,w*.5f,h*.5f,px,py)){
        x=z->mX+px;y=z->mY+py;
    }
}
int DamageStage(const Zombie* z){
    if(!z||z->mBodyMaxHealth<=0)return 0;
    return z->mBodyHealth<=z->mBodyMaxHealth/3?2:z->mBodyHealth<=z->mBodyMaxHealth*2/3?1:0;
}
const char* StagePart(int stage){
    return stage==2?"body-damage2":stage==1?"body-damage1":"body";
}
void Skin(Reanimation* anim,int id){
    if(!anim)return;const auto* d=Find(id);if(!d)return;
    for(int i=0;i<anim->mDefinition->mTracks.count;++i){
        const std::string_view name=anim->mDefinition->mTracks.tracks[i].mName;auto& track=anim->mTrackInstances[i];
        if(name=="anim_head1")track.mImageOverride=SandboxArt::Image(d->art,"head");
        else if(name=="Zombie_body")track.mImageOverride=SandboxArt::Image(d->art,"body");
        else if(name=="anim_head2")track.mImageOverride=SandboxArt::Image(d->art,"jaw");
        else if(name=="anim_cone"&&d->base==2)track.mImageOverride=SandboxArt::Image(d->art,"prop");
        else if(name=="anim_bucket"&&d->base==4)track.mImageOverride=SandboxArt::Image(d->art,"prop");
        // Each technology head already includes its own distinctive cap, wires
        // or headset.  Leaving the native hair visible was the source of the
        // obvious pasted-on layer seen in the old roster.
        else if(name=="anim_hair")track.mRenderGroup=RENDER_GROUP_HIDDEN;
        else if(name=="anim_innerarm1")track.mImageOverride=SandboxArt::Image(d->art,"inner-upper");
        else if(name=="anim_innerarm2")track.mImageOverride=SandboxArt::Image(d->art,"inner-lower");
        else if(name=="Zombie_outerarm_upper")track.mImageOverride=SandboxArt::Image(d->art,"outer-upper");
        else if(name=="Zombie_outerarm_lower")track.mImageOverride=SandboxArt::Image(d->art,"outer-lower");
        else if(name=="Zombie_outerarm_hand")track.mImageOverride=SandboxArt::Image(d->art,"hand");
        // Walking uses six distinct leg bones.  The tech roster supplies a
        // matching six-piece rig, so knees and feet follow the native walk
        // motion instead of a whole-leg recolour sliding underneath it.
        else if(name=="Zombie_innerleg_upper")track.mImageOverride=SandboxArt::Image(d->art,"inner-leg-upper");
        else if(name=="Zombie_innerleg_lower")track.mImageOverride=SandboxArt::Image(d->art,"inner-leg-lower");
        else if(name=="Zombie_innerleg_foot")track.mImageOverride=SandboxArt::Image(d->art,"inner-leg-foot");
        else if(name=="Zombie_outerleg_upper")track.mImageOverride=SandboxArt::Image(d->art,"outer-leg-upper");
        else if(name=="Zombie_outerleg_lower")track.mImageOverride=SandboxArt::Image(d->art,"outer-leg-lower");
        else if(name=="Zombie_outerleg_foot")track.mImageOverride=SandboxArt::Image(d->art,"outer-leg-foot");
        // The chest bone remains independent from the face, so packs and
        // antennae cannot ever stretch the jaw or obscure the muzzle.
        else if(name=="Zombie_tie"){
            // Turbine Boot has no chest pack: its two independently animated
            // turbines live on the legs, so hiding this legacy tie avoids a
            // fake accessory being pasted over the torso.
            if(d->id==202)track.mRenderGroup=RENDER_GROUP_HIDDEN;
            else track.mImageOverride=SandboxArt::Image(d->art,"pack");
        }
    }
}
void RepairArmor(Zombie* repairer){
    if(!repairer||!repairer->mBoard)return;
    Zombie* target=nullptr;int missing=0;
    for(auto* other:repairer->mBoard->mZombies){
        if(other==repairer||!Alive(other)||std::abs(other->mRow-repairer->mRow)>1||std::abs(other->mPosX-repairer->mPosX)>165||other->mHelmHealth<=0)continue;
        const int need=other->mHelmMaxHealth-other->mHelmHealth;
        if(need>missing){missing=need;target=other;}
    }
    if(target){
        target->mHelmHealth=SandboxCombatRules::Repair(target->mHelmHealth,target->mHelmMaxHealth);
        auto it=states.find(repairer);if(it!=states.end())it->second.repairFlash=36;
    }
}
void TryHookRoot(Board* board,Zombie* hooker){
    if(!board||!hooker)return;Plant* target=nullptr;float nearest=245;
    for(auto* p:board->mPlants){
        if(!Targetable(p)||SandboxPlants::Type(p)==103||std::abs(p->mRow-hooker->mRow)>1)continue;
        const auto root=SandboxPlants::RootNetwork(p);if(!root.linked)continue;
        const float distance=std::abs(p->mX+40-(hooker->mPosX+42));
        if(distance<nearest){nearest=distance;target=p;}
    }
    if(target&&HookRootCable(board,hooker,target))EffectAt(target->mX+40,target->mY+44,target->mRow,2,28);
}
}

void Reset(){states.clear();resin.clear();shots.clear();effects.clear();portraits.clear();}
void Forget(Zombie* z){states.erase(z);resin.erase(z);}
void ForgetPlant(Plant*){}
void ForgetShot(Projectile* p){shots.erase(p);}

bool IsMechanical(const Zombie* z){return z&&states.contains(z);}
bool IsResinSlowed(const Zombie* z){auto it=resin.find(z);return it!=resin.end()&&it->second>0;}
void ApplyResin(Zombie* z,int ticks){
    if(!z)return;
    resin[z]=std::max(resin[z],SandboxCombatRules::CappedTimer(ticks));
}
bool IsJamming(const Plant* relay){
    if(!relay||!relay->mBoard)return false;
    for(auto* z:relay->mBoard->mZombies){
        auto it=states.find(z);
        if(it!=states.end()&&it->second.id==207&&Alive(z)&&std::abs(z->mRow-relay->mRow)<=1&&std::abs(z->mPosX-(relay->mX+40))<190)return true;
    }
    return false;
}
bool HookRootCable(Board* board,Zombie* hooker,Plant* target){
    auto it=states.find(hooker);
    if(!board||!hooker||!target||it==states.end()||it->second.id!=203||it->second.hookCooldown>0)return false;
    if(!SandboxPlants::RootNetwork(target).linked)return false;
    if(!SandboxPlants::SeverRootCable(board,target,330))return false;
    it->second.hookCooldown=240;return true;
}
bool Intercept(Projectile* shot){
    if(!shot||!shot->mBoard)return false;
    for(auto* z:shot->mBoard->mZombies){
        auto it=states.find(z);
        if(it==states.end()||it->second.id!=208||!Alive(z)||z->mRow!=shot->mRow||std::abs(z->mPosX+44-shot->mPosX)>48)continue;
        it->second.field=42;EffectAt(z->mPosX+45,z->mPosY+54,z->mRow,7,22);return true;
    }
    return false;
}

void RefreshDamageArt(Zombie* z){
    auto it=states.find(z);if(it==states.end())return;auto* anim=gLawnApp->ReanimationTryToGet(z->mBodyReanimID);if(!anim)return;
    const auto* d=Find(it->second.id);if(!d)return;
    anim->SetImageOverride("Zombie_body",SandboxArt::Image(d->art,StagePart(DamageStage(z))));
    if(!z->mHasArm){
        // Keep a uniquely drawn torn shoulder stump on the upper-arm bone;
        // only the detached lower arm and hand disappear.  This preserves a
        // readable damage stage without allowing the intact limb to ghost
        // over the body after a bite.
        for(int i=0;i<anim->mDefinition->mTracks.count;++i){
            const std::string_view name=anim->mDefinition->mTracks.tracks[i].mName;
            if(name=="Zombie_outerarm_upper")anim->mTrackInstances[i].mImageOverride=SandboxArt::Image(d->art,"outer-upper-damaged");
            else if(name=="Zombie_outerarm_lower"||name=="Zombie_outerarm_hand")anim->mTrackInstances[i].mRenderGroup=RENDER_GROUP_HIDDEN;
        }
    }
    if(d->armor&&z->mHelmHealth>0){
        const int stage=z->mHelmMaxHealth>0?(z->mHelmHealth<z->mHelmMaxHealth/3?2:z->mHelmHealth<z->mHelmMaxHealth*2/3?1:0):0;
        anim->SetImageOverride(d->base==2?"anim_cone":"anim_bucket",SandboxArt::Image(d->art,stage==0?"prop":stage==1?"prop-damage1":"prop-damage2"));
    }
}
Sexy::Image* DetachedArmor(const Zombie* z){
    auto it=states.find(z);if(it==states.end())return nullptr;const auto* d=Find(it->second.id);
    return d&&d->armor?SandboxArt::Image(d->art,"prop-damage2"):nullptr;
}

void Assign(Zombie* z,int id){
    const auto* d=Find(id);if(!d||!z)return;states[z]={};states[z].id=id;
    z->mBodyHealth=z->mBodyMaxHealth=d->health;
    if(d->armor)z->mHelmHealth=z->mHelmMaxHealth=d->armor;
    z->mScaleZombie=SandboxCombatRules::Scale(id);
    Skin(gLawnApp->ReanimationTryToGet(z->mBodyReanimID),id);z->UpdateReanim();
}
float Speed(const Zombie* z){
    if(!gSandboxEnabled||!z||z->mMindControlled)return 1;
    auto it=states.find(z);if(it==states.end())return IsResinSlowed(z)?0.56f:1.0f;const auto& s=it->second;
    if(s.windup>0)return 0;
    float speed=SandboxCombatRules::Speed(s.id,z->mHelmHealth>0,s.burst,false);
    if(IsResinSlowed(z))speed*=0.56f;
    return speed;
}
int Damage(const Zombie* z,int damage){
    if(!gSandboxEnabled||damage<=0)return damage;auto it=states.find(z);if(it==states.end())return damage;
    auto& s=it->second;
    if(s.id==209&&s.decoyReady&&s.age>=45){
        s.decoyReady=false;s.decoyCooldown=330;EffectAt(z->mPosX+46,z->mPosY+42,z->mRow,8,28);return 0;
    }
    return damage;
}
bool ArcHit(Zombie* z,int damage){
    auto it=states.find(z);if(it==states.end()||it->second.id!=200||damage<=0)return false;
    // The leaking battery is a conductance target; one arc gets a high-impact
    // strike, while other mechanical enemies retain their own counterplay.
    z->TakeDamage(damage*2,0);it->second.field=38;
    if(z->IsDeadOrDying())SandboxPlants::OnMechanicalCoreDestroyed(z->mBoard,z);
    return true;
}
void CombatDeath(Zombie* z){
    if(gSandboxEnabled&&IsMechanical(z)&&z&&z->mBoard)SandboxPlants::OnMechanicalCoreDestroyed(z->mBoard,z);
}

bool HasShot(const Projectile* p){return shots.contains(p);}
Plant* CollisionTarget(Projectile* shot){
    if(!shot||!shot->mBoard)return nullptr;Plant* nearest=nullptr;
    for(auto* p:shot->mBoard->mPlants){
        if(!Targetable(p)||p->mRow!=shot->mRow||shot->mPosX+24<p->mX||shot->mPosX>p->mX+80)continue;
        if(!nearest||p->mX>nearest->mX||(p->mX==nearest->mX&&int(p->mSeedType)==30))nearest=p;
    }
    return nearest;
}
bool Impact(Projectile* shot,Plant* plant){
    auto it=shots.find(shot);if(it==shots.end()||!plant)return false;
    plant->mPlantHealth-=24;plant->mEatenFlashCountdown=std::max(plant->mEatenFlashCountdown,25);
    EffectAt(shot->mPosX+12,shot->mPosY+shot->mPosZ+12,shot->mRow,4);return true;
}
bool DrawShot(Sexy::Graphics* g,const Projectile* p){
    auto it=shots.find(p);if(it==shots.end())return false;
    SandboxArt::Sprite(g,"bolt-shot",p->mPosX-p->mX+12,p->mPosY+p->mPosZ-p->mY+12,25,15,p->mProjectileAge*.05f);return true;
}

void Tick(Board* board){
    if(!board||board->mPaused)return;
    for(auto it=resin.begin();it!=resin.end();){
        auto* z=const_cast<Zombie*>(it->first);if(!Alive(z)||--it->second<=0)it=resin.erase(it);else ++it;
    }
    for(auto it=effects.begin();it!=effects.end();){if(--it->ticks<=0)it=effects.erase(it);else ++it;}
    for(auto* z:board->mZombies){
        auto it=states.find(z);if(it==states.end()||!Alive(z))continue;auto& s=it->second;++s.age;
        if(s.burst>0)--s.burst;if(s.hookCooldown>0)--s.hookCooldown;if(s.repairFlash>0)--s.repairFlash;if(s.field>0)--s.field;
        if(s.decoyCooldown>0&&--s.decoyCooldown==0)s.decoyReady=true;
        RefreshDamageArt(z);
        if(s.id==202&&s.age%420==0){s.burst=96;EffectAt(z->mPosX+42,z->mPosY+64,z->mRow,1,26);}
        if(s.id==203&&s.hookCooldown==0&&s.age%90==0)TryHookRoot(board,z);
        if(s.id==204&&s.age%300==0)RepairArmor(z);
        if(s.id!=205)continue;
        Plant* target=nullptr;float nearest=SandboxCombatRules::RangedRange(s.id);
        for(auto* p:board->mPlants){
            if(!Targetable(p)||p->mRow!=z->mRow)continue;const float distance=z->mPosX+30-(p->mX+40);
            if(distance>=80&&distance<nearest){nearest=distance;target=p;}
        }
        const bool interrupted=z->mIceTrapCounter>0||z->mButteredCounter>0||!z->mHasArm||z->mIsEating;
        if(interrupted||!target){
            if(s.windup>0){s.windup=0;s.reload=90;if(!z->mIsEating)z->StartWalkAnim(10);}
            continue;
        }
        if(s.reload>0)--s.reload;
        if(s.windup>0){
            if(--s.windup==0){
                if(board->mProjectiles.mSize<board->mProjectiles.mMaxSize-8){
                    float x=z->mX+25,y=z->mY+65;Point(z,"Zombie_outerarm_hand",25,27,x,y);
                    if(auto* shot=board->AddProjectile(x-12,y-12,z->mRenderOrder,z->mRow,PROJECTILE_ZOMBIE_PEA)){
                        shot->mMotionType=MOTION_BACKWARDS;shots[shot]={s.id};
                    }
                }
                s.reload=SandboxCombatRules::RangedRate(s.id);z->StartWalkAnim(10);
            }
        }else if(s.reload==0){
            s.windup=SandboxCombatRules::AimTicks(s.id);
            if(auto* anim=gLawnApp->ReanimationTryToGet(z->mBodyReanimID))anim->PlayReanim("anim_eat",REANIM_PLAY_ONCE_AND_HOLD,10,12.0f);
        }
    }
}

void DrawPortrait(Sexy::Graphics* g,int x,int y,int w,int h,int id){
    const auto* d=Find(id);if(!d)return;auto& cached=portraits[id];
    if(!cached){
        cached=gLawnApp->mReanimatorCache->MakeBlankMemoryImage(220,240);Sexy::Graphics canvas(cached.get());canvas.SetLinearBlend(true);
        Reanimation anim;anim.ReanimationInitializeType(50,50,REANIM_ZOMBIE);anim.SetFramesForLayer("anim_idle");
        Zombie::SetupReanimLayers(&anim,static_cast<ZombieType>(d->base));Skin(&anim,id);anim.Draw(&canvas);
    }
    SandboxArt::DrawFit(g,cached.get(),x+5,y+3,w-10,h-6,d->id==206?0.92f:1.0f);
}
void DrawEffects(Sexy::Graphics* graphics,Board* board,int row){
    if(!graphics||!board)return;Sexy::Graphics clipped(*graphics);clipped.ClipRect(0,82,800,518);auto* g=&clipped;
    for(const auto& effect:effects)if(effect.row==row){
        const char* art=effect.kind==1?"turbine-burst":effect.kind==2?"hook-snap":effect.kind==4?"bolt-hit-0":effect.kind==7?"magnet-field":effect.kind==8?"holo-glitch":"tech-spark";
        SandboxArt::Sprite(g,art,effect.x,effect.y,34,30,0,std::min(240,effect.ticks*12));
    }
    for(const auto& [raw,s]:states){
        auto* z=const_cast<Zombie*>(raw);if(!Alive(z)||z->mRow!=row)continue;
        float hx=z->mX+45,hy=z->mY+30;Point(z,"anim_head1",53,48,hx,hy);
        float bx=z->mX+45,by=z->mY+65;Point(z,"Zombie_body",53,63,bx,by);
        if(s.id==200&&s.field>0)SandboxArt::Sprite(g,"battery-core",bx,by,30,30,0,170);
        if(s.id==202&&s.burst>0)SandboxArt::Sprite(g,"turbine-burst",bx-20,by+23,48,22,0,160);
        if(s.id==203&&s.hookCooldown>210)SandboxArt::Sprite(g,"hook-line",hx-25,hy+20,48,16,0,180);
        if(s.id==204&&s.repairFlash>0)SandboxArt::Sprite(g,"repair-pulse",hx+18,hy-6,27,27,0,s.repairFlash*6);
        if(s.id==205&&s.windup>0){
            float x=z->mX+25,y=z->mY+65;Point(z,"Zombie_outerarm_hand",25,27,x,y);
            SandboxArt::Sprite(g,"bolt-ready",x,y,15,15,0,200);
        }
        if(s.id==206)SandboxArt::Sprite(g,"drone-hum",bx,by+22,46,16,0,130);
        if(s.id==207)SandboxArt::Sprite(g,"jam-wave",hx,hy-20,48,32,0,135);
        if(s.id==208&&s.field>0)SandboxArt::Sprite(g,"magnet-field",bx,by,55,55,0,155);
        if(s.id==209&&s.decoyReady)SandboxArt::Sprite(g,"holo-ready",hx+18,hy-18,25,25,0,130);
    }
}
}
