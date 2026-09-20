// Center the actual Chinese bitmap glyph rectangles, not the old Latin baseline.
#include "SandboxButton.h"
#include "Resources.h"
#include "graphics/Graphics.h"
#include "graphics/ImageFont.h"
#include <map>
#include <algorithm>
using namespace Sexy;
void SandboxDrawButton(Graphics* g,SandboxUIRules::Box b,const std::string& label,bool down,bool hover,bool large){
    auto* left=down?IMAGE_BUTTON_DOWN_LEFT:IMAGE_BUTTON_LEFT;
    auto* middle=down?IMAGE_BUTTON_DOWN_MIDDLE:IMAGE_BUTTON_MIDDLE;
    auto* right=down?IMAGE_BUTTON_DOWN_RIGHT:IMAGE_BUTTON_RIGHT;
    g->DrawImage(left,b.x,b.y,left->mWidth,b.h);
    g->DrawImage(middle,b.x+left->mWidth,b.y,b.w-left->mWidth-right->mWidth,b.h);
    g->DrawImage(right,b.x+b.w-right->mWidth,b.y,right->mWidth,b.h);
    auto* font=static_cast<ImageFont*>(large?FONT_DWARVENTODCRAFT24:hover?FONT_DWARVENTODCRAFT18BRIGHTGREENINSET:FONT_DWARVENTODCRAFT18GREENINSET);
    static std::map<std::pair<ImageFont*,std::string>,Rect> metrics;
    const auto key=std::make_pair(font,label);
    if(!metrics.contains(key)){
        RectList areas;Graphics measure(*g);measure.SetClipRect(0,0,0,0);
        font->DrawStringEx(&measure,0,0,label,Color(0,0,0,0),&areas,nullptr);
        int x1=0,y1=0,x2=0,y2=0;bool first=true;
        for(const auto& r:areas){if(!r.mWidth||!r.mHeight)continue;
            if(first){x1=r.mX;y1=r.mY;x2=r.mX+r.mWidth;y2=r.mY+r.mHeight;first=false;}
            else{x1=std::min(x1,r.mX);y1=std::min(y1,r.mY);x2=std::max(x2,r.mX+r.mWidth);y2=std::max(y2,r.mY+r.mHeight);}}
        metrics[key]=Rect(x1,y1,x2-x1,y2-y1);
    }
    const auto r=metrics[key];
    const auto pos=SandboxUIRules::CenterInk(b,{r.mX,r.mY,r.mWidth,r.mHeight},down);
    // In particular, don't run PvzpDrawString's second alignment pass here.
    const auto color=large?(hover?Color(180,235,80):Color(80,130,30)):Color::White;
    font->DrawStringEx(g,pos.x,pos.y,label,color,nullptr,nullptr);
}
