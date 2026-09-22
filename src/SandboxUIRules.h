#pragma once
#include <array>
namespace SandboxUIRules {
struct Box {
    int x, y, w, h;
    constexpr bool Contains(int px, int py) const { return px >= x && px < x+w && py >= y && py < y+h; }
};
constexpr Box CenterInk(Box button,Box ink,bool down=false) {
    return {button.x+(button.w-ink.w)/2-ink.x+(down?1:0),button.y+(button.h-ink.h)/2-ink.y+(down?1:0),ink.w,ink.h};
}
inline constexpr Box MenuEntry{140, 320, 240, 56};
inline constexpr int CanvasWidth=1024, CanvasHeight=600, WorldOffset=224, SidebarWidth=264;
// Screen shake is relative to the layout origin, never an absolute board position.
constexpr int BoardX(bool sandbox,int shake=0) { return (sandbox ? WorldOffset:0)+shake; }
inline constexpr Box Panel{392, 82, 465, 513};
constexpr Box Hotbar(int i) { return {79+i*59, 8, 50, 70}; }
inline constexpr int ControlCount=8;
constexpr Box Control(int i) { return {548+(i%4)*115, 8+(i/4)*37, 110, 33}; }
// Wall-clock cadence: game speed and pause do not change the placement rate.
struct RepeatPlacement {
    bool active=false;
    int cell=-1;
    long long next=0;
    static constexpr int IntervalMs=300;
    void Stop() { active=false;cell=-1; }
    void Begin(int at,long long now) { active=at>=0;cell=at;next=now+IntervalMs; }
    bool Poll(int at,long long now,bool held,bool available) {
        if(!held||!available){Stop();return false;}
        if(!active)return false;
        if(at<0){cell=-1;next=now+IntervalMs;return false;}
        if(at==cell&&now<next)return false;
        cell=at;next=now+IntervalMs;return true;
    }
};
inline constexpr Box Shovel{447,4,86,74};
constexpr Box SidebarZombie(int i) { return {8+(i%5)*50,118+(i/5)*62,46,58}; }
constexpr Box SidebarPlant(int i) { return {5+(i%5)*51,118+(i/5)*78,50,70}; }
inline constexpr Box NativeFilter{7,520,121,30},CustomFilter{135,520,121,30};
inline constexpr Box PrevPage{7,557,70,30},NextPage{187,557,70,30};
constexpr Box PlantCard(int i) { return {185+(i%8)*54, 128+(i/8)*70, 50, 70}; }
constexpr Box CustomPlantCard(int i) { return {185+(i%8)*54, 128+(i/8)*82, 50, 70}; }
constexpr Box ZombieCard(int i) { return {190+(i%5)*84, 128+(i/5)*84, 76, 76}; }
constexpr Box MenuAction(int i) { return {419+(i%2)*212, 145+(i/2)*45, 190, 33}; }
inline constexpr Box Close{554, 554, 140, 33};
inline constexpr Box OriginalPage{185,554,132,33}, CustomPage{483,554,132,33};
inline constexpr std::array<int,23> Zombies{0,1,2,3,4,5,6,7,8,10,11,12,14,15,16,17,18,19,21,22,23,24,32};
constexpr int Cell(int x, int y, bool pool) {
    if (x < 40 || x >= 760 || y < 80 || y >= (pool ? 590 : 580)) return -1;
    return ((y-80)/(pool ? 85 : 100))*9+(x-40)/80;
}
}
