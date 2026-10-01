#include "MemeCharacters.h"
#include "MemeShooterRules.h"
#include "SandboxZombies.h"
#include <cassert>
#include <iostream>
#include <string>
#include <string_view>

std::string number(double value){
 auto s=std::to_string(value);
 while(s.back()=='0')s.pop_back();
 if(s.back()=='.')s.pop_back();
 return s;
}
void includes(std::string_view text,const std::string& value){
 if(text.find(value)==text.npos){std::cerr<<"Missing stat: "<<value<<'\n';assert(false);}
}
std::string_view plant(int id){return MemeCharacters::Find(id)->description;}
int main(){
 using namespace MemeCharacters;
 using namespace MemeShooterRules;
 includes(plant(EverythingShooter),"每"+number(EverythingShooterRules::Interval/100.0)+"秒");
 includes(plant(EverythingShooter),number(EverythingShooterRules::Cost)+"阳光");
 includes(plant(EverythingShooter),"冷却"+number(PlantingCooldown(EverythingShooter)/100.0)+"秒");
 includes(plant(EverythingShooter),number(EverythingShooterRules::NativeCount)+"种原版子弹");
 includes(plant(EverythingShooter),number(EverythingShooterRules::DoomPercent)+"%毁灭菇弹");
 includes(plant(EverythingShooter),number(EverythingShooterRules::CherryPercent)+"%樱桃弹");
 includes(plant(EverythingShooter),number(100-EverythingShooterRules::DoomPercent-EverythingShooterRules::CherryPercent-EverythingShooterRules::PoopPercent)+"%随机");
 includes(plant(EverythingShooter),number(EverythingShooterRules::PoopPercent)+"%大粪：伤害"+number(EverythingShooterRules::PoopDamage));
 includes(plant(IceChili),"整行伤害"+number(IceChiliRules::Damage));
 includes(plant(IceChili),"种下"+number(IceChiliRules::Windup/100.0)+"秒");
 includes(plant(IceChili),"冻结"+number(IceChiliRules::Freeze/100.0)+"秒");
 includes(plant(IceChili),number(IceChiliRules::Cost)+"阳光");
 includes(plant(IceChili),"冷却"+number(PlantingCooldown(IceChili)/100.0)+"秒");
 includes(plant(StinkShroom),"伤害"+number(StinkShroomRules::Damage));
 includes(plant(StinkShroom),"每"+number(StinkShroomRules::Interval/100.0)+"秒喷射");
 includes(plant(StinkShroom),"眩晕"+number(StinkShroomRules::StunTicks/100.0)+"秒");
 includes(plant(StinkShroom),"普通"+number(StinkShroomRules::PushNormal/80.0)+"格 / 巨人"+number(StinkShroomRules::PushGiant/80.0)+"格");
 includes(plant(StinkShroom),"待"+number(StinkShroomRules::Exposure/100.0)+"秒");
 includes(plant(StinkShroom),"冷却"+number(PlantingCooldown(StinkShroom)/100.0)+"秒");
 includes(plant(NukeShroom),"全屏爆炸"+number(NukeShroomRules::Pulses)+"次");
 includes(plant(NukeShroom),"间隔"+number(NukeShroomRules::Interval/100.0)+"秒");
 includes(plant(NukeShroom),"留下3×3大坑");
 includes(plant(NukeShroom),number(NukeShroomRules::CraterLife/100.0)+"秒后恢复");
 includes(plant(NukeShroom),number(NukeShroomRules::Cost)+"阳光");
 includes(plant(NukeShroom),"冷却"+number(PlantingCooldown(NukeShroom)/100.0)+"秒");
 includes(plant(500),"单发伤害20");
 includes(plant(500),"普攻"+number(NormalDelay/100.0)+"秒/发");
 int hits=0;for(int roll=0;roll<10;++roll)hits+=CanHit(NormalStyle(roll));
 includes(plant(500),"命中判定"+number(hits*10)+"%");
 includes(plant(500),"每发怒气+"+number(PerShot));
 includes(plant(500),"满"+number(MaxRage)+"自动红温");
 includes(plant(500),"红温"+number(BurstTicks/100.0)+"秒"+number(BurstCount)+"发");
 includes(plant(500),"结束休息"+number(RecoveryDelay/100.0)+"秒");
 includes(plant(LongRepeater),"每轮"+number(RepeaterCount)+"发");
 includes(plant(LongRepeater),"间隔"+number(RepeaterInterval/100.0)+"秒");
 includes(plant(LongRepeater),"休息"+number(RepeaterRest/100.0)+"秒");
 includes(plant(GatlingShooter),"攻速"+number(GatlingInterval/100.0)+"秒/发");
 includes(plant(GatlingShooter),"连射"+number(GatlingHeatLimit*GatlingInterval/100.0)+"秒共"+number(GatlingHeatLimit)+"发");
 includes(plant(GatlingShooter),"散热"+number(GatlingCooldown/100.0)+"秒");
 includes(plant(SmallNut),"生命"+number(SmallNutHealth));
 includes(plant(SmallNut),"种植冷却"+number(PlantingCooldown(SmallNut)/100.0)+"秒");
 includes(plant(CactusPalm),number(PalmInterval/100.0)+"秒一掌");
 includes(plant(CactusPalm),"普通伤害"+number(PalmDamage(PalmProjectile)));
 includes(plant(CactusPalm),"暴击造成"+number(PalmDamage(CriticalPalmProjectile))+"伤害");
 includes(plant(CactusPalm),"普通"+number(PalmPush(PalmProjectile,false)/80.0)+"格 / 巨人"+number(PalmPush(PalmProjectile,true)/80.0)+"格");
 includes(plant(CactusPalm),number(PalmPush(CriticalPalmProjectile,false)/80.0)+"格 / "+number(PalmPush(CriticalPalmProjectile,true)/80.0)+"格");
 for(const auto& d:SandboxZombies::Definitions){
  const std::string_view stats(d.note);
  includes(stats,"生命"+number(d.health));includes(stats,"护甲"+number(d.armor));
  if(d.armor)includes(stats,"合计"+number(d.health+d.armor));
 }
 using namespace SandboxZombies;
 for(int id:{Clever,CleverCone}){
  const std::string_view s=SandboxZombies::Find(id)->note;
  includes(s,"普速"+number(CleverSpeed)+"倍 / 逃跑"+number(CleverFleeSpeed)+"倍");
  includes(s,"躲弹"+number(DodgePercent)+"%");includes(s,"翻身"+number(FlipTicks/100.0)+"秒");
  includes(s,"换行"+number(LaneChangePercent)+"%");
  includes(s,"前飞"+number(ForwardFlightDistance/80.0)+"格"+number(ForwardFlightPercent)+"%");
  includes(s,"间隔"+number(DodgeRecovery/100.0)+"秒");
 }
 const std::string_view runner=SandboxZombies::Find(Runner)->note;
 includes(runner,"冲刺"+number(RunInSpeed*100/80)+"格/秒");
 includes(runner,"逃跑"+number(RunOutSpeed*100/80)+"格/秒");
 includes(runner,"刹车"+number(BrakeTicks/100.0)+"秒");
 includes(SandboxZombies::Find(GiantImp)->note,"前摇"+number((JawTicks-JawImpact)/100.0)+"秒");
 includes(SandboxZombies::Find(GiantImp)->note,"动作"+number(JawTicks/100.0)+"秒");
 includes(SandboxZombies::Find(ConeWrap)->note,"外形"+number(ConeVisualCount)+"个桶");
 includes(SandboxZombies::Find(ConeWrap)->note,"相当于"+number(ConeCount)+"个桶");
 includes(SandboxZombies::Find(ConeWrap)->note,"普通"+number(int(ConeWrapSpeed*100+.5f))+"%");
 includes(SandboxZombies::Find(ConeTower)->note,number(TowerCones)+"个路障，每个"+number(ConeHealth)+"耐久");
 includes(SandboxZombies::Find(ConeTower)->note,"普通"+number(int(ConeTowerSpeed*100+.5f))+"%");
 std::cout<<"Almanac numeric statistics match native constants\n";
}
