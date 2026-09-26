// Garden-tech sandbox roster. Every state belongs to a placed instance; adventure plants and native animation definitions stay untouched.
#include "SandboxPlants.h"
#include "SandboxZombies.h"
#include "SandboxArt.h"
#include "SandboxVisualRules.h"
#include "Sandbox.h"
#include "LawnApp.h"
#include "Resources.h"
#include "Lawn/Board.h"
#include "Lawn/Plant.h"
#include "Lawn/Zombie.h"
#include "Lawn/Projectile.h"
#include "Lawn/SeedPacket.h"
#include "Lawn/System/ReanimationLawn.h"
#include "PvzpLib/Reanimator.h"
#include "PvzpLib/PvzpCommon.h"
#include "graphics/Graphics.h"
#include "graphics/MemoryImage.h"
#include <algorithm>
#include <cmath>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace SandboxPlants {
namespace {
constexpr int kMaxNetworkMembers=6;
constexpr int kMaxCharge=3;
constexpr int kResonanceTicks=240;

struct State {
    int id=0,shots=0,age=0,cooldown=0,health=0,hits=0;
    int network=-1,charge=0,resonance=0,pulseSerial=0,severed=0;
    bool closed=false;
};
struct Shot { int id=0,damage=0,hops=1; Plant* source=nullptr; bool powered=false; };
struct Beam { float x1=0,y1=0,x2=0,y2=0; int ticks=0,row=0,kind=0; };
struct Effect { float x=0,y=0; int row=0,kind=0,ticks=18; bool muzzle=false; };
struct Network { Plant* hub=nullptr; std::vector<Plant*> members; bool jammed=false; };

std::map<const Plant*,State> states;
std::map<const Projectile*,Shot> shots;
std::map<int,std::unique_ptr<Sexy::MemoryImage>> cards;
std::vector<Network> networks;
std::vector<Beam> beams;
std::vector<Effect> effects;
std::map<ZombieID,bool> harvestedCores;

bool Alive(const Plant* p){return p&&!p->mDead&&p->mPlantHealth>0;}
bool Enemy(Zombie* z){return z&&!z->mMindControlled&&!z->IsDeadOrDying()&&z->mHasHead;}
bool Orthogonal(const Plant* a,const Plant* b){return a&&b&&std::abs(a->mPlantCol-b->mPlantCol)+std::abs(a->mRow-b->mRow)==1;}
bool HasState(const Plant* p){return states.contains(p);}
bool IsHub(const Plant* p){auto i=states.find(p);return i!=states.end()&&i->second.id==103;}
void EffectAt(float x,float y,int row,int kind,bool muzzle=false){
    if(effects.size()>=256)effects.erase(effects.begin());
    effects.push_back({x,y,row,kind,muzzle?10:18,muzzle});
}
const char* ProjectileArt(int id){
    switch(id){case 101:return "pulse";case 104:return "arc";case 105:return "magnet";case 106:return "resin";case 107:return "scout";default:return "pulse";}
}
const char* ImpactArt(int id,int frame){
    switch(id){
    case 101:return frame?"pulse-hit-1":"pulse-hit-0";case 104:return frame?"arc-hit-1":"arc-hit-0";
    case 105:return frame?"magnet-hit-1":"magnet-hit-0";case 106:return frame?"resin-hit-1":"resin-hit-0";
    case 107:return frame?"scout-hit-1":"scout-hit-0";default:return frame?"pulse-hit-1":"pulse-hit-0";
    }
}
void PlantPoint(const Plant* p,const char* track,float w,float h,float& x,float& y){
    float px=0,py=0;
    if(SandboxArt::TrackPoint(gLawnApp->ReanimationTryToGet(p->mBodyReanimID),track,w,h,w*.5f,h*.5f,px,py)){x=p->mX+px;y=p->mY+py;}
}
bool Muzzle(const Plant* p,int row,float& x,float& y){
    const auto* d=Find(Type(p));if(!d)return false;
    // The native Peashooter/Threepeater mouth pivots have a visible gap from
    // their heads during both idle and firing frames.  Technology Garden's
    // living nozzles are therefore part of the custom head silhouette; read
    // the live face bone's front edge so a projectile never starts in a
    // detached legacy mouth layer.
    auto reanim=p->mHeadReanimID;const char* track="anim_face";float w=70,h=65;
    if(d->base==18){
        const int head=SandboxVisualRules::HeadForRow(p->mRow,row);
        reanim=head==1?p->mHeadReanimID:head==2?p->mHeadReanimID2:p->mHeadReanimID3;
        track=head==1?"anim_face1":head==2?"anim_face2":"anim_face3";w=30;h=30;
    }
    float localX=0,localY=0;
    if(!SandboxArt::TrackPoint(gLawnApp->ReanimationTryToGet(reanim),track,w,h,w-3,h*.5f,localX,localY))return false;
    x=p->mX+localX;y=p->mY+localY;return true;
}
void Skin(Reanimation* anim,int id,bool closed,int health){
    if(!anim)return;const auto* d=Find(id);if(!d)return;
    for(int i=0;i<anim->mDefinition->mTracks.count;++i){
        const std::string_view name=anim->mDefinition->mTracks.tracks[i].mName;auto& track=anim->mTrackInstances[i];
        if(d->base==1){
            if(name=="anim_idle")track.mImageOverride=SandboxArt::Image(d->art,closed?"blink":"head");
            else if(name.find("blink")!=name.npos)track.mRenderGroup=RENDER_GROUP_HIDDEN;
        }else if(d->base==3){
            if(name=="anim_face")track.mImageOverride=SandboxArt::Image(d->art,health<1200?"cracked2":health<2600?"cracked1":"head");
            else if(name.find("blink")!=name.npos)track.mRenderGroup=RENDER_GROUP_HIDDEN;
        }else if(d->base==8){
            if(name=="anim_face")track.mImageOverride=SandboxArt::Image(d->art,closed?"blink":"head");
            if(name=="PuffShroom_head")track.mImageOverride=SandboxArt::Image(d->art,"cap");
            if(name=="PuffShroom_stem")track.mImageOverride=SandboxArt::Image(d->art,"stem");
            if(name=="PuffShroom_eyes"||name.find("blink")!=name.npos)track.mRenderGroup=RENDER_GROUP_HIDDEN;
        }else{
            const bool triple=d->base==18;
            if(name=="anim_face"||name=="anim_face1"||name=="anim_face2"||name=="anim_face3")track.mImageOverride=SandboxArt::Image(d->art,triple?(closed?"small-blink":"small-head"):(closed?"blink":"head"));
            // Never show a legacy mouth bone under a full custom head: their
            // native pivots separate during firing and produced the floating
            // nozzle/transparent-hole regression.  Unique muzzle, beam and
            // hit sprites remain drawn by the combat effect layer.
            else if(name=="idle_mouth"||name=="ThreePeater_mouth1"||name=="ThreePeater_mouth2"||name=="ThreePeater_mouth3")track.mRenderGroup=RENDER_GROUP_HIDDEN;
            else if(name.find("blink")!=name.npos)track.mRenderGroup=RENDER_GROUP_HIDDEN;
        }
    }
}
void SkinPlant(Plant* p,State& s){for(auto id:{p->mBodyReanimID,p->mHeadReanimID,p->mHeadReanimID2,p->mHeadReanimID3})Skin(gLawnApp->ReanimationTryToGet(id),s.id,s.closed,p->mPlantHealth);}
void DirectDamage(Plant* source,Zombie* target,int damage){
    if(!source||!Enemy(target)||damage<=0)return;
    target->TakeDamage(damage,source->GetDamageRangeFlags(WEAPON_PRIMARY));
    auto i=states.find(source);if(i!=states.end()&&++i->second.hits%4==0)AddRootEnergy(source->mBoard,source,RootEnergySource::Damage);
    if(target->IsDeadOrDying()&&SandboxZombies::IsMechanical(target))OnMechanicalCoreDestroyed(source->mBoard,target);
}
void AddShot(Projectile* p,Plant* source,int id,int damage,bool powered){shots[p]={id,damage,id==104?(powered?5:3):1,source,powered};}
std::vector<Plant*> Nearby(Board* board,const Plant* p){
    std::vector<Plant*> result;if(!board||!p)return result;
    for(auto* other:board->mPlants)if(Alive(other)&&HasState(other)&&Orthogonal(p,other))result.push_back(other);return result;
}
}

