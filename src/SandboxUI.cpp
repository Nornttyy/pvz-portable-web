// Native sandbox interface. Reuses the game's cards, wood panel, stone buttons and fonts.
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Sandbox.h"
#include "SandboxRules.h"
#include "SandboxUIRules.h"
#include "SandboxPlants.h"
#include "SandboxZombies.h"
#include "SandboxButton.h"
#include "SandboxFonts.h"
#include "LawnApp.h"
#include "Resources.h"
#include "Lawn/Board.h"
#include "Lawn/Plant.h"
#include "Lawn/Zombie.h"
#include "Lawn/SeedPacket.h"
#include "Lawn/System/ReanimationLawn.h"
#include "Lawn/Widget/GameButton.h"
#include "PvzpLib/PvzpCommon.h"
#include "PvzpLib/PvzpStringFile.h"
#include "graphics/Graphics.h"
#include "widget/WidgetManager.h"
#include "widget/Dialog.h"
#include "widget/Widget.h"
#include <array>
#include <format>
#include <memory>
#include <chrono>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

using namespace Sexy;
using namespace SandboxUIRules;
namespace {
enum Tool { PlantTool, ZombieTool, EraseTool, InteractTool };
int panel=0, plantPage=0, selectedPlant=0, selectedZombie=0, plantSlot=0, catalog=2, catalogPage=0;
Tool tool=PlantTool;
std::array<int,6> plants{0,1,2,3,5,7};
bool wasPaused=true, painting=false, dirty=false;
int lastCell=-1, lastPlantCount=0, messageTicks=0;
std::string message;
RepeatPlacement zombieRepeat;
long long PlacementTime() { return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count(); }
void StopPainting() { painting=false;lastCell=-1;zombieRepeat.Stop(); }
int Command(int cmd, int type=0, int col=0, int row=0) { return pvz_sandbox_command(cmd,type,col,row); }
bool Hover(Box box) { auto* wm=gLawnApp->mWidgetManager.get(); return box.Contains(wm->mLastMouseX,wm->mLastMouseY); }
void Say(const char* text) { message=text;messageTicks=240; }
void Button(Graphics* g, Box b, const std::string& label, bool selected=false) {
    SandboxDrawButton(g,b,label,Hover(b)&&(gLawnApp->mWidgetManager->mDownButtons&1),Hover(b)||selected);
}
void Outline(Graphics* g, Box b) {
    g->SetColor(Color(235,220,78));g->DrawRect(b.x-2,b.y-2,b.w+3,b.h+3);
    g->SetColor(Color(124,181,45));g->DrawRect(b.x-1,b.y-1,b.w+1,b.h+1);
}
void ClosePanel() { panel=0;StopPainting();Command(4,wasPaused?1:0); }
void OpenPanel(int next) {
    if (!panel) wasPaused=bool(Command(0)&2);
    panel=next;StopPainting();Command(4,1);
}
// Native overlay, in the same canvas. Lawn input passes through to Board,
// whose own origin supplies the inverse transform for every native interaction.
class SandboxOverlay final : public Widget {
public:
    SandboxOverlay(){mHasTransparencies=true;mWantsFocus=false;Resize(0,0,CanvasWidth,CanvasHeight);}
    void Draw(Graphics* g) override { SandboxDrawUI(g); }
    bool IsPointVisible(int x,int y) override {return panel || y<80 || x<SidebarWidth;}
    void MouseDown(int x,int y,int clicks) override {SandboxMouseDown(x,y,clicks);}
    void MouseDrag(int x,int y) override {SandboxMouseDrag(x,y);}
    void MouseUp(int,int,int) override {SandboxMouseUp();}
    void KeyDown(KeyCode key) override {SandboxKeyDown(key);}
};
std::unique_ptr<SandboxOverlay> overlay;
bool Confirm(const char* text) {
    return gLawnApp->LawnMessageBox(DIALOG_MESSAGE,"沙盒模式",text,"[DIALOG_BUTTON_YES]","[DIALOG_BUTTON_NO]",Dialog::BUTTONS_YES_NO)==Dialog::ID_YES;
}
void StorageAction(int action) {
#ifdef __EMSCRIPTEN__
    EM_ASM({ window.dispatchEvent(new CustomEvent('pvz-sandbox-action', {detail:$0})); },action);
#else
    Say("此功能需要浏览器");
#endif
}
void Portrait(Graphics* g, Box b, int type) {
    const float scale=b.w/76.0f;
    g->DrawImage(IMAGE_ALMANAC_ZOMBIEWINDOW,b.x,b.y,b.w,b.h);
    if(SandboxZombies::Find(type)){
        Graphics clipped(*g);clipped.SetClipRect(b.x+2,b.y+2,b.w-4,b.h-4);
        SandboxZombies::DrawPortrait(&clipped,b.x,b.y,b.w,b.h,type);
        g->DrawImage(IMAGE_ALMANAC_ZOMBIEWINDOW2,b.x,b.y,b.w,b.h);return;
    }
    Graphics z(*g);
    z.SetClipRect(b.x+2,b.y+2,b.w-4,b.h-4);
    z.TranslateF(b.x+scale,b.y-6*scale);
    z.mScaleX=0.5f*scale;z.mScaleY=0.5f*scale;
    auto drawType=static_cast<ZombieType>(type);
    float dx=0,dy=0;
    switch(drawType) {
    case ZOMBIE_POLEVAULTER: dx=2;dy=-3;drawType=ZOMBIE_CACHED_POLEVAULTER_WITH_POLE;break;
    case ZOMBIE_FLAG: dx=2;dy=10;break;
    case ZOMBIE_TRAFFIC_CONE:dy=12;break;
    case ZOMBIE_PAIL:dy=9;break;
    case ZOMBIE_FOOTBALL:dx=-15;dy=-1;break;
    case ZOMBIE_DOLPHIN_RIDER:dx=-2;dy=-10;break;
    case ZOMBIE_GARGANTUAR:case ZOMBIE_REDEYE_GARGANTUAR:dx=15;dy=17;break;
    case ZOMBIE_IMP:dx=-8;dy=-7;break;
    case ZOMBIE_DANCER:dy=15;break;
    case ZOMBIE_SNORKEL:dx=-10;break;
    case ZOMBIE_CATAPULT:dx=-24;dy=-1;break;
    default:break;
    }
    z.TranslateF(dx*scale,dy*scale);
    gLawnApp->mReanimatorCache->DrawCachedZombie(&z,0,0,drawType);
    g->DrawImage(IMAGE_ALMANAC_ZOMBIEWINDOW2,b.x,b.y,b.w,b.h);
}
std::string SelectedName() {
    if(tool==EraseTool)return "铲除";
    if(tool==InteractTool)return "操作场地";
    if(tool==PlantTool){const auto* custom=SandboxPlants::Find(selectedPlant);return custom?custom->name:Plant::GetNameString(static_cast<SeedType>(selectedPlant));}
    if(auto* custom=SandboxZombies::Find(selectedZombie))return custom->name;
    return std::string(PvzpStringTranslate(std::string("[")+GetZombieDefinition(static_cast<ZombieType>(selectedZombie)).mZombieName+"]"));
}
bool Place(int cell, bool erase=false) {
    if(cell<0)return false;
    const int result=Command(erase||tool==EraseTool?3:tool==PlantTool?1:2,tool==PlantTool?selectedPlant:selectedZombie,cell%9,cell/9);
    if(result>0)dirty=true;
    else if(result==-3)Say("数量已满，请先清理场地");
    else if(result==-5)Say("这只僵尸不能放在此处");
    else Say("不能放在这里，请检查位置和底座");
    return result>0;
}
void RepeatZombieAt(int x,int y) {
    const auto* wm=gLawnApp->mWidgetManager.get();
    const int flags=Command(0);
    const bool available=flags>=0&&(flags&32)&&tool==ZombieTool&&!panel&&gLawnApp->GetDialogCount()==0&&gLawnApp->mHasFocus&&!gLawnApp->mMinimized;
    const int cell=x<SidebarWidth?-1:Cell(x-WorldOffset,y,bool(flags&4));
    if(zombieRepeat.Poll(cell,PlacementTime(),bool(wm->mDownButtons&1),available)&&!Place(cell))StopPainting();
}
void Action(int index) {
    switch(index) {
    case 0:case 1:
        if((Command(9)||Command(10))&&!Confirm("切换场景会清空场地，继续吗？"))return;
        Command(8,index);return;
    case 2:Command(6);Say("僵尸已清除");return;
    case 3:if(Confirm("清空场地？已保存的阵型不会删除。"))Command(7);return;
    case 4:StorageAction(1);return;
    case 5:if(!dirty||Confirm("读取阵型会替换当前场地，继续吗？"))StorageAction(2);return;
    case 6:StorageAction(3);return;
    case 7:if(!dirty||Confirm("导入阵型会替换当前场地，继续吗？"))StorageAction(4);return;
    case 8:Command(12,(Command(0)&8)?0:1);return;
    case 9:panel=0;Command(13);return;
    case 10:Command(11,selectedZombie);return;
    case 11:tool=InteractTool;ClosePanel();return;
    case 12:StorageAction(5);return;
    case 13:
        if(!dirty||Confirm("返回主菜单？未保存的阵型不会保留。"))SandboxExit();
        return;
    }
}
}

