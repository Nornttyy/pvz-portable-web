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
#include <array>
#include <format>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

using namespace Sexy;
using namespace SandboxUIRules;
namespace {
enum Tool { PlantTool, ZombieTool, EraseTool, InteractTool };
int panel=0, plantPage=0, zombiePage=0, selectedPlant=0, selectedZombie=0, plantSlot=0, zombieSlot=0;
Tool tool=PlantTool;
std::array<int,6> plants{0,1,2,3,5,7}, zombies{0,2,4,7,23,24};
bool wasPaused=true, painting=false, dirty=false, showZombies=false;
int lastCell=-1, lastPlantCount=0, messageTicks=0;
std::string message;
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
void ClosePanel() { panel=0;painting=false;Command(4,wasPaused?1:0); }
void OpenPanel(int next) {
    if (!panel) wasPaused=bool(Command(0)&2);
    panel=next;painting=false;Command(4,1);
}
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
void Place(int cell, bool erase=false) {
    if(cell<0)return;
    const int result=Command(erase||tool==EraseTool?3:tool==PlantTool?1:2,tool==PlantTool?selectedPlant:selectedZombie,cell%9,cell/9);
    if(result>0)dirty=true;
    else if(result==-3)Say("数量已满，请先清理场地");
    else if(result==-5)Say("这只僵尸不能放在此处");
    else Say("不能放在这里，请检查位置和底座");
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
    panel=0;plantPage=zombiePage=0;tool=PlantTool;showZombies=false;painting=false;dirty=false;wasPaused=true;
    selectedPlant=0;selectedZombie=0;plantSlot=zombieSlot=0;lastCell=-1;lastPlantCount=0;messageTicks=0;
    plants={0,1,2,3,5,7};zombies={0,2,4,7,23,24};
    PvzpLoadResources("DelayLoad_Almanac");
    SandboxRepairFonts();
    for(int i=0;i<48;++i)Plant::PreloadPlantResources(static_cast<SeedType>(i));
    for(int type:Zombies)Zombie::PreloadZombieResources(static_cast<ZombieType>(type));
}
void SandboxUITick(Board* board) {
    if(messageTicks>0)--messageTicks;
    const int count=Command(9);
    if(count!=lastPlantCount){dirty=true;lastPlantCount=count;}
    board->MarkDirty();
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
    g->DrawImage(IMAGE_SEEDBANK,0,0);
    PvzpDrawString(g,"9999",34,78,FONT_CONTINUUMBOLD14,Color::Black,DS_ALIGN_CENTER);
    for(int i=0;i<6;++i){
        auto b=Hotbar(i);
        if(showZombies){Portrait(g,{b.x,b.y,b.w,64},zombies[i]);if(tool==ZombieTool&&i==zombieSlot)Outline(g,b);}
        else {SandboxPlants::DrawCard(g,b.x,b.y,plants[i]);if(tool==PlantTool&&i==plantSlot)Outline(g,b);}
    }
    Button(g,Control(0),"植物",panel==1);
    Button(g,Control(1),"僵尸",panel==2);
    Button(g,Control(2),"菜单",panel==3);
    Button(g,Control(3),flags&2?"开始":"暂停");
    Button(g,Control(4),std::format("{}x",static_cast<int>(gLawnApp->mUpdateMultiplier)));
    Button(g,Control(5),"铲除",tool==EraseTool);
    if(!panel){
        auto* wm=gLawnApp->mWidgetManager.get();
        int cell=Cell(wm->mLastMouseX,wm->mLastMouseY,bool(flags&4));
        if(cell>=0&&tool!=InteractTool){
            const int h=flags&4?85:100;
            g->SetColor(tool==EraseTool?Color(220,65,45,70):Color(245,244,103,65));
            g->FillRect(40+cell%9*80,80+cell/9*h,80,h);
        }
        const auto text=messageTicks>0?message:SelectedName();
        PvzpDrawString(g,text,400,598,FONT_BRIANNETOD12,Color(35,30,17),DS_ALIGN_CENTER);
        return;
    }
    g->SetColor(Color(0,0,0,125));g->FillRect(0,80,800,520);
    g->DrawImage(IMAGE_SEEDCHOOSER_BACKGROUND,Panel.x,Panel.y);
    std::string title=panel==1?"选择植物":panel==2?"选择僵尸":"沙盒模式";
    if(panel==1) {
        title=plantPage?"原创植物":"选择植物";
        std::string note=plantPage?"选择卡片后，点击草地种植":"";
        for(int i=0;i<(plantPage?int(SandboxPlants::Definitions.size()):48);++i){
            const int id=plantPage?100+i:i;auto b=plantPage?CustomPlantCard(i):PlantCard(i);
            SandboxPlants::DrawCard(g,b.x,b.y,id);
            if(Hover(b)){
                Outline(g,b);
                const auto* custom=SandboxPlants::Find(id);
                title=custom?custom->name:Plant::GetNameString(static_cast<SeedType>(id));
                if(custom)note=custom->note;
            }
        }
        if(plantPage){
            PvzpDrawString(g,note,400,538,FONT_BRIANNETOD12,Color(224,187,98),DS_ALIGN_CENTER);
            for(int i=0;i<int(SandboxPlants::Definitions.size());++i){
                const auto b=CustomPlantCard(i);
                PvzpDrawString(g,std::to_string(i+1),b.x+25,b.y+80,FONT_BRIANNETOD12,Color(224,187,98),DS_ALIGN_CENTER);
                const auto& d=SandboxPlants::Definitions[i];
                PvzpDrawString(g,std::format("{}. {}",i+1,d.name),190+(i%3)*144,390+(i/3)*24,FONT_BRIANNETOD12,Color(224,187,98),DS_ALIGN_LEFT);
            }
        }
        Button(g,OriginalPage,"原版植物",plantPage==0);
        Button(g,CustomPage,"原创植物",plantPage==1);
    } else if(panel==2) {
        std::string note="选择卡片后，点击草地放置";
        title=zombiePage?"原创僵尸":"选择僵尸";
        const int count=zombiePage?int(SandboxZombies::Definitions.size()):int(Zombies.size());
        for(int i=0;i<count;++i){
            auto b=ZombieCard(i);const int id=zombiePage?200+i:Zombies[i];Portrait(g,b,id);
            if(Hover(b)){Outline(g,b);if(auto* d=SandboxZombies::Find(id)){title=d->name;note=d->note;}else title=PvzpStringTranslate(std::string("[")+GetZombieDefinition(static_cast<ZombieType>(id)).mZombieName+"]");}
            if(zombiePage)PvzpDrawString(g,std::to_string(i+1),b.x+38,b.y+82,FONT_BRIANNETOD12,Color(224,187,98),DS_ALIGN_CENTER);
        }
        if(zombiePage){
            for(int i=0;i<count;++i)PvzpDrawString(g,std::format("{}. {}",i+1,SandboxZombies::Definitions[i].name),205+(i%2)*210,332+(i/2)*38,FONT_BRIANNETOD12,Color(224,187,98),DS_ALIGN_LEFT);
            PvzpDrawString(g,note,400,530,FONT_BRIANNETOD12,Color(224,187,98),DS_ALIGN_CENTER);
        }
        Button(g,OriginalPage,"原版僵尸",zombiePage==0);
        Button(g,CustomPage,"原创僵尸",zombiePage==1);
    } else {
        const std::array<std::string,14> labels{"白天草地","白天泳池","清除僵尸","清空场地","保存阵型","读取阵型","导出阵型","导入阵型",flags&8?"蘑菇免唤醒":"蘑菇正常睡眠","单步","每路一只","操作场地","全屏","返回主菜单"};
        for(int i=0;i<14;++i)Button(g,MenuAction(i),labels[i],i==(flags&4?1:0));
        const auto counts=std::format("植物 {}   僵尸 {}   越界 {}",Command(9),Command(10),Command(14));
        PvzpDrawString(g,counts,400,504,FONT_BRIANNETOD12,Color(224,187,98),DS_ALIGN_CENTER);
        if(messageTicks>0)PvzpDrawString(g,message,400,531,FONT_BRIANNETOD12,Color(224,187,98),DS_ALIGN_CENTER);
    }
    PvzpDrawString(g,title,400,116,FONT_DWARVENTODCRAFT18,Color(224,187,98),DS_ALIGN_CENTER);
    Button(g,Close,panel==3?"继续游戏":"返回");
}

bool SandboxMouseDown(int x,int y,int clicks) {
    if(!gSandboxEnabled)return false;
    if(gLawnApp->GetDialogCount()>0)return true;
    const int flags=Command(0);
    if(flags<0)return true;
    if(clicks<0){if(panel)ClosePanel();else if(tool==InteractTool)return false;else Place(Cell(x,y,bool(flags&4)),true);return true;}
    for(int i=0;i<6;++i)if(Control(i).Contains(x,y)){
        painting=false;
        gLawnApp->PlaySample(SOUND_GRAVEBUTTON);
        if(i<3){
            if(panel==i+1)ClosePanel();
            else {if(i<2)showZombies=i==1;OpenPanel(i+1);}
        }
        else if(i==3){if(panel){wasPaused=false;ClosePanel();}else Command(4,flags&2?0:1);}
        else if(i==4){int speed=static_cast<int>(gLawnApp->mUpdateMultiplier);Command(5,speed==4?1:speed*2);}
        else {if(panel)ClosePanel();tool=tool==EraseTool?InteractTool:EraseTool;}
        return true;
    }
    for(int i=0;i<6;++i)if(Hotbar(i).Contains(x,y)){
        if(showZombies){tool=ZombieTool;zombieSlot=i;selectedZombie=zombies[i];}
        else{tool=PlantTool;plantSlot=i;selectedPlant=plants[i];}
        gLawnApp->PlaySample(SOUND_SEEDLIFT);return true;
    }
    if(panel){
        if(Close.Contains(x,y)){ClosePanel();return true;}
        if(panel==1&&(OriginalPage.Contains(x,y)||CustomPage.Contains(x,y))){plantPage=CustomPage.Contains(x,y)?1:0;gLawnApp->PlaySample(SOUND_GRAVEBUTTON);return true;}
        if(panel==2&&(OriginalPage.Contains(x,y)||CustomPage.Contains(x,y))){zombiePage=CustomPage.Contains(x,y)?1:0;gLawnApp->PlaySample(SOUND_GRAVEBUTTON);return true;}
        if(panel==1)for(int i=0;i<(plantPage?int(SandboxPlants::Definitions.size()):48);++i)if((plantPage?CustomPlantCard(i):PlantCard(i)).Contains(x,y)){
            selectedPlant=plantPage?100+i:i;plants[plantSlot]=selectedPlant;tool=PlantTool;showZombies=false;ClosePanel();gLawnApp->PlaySample(SOUND_SEEDLIFT);return true;
        }
        if(panel==2)for(int i=0;i<(zombiePage?int(SandboxZombies::Definitions.size()):int(Zombies.size()));++i)if(ZombieCard(i).Contains(x,y)){
            selectedZombie=zombiePage?200+i:Zombies[i];zombies[zombieSlot]=selectedZombie;tool=ZombieTool;showZombies=true;ClosePanel();gLawnApp->PlaySample(SOUND_SEEDLIFT);return true;
        }
        if(panel==3)for(int i=0;i<14;++i)if(MenuAction(i).Contains(x,y)){gLawnApp->PlaySample(SOUND_GRAVEBUTTON);Action(i);return true;}
        // The panel owns all pointer input, including the dimmed lawn.
        return true;
    }
    if(y<80)return true;
    if(tool==InteractTool)return false;
    lastCell=Cell(x,y,bool(flags&4));Place(lastCell);painting=tool==PlantTool||tool==EraseTool;
    return true;
}
bool SandboxMouseDrag(int x,int y) {
    if(!gSandboxEnabled)return false;
    if(panel)return true;
    if(tool==InteractTool)return false;
    const int cell=Cell(x,y,bool(Command(0)&4));
    if(painting&&cell>=0&&cell!=lastCell){lastCell=cell;Place(cell);}
    return true;
}
bool SandboxMouseUp() {
    if(!gSandboxEnabled)return false;
    painting=false;lastCell=-1;
    return panel||tool!=InteractTool;
}
void SandboxKeyDown(int key) {
    if(key==KEYCODE_ESCAPE){if(panel)ClosePanel();else OpenPanel(3);}
    else if(key==KEYCODE_SPACE&&!panel)Command(4,Command(0)&2?0:1);
    else if(key>='1'&&key<='6'&&!panel){int i=key-'1';if(showZombies){tool=ZombieTool;zombieSlot=i;selectedZombie=zombies[i];}else{tool=PlantTool;plantSlot=i;selectedPlant=plants[i];}}
}
