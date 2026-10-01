#include "graphics/ImageFont.h"
#include "graphics/MemoryImage.h"
#include "../src/SandboxFonts.h"
#include "../src/SandboxFontRules.h"
#include <cassert>
#include <iostream>
namespace Sexy {_Font* FONT_BRIANNETOD12=nullptr;_Font* FONT_BRIANNETOD16=nullptr;_Font* FONT_DWARVENTODCRAFT18=nullptr;_Font* FONT_DWARVENTODCRAFT24=nullptr;_Font* FONT_DWARVENTODCRAFT18YELLOW=nullptr;_Font* FONT_DWARVENTODCRAFT18GREENINSET=nullptr;_Font* FONT_DWARVENTODCRAFT18BRIGHTGREENINSET=nullptr;_Font* FONT_HOUSEOFTERROR16=nullptr;_Font* FONT_HOUSEOFTERROR28=nullptr;}
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
    {FontData dots;dots.mFontLayerList.emplace_back();auto& donor=dots.mFontLayerList.back();
     const CharData period{{100,200,24,24},{-10,-4},3};donor.mCharDataMap[U'.']=period;donor.mCharDataMap[U'·']={};
     ImageFont dotFont;dotFont.mFontData=&dots;FONT_BRIANNETOD12=&dotFont;SandboxRepairFonts();
     const auto& dot=dots.mFontLayerMap.at("MEMEGLYPHMIDDLEDOT0")->mCharDataMap.at(U'·');
     assert(dot.mImageRect==period.mImageRect&&dot.mWidth==period.mWidth&&dot.mOffset.mX==period.mOffset.mX&&dot.mOffset.mY==period.mOffset.mY-6);
     SandboxRepairFonts();assert(dots.mFontLayerList.size()==2&&dotFont.prepared==1);FONT_BRIANNETOD12=nullptr;
    }
    check(24,14,-5,11,false);check(27,16,-5,12,true);check(42,24,-9,19,true);
    FontData native;native.mFontLayerList.emplace_back();auto& layer=native.mFontLayerList.back();
    for(char32_t c:U"火烤陷焰种沙秒")layer.mCharDataMap[c]={{0,0,24,24},{-5,-4},14};
    ImageFont font;font.mFontData=&native;FONT_DWARVENTODCRAFT24=&font;
    SandboxRepairFonts();assert(native.mFontLayerList.size()==1&&font.prepared==0);
    FONT_DWARVENTODCRAFT24=nullptr;SandboxRepairFonts();
    FontData added;added.mFontLayerList.emplace_back();auto& original=added.mFontLayerList.back();
    for(char32_t c:U"样硬钢蜗用电")original.mCharDataMap[c]={{0,0,24,24},{-5,-4},14};
    ImageFont fixed;fixed.mFontData=&added;FONT_BRIANNETOD12=&fixed;SandboxRepairFonts();
    assert(added.mFontLayerList.size()==8&&fixed.prepared==3);
    for(char32_t c:U"梗锅甩"){if(!c)continue;int found=0;for(const auto& l:added.mFontLayerList)if(l.mCharDataMap.contains(c)){const auto& g=l.mCharDataMap.at(c);assert(g.mWidth==14&&g.mImageRect.mWidth>0&&g.mImageRect.mHeight>0);++found;}assert(found>=2);}
    SandboxRepairFonts();assert(added.mFontLayerList.size()==8&&fixed.prepared==3);FONT_BRIANNETOD12=nullptr;
    FontData batch;batch.mFontLayerList.emplace_back();auto& source=batch.mFontLayerList.back();
    for(char32_t c:U"蹦增馆板样场提模边油")source.mCharDataMap[c]={{0,0,24,24},{-5,-4},14};
    ImageFont newNames;newNames.mFontData=&batch;FONT_BRIANNETOD12=&newNames;SandboxRepairFonts();
    assert(batch.mFontLayerList.size()==12&&newNames.prepared==5);
    for(char32_t c:U"蹭饭杨摸迪"){if(!c)continue;int count=0;for(const auto& layer:batch.mFontLayerList)if(layer.mCharDataMap.contains(c)){const auto& g=layer.mCharDataMap.at(c);assert(g.mWidth==14&&g.mImageRect.mWidth>0&&g.mImageRect.mHeight>0);++count;}assert(count>=2);}
    SandboxRepairFonts();assert(batch.mFontLayerList.size()==12&&newNames.prepared==5);FONT_BRIANNETOD12=nullptr;
    FontData awkward;awkward.mFontLayerList.emplace_back();auto& glyphs=awkward.mFontLayerList.back();
    for(char32_t c:U"优价池杆")glyphs.mCharDataMap[c]={{0,0,24,24},{-5,-4},14};
    ImageFont awkwardFont;awkwardFont.mFontData=&awkward;FONT_BRIANNETOD12=&awkwardFont;SandboxRepairFonts();
    assert(awkward.mFontLayerList.size()==6&&awkwardFont.prepared==2);
    for(char32_t c:U"尬汗"){if(!c)continue;int n=0;for(const auto& l:awkward.mFontLayerList)if(l.mCharDataMap.contains(c)){const auto& g=l.mCharDataMap.at(c);assert(g.mWidth==14&&g.mImageRect.mWidth>0&&g.mImageRect.mHeight>0);++n;}assert(n>=2);}
    SandboxRepairFonts();assert(awkward.mFontLayerList.size()==6&&awkwardFont.prepared==2);FONT_BRIANNETOD12=nullptr;
    FontData tucking;tucking.mFontLayerList.emplace_back();tucking.mFontLayerList.back().mCharDataMap[U'鸣']={{0,0,24,24},{-5,-4},14};
    ImageFont tuckingFont;tuckingFont.mFontData=&tucking;FONT_BRIANNETOD12=&tuckingFont;SandboxRepairFonts();
    assert(tucking.mFontLayerList.size()==5&&tuckingFont.prepared==1);
    for(const auto& l:tucking.mFontLayerList)if(l.mCharDataMap.contains(U'乌')){const auto& g=l.mCharDataMap.at(U'乌');assert(g.mWidth==14&&g.mImageRect.mWidth>0&&g.mImageRect.mHeight>0);}
    SandboxRepairFonts();assert(tucking.mFontLayerList.size()==5&&tuckingFont.prepared==1);FONT_BRIANNETOD12=nullptr;
    for(const int cell:{24,27,42}){
        FontData wrap;wrap.mFontLayerList.emplace_back();auto& wrapDonor=wrap.mFontLayerList.back();wrapDonor.mImage=42;
        for(char32_t c:U"六果装")wrapDonor.mCharDataMap[c]={{0,0,cell,cell},{-5,-4},14};
        ImageFont wrapFont;wrapFont.mFontData=&wrap;FONT_BRIANNETOD12=&wrapFont;SandboxRepairFonts();
        assert(wrap.mFontLayerList.size()==4&&wrapFont.prepared==3);
        for(const auto& l:wrap.mFontLayerList)for(char32_t c:U"裹叠绿")if(c&&l.mCharDataMap.contains(c)){
         const auto& g=l.mCharDataMap.at(c);assert(g.mWidth==14&&g.mImageRect.mWidth==cell&&g.mImageRect.mHeight==cell);
         assert(g.mOffset.mX==-5&&g.mOffset.mY==-4);MemoryImage* bitmap=l.mImage;assert(bitmap);
         int ink=0;for(auto p:bitmap->bits)ink+=(p>>24)>0;assert(ink>30&&ink<14*14);
        }
        SandboxRepairFonts();assert(wrap.mFontLayerList.size()==4&&wrapFont.prepared==3);FONT_BRIANNETOD12=nullptr;
        FontData timing;timing.mFontLayerList.emplace_back();auto& donor=timing.mFontLayerList.back();donor.mImage=42;donor.mAscent=12;
        donor.mCharDataMap[U'种']={{100,200,cell,cell},{-5,-4},14};
        donor.mCharDataMap[U'沙']={{300,400,cell,cell},{-5,-4},14};
        donor.mCharDataMap[U'秒']={}; // A width query may have left an empty entry.
        ImageFont timingFont;timingFont.mFontData=&timing;FONT_BRIANNETOD12=&timingFont;SandboxRepairFonts();
        assert(timing.mFontLayerList.size()==3&&timingFont.prepared==1);
        const auto& left=*timing.mFontLayerMap.at("MEMEGLYPHMIAO0");const auto& right=*timing.mFontLayerMap.at("MEMEGLYPHMIAO1");
        const auto& a=left.mCharDataMap.at(U'秒');const auto& b=right.mCharDataMap.at(U'秒');const int split=int(cell*.46f+.5f);
        assert(left.mImage==42&&right.mImage==42&&left.mAscent==12&&right.mAscent==12);
        assert(a.mWidth==14&&b.mWidth==14&&a.mImageRect.mWidth==split&&b.mImageRect.mWidth==cell-split);
        assert(a.mOffset.mX==-5&&b.mOffset.mX==-5+split&&a.mOffset.mY==-4&&b.mOffset.mY==-4);
        SandboxRepairFonts();assert(timing.mFontLayerList.size()==3&&timingFont.prepared==1);FONT_BRIANNETOD12=nullptr;
    }
    // Coloured atlases keep their dominant ink (never the brightest bevel),
    // and complete native glyphs are left untouched on every repeated call.
    {MemoryImage atlas;atlas.Create(27,27);for(int y=5;y<21;++y)for(int x=5;x<21;++x)atlas.bits[y*27+x]=0xff00c400;
     atlas.bits[5*27+5]=0xffffffff;
     FontData data;data.mFontLayerList.emplace_back();auto& main=data.mFontLayerList.back();main.mImage=&atlas;
     main.mCharDataMap[U'装']={{0,0,27,27},{-5,-4},16};main.mCharDataMap[U'叠']={{0,0,27,27},{-5,-4},16};
     ImageFont font;font.mFontData=&data;FONT_DWARVENTODCRAFT18GREENINSET=&font;SandboxRepairFonts();
     assert(data.mFontLayerList.size()==3&&font.prepared==2);assert(!data.mFontLayerMap.contains("EXACTGLYPHSTACK"));
     for(const char* key:{"EXACTGLYPHWRAP","EXACTGLYPHGREEN"}){auto* l=data.mFontLayerMap.at(key);MemoryImage* image=l->mImage;assert(image);for(auto p:image->bits)if(p>>24)assert((p&0xffffff)==0x00c400);}
     SandboxRepairFonts();assert(font.prepared==2&&main.mCharDataMap.at(U'叠').mWidth==16);FONT_DWARVENTODCRAFT18GREENINSET=nullptr;
    }
    std::cout<<"Three font sizes, prior measurement, original glyph preservation and idempotence passed.\n";
}