void Reset(){states.clear();shots.clear();cards.clear();networks.clear();beams.clear();effects.clear();harvestedCores.clear();}
void Forget(Plant* p){states.erase(p);}
void ForgetShot(Projectile* p){shots.erase(p);}
bool IsCustom(const Plant* p){return gSandboxEnabled&&states.contains(p);}
bool IsTech(const Plant* p){return IsCustom(p);}
int Type(const Plant* p){auto i=states.find(p);return i==states.end()?int(p->mSeedType):i->second.id;}
bool IsResonating(const Plant* p){
    auto i=states.find(p);if(i==states.end()||i->second.network<0||i->second.network>=int(networks.size()))return false;
    auto h=states.find(networks[i->second.network].hub);return h!=states.end()&&h->second.resonance>0;
}
void Assign(Plant* p,int id){
    const auto* d=Find(id);if(!d)return;auto& s=states[p];s={};s.id=id;s.health=p->mPlantHealth;
    if(d->rate){p->mLaunchRate=d->rate;p->mLaunchCounter=std::min(p->mLaunchCounter,d->rate);}
    if(d->role==Role::Relay||d->role==Role::Prism||d->role==Role::Repair)p->mLaunchCounter=9999;SkinPlant(p,s);
}
void RebuildRootNetworks(Board* board){
    networks.clear();for(auto& [plant,state]:states)state.network=-1;if(!board)return;
    std::vector<Plant*> hubs;for(auto* p:board->mPlants)if(Alive(p)&&IsHub(p))hubs.push_back(p);
    std::sort(hubs.begin(),hubs.end(),[](const Plant* a,const Plant* b){return a->mRow==b->mRow?a->mPlantCol<b->mPlantCol:a->mRow<b->mRow;});
    for(auto* hub:hubs){
        auto h=states.find(hub);if(h==states.end()||h->second.severed>0||h->second.network>=0)continue;
        Network net;net.hub=hub;net.members.push_back(hub);h->second.network=int(networks.size());
        for(size_t cursor=0;cursor<net.members.size()&&net.members.size()<kMaxNetworkMembers;++cursor){
            Plant* from=net.members[cursor];for(auto* next:Nearby(board,from)){
                auto n=states.find(next);if(n==states.end()||n->second.severed>0||n->second.network>=0||IsHub(next))continue;
                n->second.network=int(networks.size());net.members.push_back(next);if(net.members.size()>=kMaxNetworkMembers)break;
            }
        }
        net.jammed=SandboxZombies::IsJamming(hub);networks.push_back(std::move(net));
    }
}
RootNetworkView RootNetwork(const Plant* p){
    RootNetworkView view;auto i=states.find(p);if(i==states.end())return view;const auto& s=i->second;view.network=s.network;view.severed=s.severed>0;
    if(s.network<0||s.network>=int(networks.size()))return view;const auto& net=networks[s.network];auto h=states.find(net.hub);if(h==states.end())return view;
    view.members=int(net.members.size());view.linked=view.members>1;view.charge=h->second.charge;view.activeTicks=h->second.resonance;view.pulseSerial=h->second.pulseSerial;view.jammed=net.jammed;return view;
}
bool RootLinked(const Plant* a,const Plant* b){const auto av=RootNetwork(a),bv=RootNetwork(b);return av.linked&&bv.linked&&av.network>=0&&av.network==bv.network;}
bool AddRootEnergy(Board* board,Plant* source,RootEnergySource sourceType,int amount){
    (void)sourceType;if(!board||board->mPaused||!Alive(source)||amount<=0)return false;RebuildRootNetworks(board);const auto view=RootNetwork(source);
    if(!view.linked||view.jammed||view.network<0)return false;auto h=states.find(networks[view.network].hub);if(h==states.end())return false;h->second.charge=std::min(kMaxCharge,h->second.charge+amount);return true;
}
RootTriggerResult TriggerRootResonance(Board* board,Plant* hub){
    if(!board||board->mPaused)return RootTriggerResult::Paused;if(!IsHub(hub))return RootTriggerResult::NotHub;RebuildRootNetworks(board);const auto view=RootNetwork(hub);
    if(!view.linked||view.charge<kMaxCharge)return RootTriggerResult::NotFull;auto h=states.find(hub);if(h==states.end())return RootTriggerResult::NotHub;
    h->second.charge=0;h->second.resonance=kResonanceTicks;++h->second.pulseSerial;EffectAt(hub->mX+42,hub->mY+38,hub->mRow,5);return RootTriggerResult::Fired;
}
bool SeverRootCable(Board* board,Plant* member,int ticks){
    auto i=states.find(member);if(!board||i==states.end()||i->second.id==103||ticks<=0)return false;i->second.severed=std::max(i->second.severed,ticks);RebuildRootNetworks(board);return true;
}
bool RepairRootCable(Board* board,Plant* repairer,Plant* member){
    auto r=states.find(repairer),m=states.find(member);if(!board||r==states.end()||m==states.end()||r->second.id!=109||m->second.severed<=0||!Orthogonal(repairer,member))return false;
    m->second.severed=0;RebuildRootNetworks(board);return true;
}
void OnMechanicalCoreDestroyed(Board* board,Zombie* zombie){
    if(!board||!zombie||!SandboxZombies::IsMechanical(zombie))return;const auto id=board->ZombieGetID(zombie);if(harvestedCores.contains(id))return;harvestedCores[id]=true;
    Plant* best=nullptr;float distance=100000;for(auto* p:board->mPlants)if(Alive(p)&&IsTech(p)){const float d=std::abs(p->mX-zombie->mPosX)+std::abs(p->mY-zombie->mPosY);if(d<distance){distance=d;best=p;}}
    if(best&&distance<230)AddRootEnergy(board,best,RootEnergySource::ScrapCore);
}
int ActivateNetwork(Board* board,int col,int row){
    if(!board)return 0;Plant* hub=nullptr;for(auto* p:board->mPlants)if(Alive(p)&&p->mPlantCol==col&&p->mRow==row&&IsHub(p)){hub=p;break;}if(!hub)return 0;
    const auto result=TriggerRootResonance(board,hub);
    if(result==RootTriggerResult::Fired)return 1;
    if(result==RootTriggerResult::Paused)return -3;
    if(result==RootTriggerResult::NotFull&&RootNetwork(hub).jammed)return -2;
    return -1;
}
int NetworkCharge(const Plant* relay){return RootNetwork(relay).charge;}
void AdjustScale(const Plant*,float&,float&,float&,float&){}
void AdjustShadow(const Plant*,float&,float&,float&){}
float ShotScale(const Projectile* p){auto i=shots.find(p);return i==shots.end()?1.0f:i->second.id==106?1.15f:1.0f;}
bool HasShot(const Projectile* p){return shots.contains(p);}
int ShotRadius(const Projectile* p){auto i=shots.find(p);return i==shots.end()?12:SandboxVisualRules::TechShotRadius(i->second.id);}
int NextShot(Plant*){return 0;}
void OnFired(Plant* p,Projectile* shot,Zombie* target){
    if(!gSandboxEnabled||!p||!shot)return;const auto* d=Find(Type(p));if(!d||d->role==Role::Prism||d->role==Role::Relay||d->role==Role::Repair)return;
    const bool powered=IsResonating(p);const int damage=d->damage+(powered?6:0);AddShot(shot,p,d->id,damage,powered);float mx=shot->mPosX+12,my=shot->mPosY+12;
    if(Muzzle(p,shot->mRow,mx,my)){shot->mPosX=mx-SandboxVisualRules::PeaCenter;shot->mPosY=my-SandboxVisualRules::PeaCenter-shot->mPosZ;}
    shot->mX=int(shot->mPosX);shot->mY=int(shot->mPosY+shot->mPosZ);EffectAt(shot->mPosX+12,shot->mPosY+shot->mPosZ+12,p->mRow,d->id,true);
    if(d->role==Role::Scout&&target){shot->mMotionType=MOTION_HOMING;shot->mTargetZombieID=p->mBoard->ZombieGetID(target);shot->mVelX=3.1f;}
    if(d->role==Role::Shooter&&powered)for(int extra=0;extra<2&&p->mBoard->mProjectiles.mSize<p->mBoard->mProjectiles.mMaxSize-8;++extra){
        auto* clone=p->mBoard->AddProjectile(shot->mPosX-extra*7,shot->mPosY+(extra?4:-4),shot->mRenderOrder,shot->mRow,PROJECTILE_PEA);
        if(clone){clone->mDamageRangeFlags=shot->mDamageRangeFlags;clone->mVelX=3.3f+extra*.25f;AddShot(clone,p,d->id,damage,true);}
    }
}
void UpdateShot(Projectile* p){
    auto i=shots.find(p);if(i==shots.end()||p->mDead)return;if(SandboxZombies::Intercept(p)){p->Die();return;}if(i->second.id!=107)return;
    auto* current=p->mBoard->ZombieTryToGet(p->mTargetZombieID);if(current&&Enemy(current))return;Zombie* best=nullptr;float distance=100000;
    for(auto* z:p->mBoard->mZombies)if(Enemy(z)&&z->EffectedByDamage(p->mDamageRangeFlags)){const float d=std::abs(z->mPosX-p->mPosX)+std::abs(z->mPosY-p->mPosY);if(d<distance){distance=d;best=z;}}
    if(best)p->mTargetZombieID=p->mBoard->ZombieGetID(best);else{p->mMotionType=MOTION_STRAIGHT;p->mVelX=3.3f;p->mVelY=0;}
}
bool Impact(Projectile* p,Zombie* target){
    auto i=shots.find(p);if(i==shots.end())return false;const auto shot=i->second;EffectAt(p->mPosX+12,p->mPosY+p->mPosZ+12,target?target->mRow:p->mRow,shot.id);if(!target)return true;
    if(shot.id==104){
        std::vector<ZombieID> hit;Zombie* from=target;int damage=shot.damage;
        for(int hop=0;hop<shot.hops&&from;++hop){
            hit.push_back(p->mBoard->ZombieGetID(from));if(!SandboxZombies::ArcHit(from,damage))DirectDamage(shot.source,from,damage);
            Zombie* next=nullptr;float distance=150;const float x=from->mPosX+55,y=from->mPosY+55;
            for(auto* z:p->mBoard->mZombies)if(Enemy(z)&&z->EffectedByDamage(p->mDamageRangeFlags)&&std::find(hit.begin(),hit.end(),p->mBoard->ZombieGetID(z))==hit.end()){
                const float d=std::hypot(z->mPosX+55-x,z->mPosY+55-y);if(d<distance){distance=d;next=z;}}
            if(next&&hop+1<shot.hops)beams.push_back({x,y,next->mPosX+55,next->mPosY+55,16,from->mRow,0});from=next;damage=std::max(6,damage-4);
        }
    }else if(shot.id==105){
        if(target->mHelmHealth>0)target->mHelmHealth=std::max(0,target->mHelmHealth-(shot.powered?65:38));DirectDamage(shot.source,target,shot.damage);
    }else{DirectDamage(shot.source,target,shot.damage);if(shot.id==106)SandboxZombies::ApplyResin(target,shot.powered?300:180);}
    return true;
}
void Tick(Board* board){
    if(!board||board->mPaused)return;for(auto it=beams.begin();it!=beams.end();)if(--it->ticks<=0)it=beams.erase(it);else ++it;for(auto it=effects.begin();it!=effects.end();)if(--it->ticks<=0)it=effects.erase(it);else ++it;
    for(auto it=states.begin();it!=states.end();){
        Plant* p=const_cast<Plant*>(it->first);if(!Alive(p)){it=states.erase(it);continue;}auto& s=it->second;++s.age;
        const bool closed=p->mBlinkCountdown>0&&p->mBlinkCountdown<=8&&p->mShootingCounter==0;if(closed!=s.closed||s.health!=p->mPlantHealth){s.closed=closed;SkinPlant(p,s);}s.health=p->mPlantHealth;if(s.severed>0)--s.severed;++it;
    }
    RebuildRootNetworks(board);
    for(auto& net:networks){
        auto h=states.find(net.hub);if(h==states.end())continue;auto& hub=h->second;if(hub.resonance>0)--hub.resonance;
        int generators=0;for(auto* p:net.members)if(Type(p)==100)++generators;
        if(!net.jammed&&net.members.size()>1&&generators>0&&hub.age%210==0)hub.charge=std::min(kMaxCharge,hub.charge+generators);
        if(hub.resonance>0)for(auto* p:net.members){auto s=states.find(p);if(s==states.end())continue;
            if((s->second.id==101||s->second.id==104||s->second.id==105||s->second.id==106||s->second.id==107)&&p->mLaunchCounter>1&&board->mMainCounter%2==0)--p->mLaunchCounter;
            if(s->second.id==102&&board->mMainCounter%24==0)p->mPlantHealth=std::min(p->mPlantMaxHealth,p->mPlantHealth+18);
        }
    }
    for(auto& [raw,s]:states){
        auto* p=const_cast<Plant*>(raw);if(!Alive(p)||p->mIsAsleep)continue;
        if(s.id==102){
            if(s.cooldown>0)--s.cooldown;if(s.cooldown==0)for(auto* z:board->mZombies)if(Enemy(z)&&z->mRow==p->mRow&&z->mPosX>p->mX-6&&z->mPosX<p->mX+105){DirectDamage(p,z,IsResonating(p)?38:18);s.cooldown=90;EffectAt(p->mX+45,p->mY+48,p->mRow,4);break;}
        }else if(s.id==108){
            p->mLaunchCounter=9999;if(s.cooldown>0)--s.cooldown;Zombie* target=nullptr;for(auto* z:board->mZombies)if(Enemy(z)&&z->mRow==p->mRow&&z->mPosX>p->mX){target=z;break;}
            if(target&&s.cooldown==0){float x=p->mX+48,y=p->mY+32;Muzzle(p,p->mRow,x,y);beams.push_back({x,y,780,y,26,p->mRow,1});const int damage=IsResonating(p)?68:36;for(auto* z:board->mZombies)if(Enemy(z)&&z->mRow==p->mRow&&z->mPosX>=x)DirectDamage(p,z,damage);s.cooldown=IsResonating(p)?180:310;EffectAt(x,y,p->mRow,8,true);}
        }else if(s.id==109){
            p->mLaunchCounter=9999;if(s.age%150==0)for(auto* other:Nearby(board,p)){other->mPlantHealth=std::min(other->mPlantMaxHealth,other->mPlantHealth+35);RepairRootCable(board,p,other);}
        }else if(s.id==103)p->mLaunchCounter=9999;
    }
}
void DrawEffects(Sexy::Graphics* graphics,Board* board,int row){
    Sexy::Graphics clipped(*graphics);clipped.ClipRect(0,82,800,518);auto* g=&clipped;
    for(const auto& net:networks){
        auto h=states.find(net.hub);if(h==states.end()||net.hub->mRow!=row)continue;const auto& s=h->second;
        for(size_t i=1;i<net.members.size();++i){auto* p=net.members[i];auto* nearest=net.hub;int best=999;for(size_t j=0;j<i;++j){auto* q=net.members[j];const int d=std::abs(p->mPlantCol-q->mPlantCol)+std::abs(p->mRow-q->mRow);if(d<best){best=d;nearest=q;}}if(p->mRow==row||nearest->mRow==row)SandboxArt::Link(g,nearest->mX+42,nearest->mY+60,p->mX+42,p->mY+60,board->mMainCounter/8,net.jammed?80:s.resonance>0?220:120);}
        const int lit=std::clamp(s.charge,0,kMaxCharge);for(int i=0;i<kMaxCharge;++i)SandboxArt::Sprite(g,i<lit?"root-charge-on":"root-charge-off",net.hub->mX+29+i*13,net.hub->mY+20,10,10,0,s.resonance>0?255:190);
        if(s.resonance>0)SandboxArt::Sprite(g,"root-resonance",net.hub->mX+42,net.hub->mY+42,54,54,0,std::min(230,s.resonance*2));
    }
    for(const auto& beam:beams)if(beam.row==row){if(beam.kind==0)SandboxArt::Link(g,beam.x1,beam.y1,beam.x2,beam.y2,board->mMainCounter/4,std::min(230,beam.ticks*25));else SandboxArt::Sprite(g,"prism-beam",(beam.x1+beam.x2)*.5f,beam.y1,std::abs(beam.x2-beam.x1),18,0,std::min(230,beam.ticks*20));}
    for(const auto& e:effects)if(e.row==row){const int age=(e.muzzle?10:18)-e.ticks;const float grow=e.muzzle?1:0.7f+age/36.0f;
        if(e.kind==5)SandboxArt::Sprite(g,"root-resonance",e.x,e.y,58*grow,58*grow,0,std::min(240,e.ticks*30));else if(e.kind==8)SandboxArt::Sprite(g,"prism-muzzle",e.x,e.y,24,22,0,std::min(240,e.ticks*30));else if(e.kind==4)SandboxArt::Sprite(g,"thorn-hit",e.x,e.y,36*grow,32*grow,0,std::min(230,e.ticks*25));else if(e.muzzle)SandboxArt::Sprite(g,"tech-muzzle",e.x,e.y,20,16,0,std::min(230,e.ticks*30));else SandboxArt::Sprite(g,ImpactArt(e.kind,age>=8?0:1),e.x,e.y,36*grow,36*grow,0,std::min(230,e.ticks*25));}
}
bool DrawShot(Sexy::Graphics* g,const Projectile* p){
    auto i=shots.find(p);if(i==shots.end())return false;const auto& shot=i->second;const float angle=p->mMotionType==MOTION_HOMING?std::atan2(p->mVelY,p->mVelX):0;const float size=shot.id==106?28:shot.id==107?23:shot.id==104?25:22;
    SandboxArt::Sprite(g,ProjectileArt(shot.id),p->mPosX-p->mX+12,p->mPosY+p->mPosZ-p->mY+12,size,size,angle,shot.powered?255:225);return true;
}
Sexy::MemoryImage* Card(int id){
    auto& cached=cards[id];if(cached)return cached.get();const auto* d=Find(id);if(!d)return nullptr;cached=gLawnApp->mReanimatorCache->MakeBlankMemoryImage(180,180);Sexy::Graphics g(cached.get());g.SetLinearBlend(true);
    Reanimation a;a.ReanimationInitializeType(40,40,GetPlantDefinition(static_cast<SeedType>(d->base)).mReanimationType);for(const char* layer:{"anim_idle","anim_face","anim_head_idle","anim_head_idle1","anim_head_idle2","anim_head_idle3"})if(a.TrackExists(layer)){a.SetFramesForLayer(layer);Skin(&a,id,false,4000);a.Draw(&g);}return cached.get();
}
void DrawCard(Sexy::Graphics* g,int x,int y,int id){
    const auto* d=Find(id);if(!d){DrawSeedPacket(g,x,y,static_cast<SeedType>(id),SEED_NONE,0,255,false,false);return;}PvzpDrawImageCelScaledF(g,Sexy::IMAGE_SEEDS,x,y,2,0,1,1);Sexy::Graphics clip(*g);clip.SetClipRect(x+3,y+9,44,48);SandboxArt::DrawFit(&clip,Card(id),x+4,y+10,42,44,1.0f);PvzpDrawString(g,"0",x+25,y+65,Sexy::FONT_BRIANNETOD12,Sexy::Color(55,64,23),DS_ALIGN_CENTER);
}
}