void SandboxUIReset() {
    SandboxUIDetach();
    panel=0;plantPage=0;catalog=2;catalogPage=0;tool=PlantTool;painting=false;dirty=false;wasPaused=true;
    selectedPlant=0;selectedZombie=0;plantSlot=0;lastCell=-1;lastPlantCount=0;messageTicks=0;
    plants={0,1,2,3,5,7};
    StopPainting();
    PvzpLoadResources("DelayLoad_Almanac");
    SandboxRepairFonts();
    for(int i=0;i<48;++i)Plant::PreloadPlantResources(static_cast<SeedType>(i));
    for(int type:Zombies)Zombie::PreloadZombieResources(static_cast<ZombieType>(type));
    overlay=std::make_unique<SandboxOverlay>();
    gLawnApp->mWidgetManager->AddWidget(overlay.get());
}
void SandboxUIDetach() {
    StopPainting();
    if(overlay){gLawnApp->mWidgetManager->RemoveWidget(overlay.get());overlay.reset();}
}
void SandboxUITick(Board* board) {
    const auto* wm=gLawnApp->mWidgetManager.get();
    RepeatZombieAt(wm->mLastMouseX,wm->mLastMouseY);
    if(messageTicks>0)--messageTicks;
    const int count=Command(9);
    if(count!=lastPlantCount){dirty=true;lastPlantCount=count;}
    board->MarkDirty();
    if(overlay)overlay->MarkDirty();
}
bool SandboxUIHasUnsaved() { return dirty; }
void SandboxUIFeedback(int code) {
    if(code==1||code==2||code==3){dirty=false;lastPlantCount=Command(9);}
    switch(code){
    case 1:Say("阵型已保存");break;
    case 2:Say("阵型已读取");break;
    case 3:Say("阵型已导出");break;
    case 5:Say("还没有保存的阵型");break;
    case 6:dirty=true;Say("部分位置不能放置，请检查阵型");break;
    default:Say("操作失败，请检查阵型文件或浏览器存储");break;
    }
}
void SandboxDrawUI(Graphics* g) {
    const int flags=Command(0);
    if(flags<0)return;
    // Same native wood/card/font assets as adventure; no HTML panel or separate page.
    g->DrawImage(IMAGE_SEEDCHOOSER_BACKGROUND,0,80,SidebarWidth,520);
    g->DrawImage(IMAGE_SEEDBANK,0,0);
    PvzpDrawString(g,"9999",34,78,FONT_CONTINUUMBOLD14,Color::Black,DS_ALIGN_CENTER);
    for(int i=0;i<6;++i){
        const auto b=Hotbar(i);SandboxPlants::DrawCard(g,b.x,b.y,plants[i]);
        if(tool==PlantTool&&i==plantSlot)Outline(g,b);
    }
    g->DrawImage(IMAGE_SHOVELBANK,Shovel.x,Shovel.y,Shovel.w,Shovel.h);
    g->DrawImage(IMAGE_SHOVEL,Shovel.x+10,Shovel.y+7,66,60);
    if(tool==EraseTool)Outline(g,Shovel);
    Button(g,Control(0),"植物",catalog==1);
    Button(g,Control(1),"僵尸",catalog==2);
    Button(g,Control(2),"菜单",panel==3);
    Button(g,Control(3),flags&2?"开始":"暂停");
    Button(g,Control(4),std::format("{}x",static_cast<int>(gLawnApp->mUpdateMultiplier)));
    Button(g,Control(5),"每路一只");
    Button(g,Control(6),flags&32?"连放：开":"连放：关",flags&32);
    Button(g,Control(7),flags&16?"同格：开":"同格：关",flags&16);

    std::string title=catalog==2?"所有僵尸":plantPage?"原创植物":"所有植物";
    std::string hoverName;
    if(catalog==2){
        const int count=int(Zombies.size()+SandboxZombies::Definitions.size());
        for(int i=0;i<count;++i){
            const int id=i<int(Zombies.size())?Zombies[i]:200+i-int(Zombies.size());
            const auto b=SidebarZombie(i);Portrait(g,b,id);
            if(tool==ZombieTool&&selectedZombie==id)Outline(g,b);
            if(Hover(b)){
                Outline(g,b);
                if(const auto* d=SandboxZombies::Find(id))hoverName=d->name;
                else hoverName=PvzpStringTranslate(std::string("[")+GetZombieDefinition(static_cast<ZombieType>(id)).mZombieName+"]");
            }
        }
    }else{
        const int count=plantPage?int(SandboxPlants::Definitions.size()):48;
        for(int i=0;i<25&&i+catalogPage*25<count;++i){
            const int id=plantPage?SandboxPlants::Definitions[i].id:i+catalogPage*25;
            const auto b=SidebarPlant(i);SandboxPlants::DrawCard(g,b.x,b.y,id);
            if(tool==PlantTool&&selectedPlant==id)Outline(g,b);
            if(Hover(b)){Outline(g,b);const auto* d=SandboxPlants::Find(id);hoverName=d?d->name:Plant::GetNameString(static_cast<SeedType>(id));}
        }
        Button(g,NativeFilter,"原版",!plantPage);Button(g,CustomFilter,"原创",plantPage);
        if(!plantPage){
            Button(g,PrevPage,"<");Button(g,NextPage,">");
            PvzpDrawString(g,std::format("{}/2",catalogPage+1),132,578,FONT_BRIANNETOD12,Color(224,187,98),DS_ALIGN_CENTER);
        }
    }
    PvzpDrawString(g,title,132,107,FONT_DWARVENTODCRAFT18,Color(92,230,40),DS_ALIGN_CENTER);
    if(!hoverName.empty())PvzpDrawString(g,hoverName,132,catalog==2?578:512,FONT_BRIANNETOD12,Color(244,215,125),DS_ALIGN_CENTER);
    if(!panel){
        const auto* wm=gLawnApp->mWidgetManager.get();
        const int cell=Cell(wm->mLastMouseX-WorldOffset,wm->mLastMouseY,bool(flags&4));
        if(cell>=0&&tool!=InteractTool){
            const int h=flags&4?85:100;
            g->SetColor(tool==EraseTool?Color(220,65,45,70):Color(245,244,103,55));
            g->FillRect(WorldOffset+40+cell%9*80,80+cell/9*h,80,h);
        }
        if(messageTicks>0)PvzpDrawString(g,message,624,598,FONT_BRIANNETOD12,Color(35,30,17),DS_ALIGN_CENTER);
        return;
    }
    // Only the settings menu is modal. Browsing plants/zombies never pauses or hides the lawn.
    g->SetColor(Color(0,0,0,125));g->FillRect(SidebarWidth,80,CanvasWidth-SidebarWidth,520);
    g->DrawImage(IMAGE_SEEDCHOOSER_BACKGROUND,Panel.x,Panel.y);
    const std::array<std::string,14> labels{"白天草地","白天泳池","清除僵尸","清空场地","保存阵型","读取阵型","导出阵型","导入阵型",flags&8?"蘑菇免唤醒":"蘑菇正常睡眠","单步","每路一只","操作场地","全屏","返回主菜单"};
    for(int i=0;i<14;++i)Button(g,MenuAction(i),labels[i],i==(flags&4?1:0));
    PvzpDrawString(g,std::format("植物 {}   僵尸 {}   越界 {}",Command(9),Command(10),Command(14)),624,504,FONT_BRIANNETOD12,Color(224,187,98),DS_ALIGN_CENTER);
    if(messageTicks>0)PvzpDrawString(g,message,624,531,FONT_BRIANNETOD12,Color(224,187,98),DS_ALIGN_CENTER);
    PvzpDrawString(g,"沙盒模式",624,116,FONT_DWARVENTODCRAFT18,Color(224,187,98),DS_ALIGN_CENTER);
    Button(g,Close,"继续游戏");
}

