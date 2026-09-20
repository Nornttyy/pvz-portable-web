#include "graphics/ImageFont.h"
#include "../src/SandboxFonts.h"
#include "../src/SandboxFontRules.h"
#include <cassert>
#include <iostream>
namespace Sexy {_Font* FONT_BRIANNETOD12=nullptr;_Font* FONT_DWARVENTODCRAFT18=nullptr;_Font* FONT_DWARVENTODCRAFT24=nullptr;}
using namespace Sexy;
void check(int cell,int advance,int offset,int split,bool measured){
    FontData data;data.mFontLayerList.emplace_back();
    auto& main=data.mFontLayerList.back();main.mImage=42;main.mAscent=12;main.mHeight=16;main.mPointSize=10;
    const CharData fire{{100,200,cell,cell},{offset,-4},advance};
    main.mCharDataMap[U'火']=fire;
    main.mCharDataMap[U'烤']={{300,200,cell,cell},{offset,-4},advance};
    main.mCharDataMap[U'陷']={{500,400,cell,cell},{offset,-4},advance};
    if(measured)main.mCharDataMap[U'焰']={}; // Font::CharWidth can insert an empty glyph.
    data.mFontLayerList.emplace_back();
    auto& extra=data.mFontLayerList.back();extra.mCharDataMap[U'焰']=fire;
    ImageFont font;font.mFontData=&data;FONT_BRIANNETOD12=&font;
    SandboxRepairFonts();
    assert(font.prepared==1);assert(data.mFontLayerList.size()==4);
    assert(!extra.mCharDataMap.contains(U'焰'));
    const auto& l=*data.mFontLayerMap.at("SANDBOXFLAMELEFT");
    const auto& r=*data.mFontLayerMap.at("SANDBOXFLAMERIGHT");
    assert(l.mImage==42&&r.mImage==42); // Same bitmap style; no system-font rasterization.
    assert(l.mAscent==main.mAscent&&r.mAscent==main.mAscent);
    const auto& a=l.mCharDataMap.at(U'焰');const auto& b=r.mCharDataMap.at(U'焰');
    assert(a.mWidth==advance&&b.mWidth==advance);
    assert(a.mImageRect.mWidth==split&&b.mImageRect.mWidth==cell-split);
    assert(a.mOffset.mX==offset&&b.mOffset.mX==offset+split);
    assert(a.mOffset.mY==fire.mOffset.mY&&b.mOffset.mY==fire.mOffset.mY);
    assert(main.mCharDataMap.at(U'火').mImageRect==fire.mImageRect);
    SandboxRepairFonts();assert(data.mFontLayerList.size()==4);assert(font.prepared==1);
    FONT_BRIANNETOD12=nullptr;
}
int main(){
    check(24,14,-5,11,false);check(27,16,-5,12,true);check(42,24,-9,19,true);
    FontData native;native.mFontLayerList.emplace_back();auto& layer=native.mFontLayerList.back();
    for(char32_t c:U"火烤陷焰")layer.mCharDataMap[c]={{0,0,24,24},{-5,-4},14};
    ImageFont font;font.mFontData=&native;FONT_DWARVENTODCRAFT24=&font;
    SandboxRepairFonts();assert(native.mFontLayerList.size()==1&&font.prepared==0);
    FONT_DWARVENTODCRAFT24=nullptr;SandboxRepairFonts();
    std::cout<<"Three font sizes, prior measurement, original glyph preservation and idempotence passed.\n";
}
