#pragma once
namespace EverythingShooterRules {
inline constexpr int Id=529,Base=49,Cost=250,Recharge=750,Interval=150,Unlock=27;
inline constexpr int First=320,NativeCount=14,Doom=334,Cherry=335,Poop=336;
inline constexpr int DoomPercent=5,CherryPercent=5,PoopPercent=10,PoopDamage=80;
constexpr bool Own(int style){return style>=First&&style<=Poop;}
constexpr bool Special(int style){return style>=Doom&&style<=Poop;}
constexpr int Choose(int rareRoll,int nativeRoll){return rareRoll<DoomPercent?Doom:rareRoll<DoomPercent+CherryPercent?Cherry:rareRoll<DoomPercent+CherryPercent+PoopPercent?Poop:First+nativeRoll%NativeCount;}
constexpr int NativeType(int style){return Special(style)?2:style-First;}
constexpr bool Lob(int style){const int t=NativeType(style);return t==2||t==3||t==5||t==9||t==10||t==11||t==12;}
// Validate the saved identity without rerolling it. Ordinary pea and snow pea
// may have passed through native torchwood; no other type may silently change.
constexpr bool Valid(int style,int type,int motion){
 if(!Own(style))return false;const int original=NativeType(style);
 if(original==0||original==1){if(type!=original&&type!=0&&type!=6)return false;}
 else if(type!=original)return false;
 return motion==(Lob(style)?1:original==4?5:0);
}
}