bool SandboxMouseDown(int x,int y,int clicks) {
    if(!gSandboxEnabled)return false;
    if(gLawnApp->GetDialogCount()>0)return true;
    const int flags=Command(0);if(flags<0)return true;
    if(clicks<0){
        StopPainting();
        if(panel)ClosePanel();
        else if(tool==InteractTool&&x>=SidebarWidth&&y>=80)return false;
        else if(x>=SidebarWidth)Place(Cell(x-WorldOffset,y,bool(flags&4)),true);
        return true;
    }
    // Modal input must not leak into the catalog or lawn behind it.
    if(panel){
        if(Close.Contains(x,y)||Control(2).Contains(x,y)){ClosePanel();return true;}
        for(int i=0;i<14;++i)if(MenuAction(i).Contains(x,y)){gLawnApp->PlaySample(SOUND_GRAVEBUTTON);Action(i);return true;}
        return true;
    }
    for(int i=0;i<ControlCount;++i)if(Control(i).Contains(x,y)){
        StopPainting();gLawnApp->PlaySample(SOUND_GRAVEBUTTON);
        if(i<2){catalog=i+1;}
        else if(i==2)OpenPanel(3);
        else if(i==3)Command(4,flags&2?0:1);
        else if(i==4){int speed=static_cast<int>(gLawnApp->mUpdateMultiplier);Command(5,speed==4?1:speed*2);}
        else if(i==5)Command(11,selectedZombie);
        else if(i==6){Command(20,flags&32?0:1);if(!(flags&32))Say("按住连续放置，拖动换位置，松手停止");}
        else {Command(19,flags&16?0:1);dirty=true;if(!(flags&16))Say("同一格可种多株，铲子每次移除一株");}
        return true;
    }
    if(Shovel.Contains(x,y)){StopPainting();tool=tool==EraseTool?InteractTool:EraseTool;return true;}
    for(int i=0;i<6;++i)if(Hotbar(i).Contains(x,y)){
        StopPainting();
        tool=PlantTool;plantSlot=i;selectedPlant=plants[i];
        gLawnApp->PlaySample(SOUND_SEEDLIFT);return true;
    }
    if(x<SidebarWidth){
        StopPainting();
        if(catalog==2){
            for(int i=0;i<int(Zombies.size()+SandboxZombies::Definitions.size());++i)if(SidebarZombie(i).Contains(x,y)){
                selectedZombie=i<int(Zombies.size())?Zombies[i]:200+i-int(Zombies.size());
                tool=ZombieTool;gLawnApp->PlaySample(SOUND_SEEDLIFT);return true;
            }
        }else{
            if(NativeFilter.Contains(x,y)||CustomFilter.Contains(x,y)){plantPage=CustomFilter.Contains(x,y)?1:0;catalogPage=0;return true;}
            if(!plantPage&&(PrevPage.Contains(x,y)||NextPage.Contains(x,y))){catalogPage=1-catalogPage;return true;}
            const int count=plantPage?int(SandboxPlants::Definitions.size()):48;
            for(int i=0;i<25&&i+catalogPage*25<count;++i)if(SidebarPlant(i).Contains(x,y)){
                selectedPlant=plantPage?SandboxPlants::Definitions[i].id:i+catalogPage*25;plants[plantSlot]=selectedPlant;
                tool=PlantTool;gLawnApp->PlaySample(SOUND_SEEDLIFT);return true;
            }
        }
        return true;
    }
    if(y<80)return true;
    if(tool==InteractTool)return false;
    StopPainting();lastCell=Cell(x-WorldOffset,y,bool(flags&4));
    if(Place(lastCell)){
        painting=tool==PlantTool||tool==EraseTool;
        if(tool==ZombieTool&&(flags&32))zombieRepeat.Begin(lastCell,PlacementTime());
    }
    return true;
}
bool SandboxMouseDrag(int x,int y) {
    if(!gSandboxEnabled)return false;
    RepeatZombieAt(x,y);
    if(panel||x<SidebarWidth||y<80)return true;
    if(tool==InteractTool)return false;
    const int cell=Cell(x-WorldOffset,y,bool(Command(0)&4));
    if(painting&&cell>=0&&cell!=lastCell){lastCell=cell;Place(cell);}
    return true;
}
bool SandboxMouseUp() {
    if(!gSandboxEnabled)return false;
    StopPainting();
    return panel||tool!=InteractTool;
}
void SandboxKeyDown(int key) {
    StopPainting();
    if(key==KEYCODE_ESCAPE){if(panel)ClosePanel();else OpenPanel(3);}
    else if(key==KEYCODE_SPACE&&!panel)Command(4,Command(0)&2?0:1);
    else if(key>='1'&&key<='6'&&!panel){const int i=key-'1';tool=PlantTool;plantSlot=i;selectedPlant=plants[i];}
}
