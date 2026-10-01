// Repair missing Chinese glyphs without changing existing bitmap metrics.
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "SandboxFonts.h"
#include "SandboxFontRules.h"
#include "Resources.h"
#include "graphics/ImageFont.h"
#include "graphics/MemoryImage.h"
#include "ExactGlyphPixels.h"
#include <memory>
#include <vector>
using namespace Sexy;
namespace {
void ExactGlyph(_Font* source,char32_t target,const char* name,const char* mask){
 auto* font=dynamic_cast<ImageFont*>(source);if(!font||!font->mFontData)return;
 auto* data=font->mFontData;const std::string key=std::string("EXACTGLYPH")+name;
 if(data->mFontLayerMap.contains(key))return;
 for(auto& l:data->mFontLayerList){const auto it=l.mCharDataMap.find(target);if(it!=l.mCharDataMap.end()&&it->second.mImageRect.mWidth>0)return;}
 FontLayer* main=nullptr;
 for(auto& l:data->mFontLayerList){const auto it=l.mCharDataMap.find(U'装');if(it!=l.mCharDataMap.end()&&it->second.mImageRect.mWidth>0){main=&l;break;}}
 if(!main)return;
 auto glyph=main->mCharDataMap.at(U'装');const auto rect=glyph.mImageRect;
 const int width=rect.mWidth,height=rect.mHeight;
 int left=std::max(0,-glyph.mOffset.mX),top=std::max(0,-glyph.mOffset.mY);
 int right=std::min(width,left+glyph.mWidth)-1,bottom=std::min(height,top+glyph.mWidth)-1;
 unsigned colour=0xffffff;MemoryImage* atlas=main->mImage;
 // Measure existing ink, not padded cell size. Keep the donor's advance,
 // baseline, native colour and layer settings; the new glyph cannot balloon.
 if(atlas){
  const auto* bits=atlas->GetBits();int l=width,t=height,r=-1,b=-1,best=0;std::map<unsigned,int> colours;
  for(int y=0;y<height;++y)for(int x=0;x<width;++x){
   const auto p=bits[(rect.mY+y)*atlas->mWidth+rect.mX+x];if((p>>24)<100)continue;
   l=std::min(l,x);r=std::max(r,x);t=std::min(t,y);b=std::max(b,y);
   // Use the dominant solid ink, not a rare grey/white bevel highlight.
   // This matters for native green-inset almanac names.
   if((p>>24)>240){const int score=++colours[p&0xffffff];if(score>best){best=score;colour=p&0xffffff;}}
  }
  if(r>=l&&b>=t){left=l;right=r;top=t;bottom=b;}
 }
 if(right<left||bottom<top)return;
 static std::vector<std::unique_ptr<MemoryImage>> images;
 auto image=std::make_unique<MemoryImage>();image->Create(width,height);auto* bits=image->GetBits();std::fill(bits,bits+width*height,0u);
 const int w=right-left+1,h=bottom-top+1;
 auto sample=[&](int x,int y){const char v=mask[y*ExactGlyphPixels::Size+x];return (v<='9'?v-'0':v-'a'+10)*17;};
 for(int y=0;y<h;++y)for(int x=0;x<w;++x){
  // Box-filter complete font outlines down to the game's small bitmap size.
  const int x0=x*64/w,x1=std::max(x0+1,(x+1)*64/w),y0=y*64/h,y1=std::max(y0+1,(y+1)*64/h);int sum=0;
  for(int yy=y0;yy<y1;++yy)for(int xx=x0;xx<x1;++xx)sum+=sample(xx,yy);
  bits[(top+y)*width+left+x]=(unsigned(sum/((x1-x0)*(y1-y0)))<<24)|colour;
 }
 image->BitsChanged();glyph.mImageRect=Rect(0,0,width,height);
 // Remove empty placeholders left behind by earlier width queries.
 for(auto& l:data->mFontLayerList)l.mCharDataMap.erase(target);
 data->mFontLayerList.emplace_back(*main);auto& layer=data->mFontLayerList.back();layer.mLayerName=key;
 layer.mImage=image.get();images.push_back(std::move(image));layer.mCharDataMap.clear();layer.mCharDataMap.emplace(target,glyph);data->mFontLayerMap.emplace(key,&layer);
 font->mActiveListValid=false;font->Prepare();
}
// Compose missing characters using the same atlas, strokes and advance.
// Fractions describe source cells; each fragment retains its native placement.
struct Fragment {char32_t source;float x,y,w,h;float dx=0,dy=0;};
void Supplement(_Font* source,char32_t target,const char* name,std::initializer_list<Fragment> parts){
 auto* font=dynamic_cast<ImageFont*>(source);if(!font||!font->mFontData)return;
 auto* data=font->mFontData;const std::string key=std::string("MEMEGLYPH")+name;
 if(data->mFontLayerMap.contains(key+"0"))return;
 for(auto& layer:data->mFontLayerList){auto found=layer.mCharDataMap.find(target);if(found!=layer.mCharDataMap.end()&&found->second.mImageRect.mWidth>0)return;}
 FontLayer* main=nullptr;
 for(auto& layer:data->mFontLayerList){bool complete=true;for(const auto& p:parts){auto it=layer.mCharDataMap.find(p.source);if(it==layer.mCharDataMap.end()||it->second.mImageRect.mWidth<2)complete=false;}if(complete){main=&layer;break;}}
 if(!main)return;
 int index=0;
 for(const auto& part:parts){
  auto glyph=main->mCharDataMap.at(part.source);const int width=glyph.mImageRect.mWidth,height=glyph.mImageRect.mHeight;
  const int x=int(width*part.x+0.5f),y=int(height*part.y+0.5f);
  glyph.mImageRect.mX+=x;glyph.mImageRect.mY+=y;
  glyph.mImageRect.mWidth=int(width*(part.x+part.w)+0.5f)-x;glyph.mImageRect.mHeight=int(height*(part.y+part.h)+0.5f)-y;
  glyph.mOffset.mX+=x+int(width*part.dx);glyph.mOffset.mY+=y+int(height*part.dy);
  data->mFontLayerList.emplace_back(*main);auto& layer=data->mFontLayerList.back();layer.mLayerName=key+std::to_string(index++);layer.mCharDataMap.clear();layer.mCharDataMap.emplace(target,glyph);data->mFontLayerMap.emplace(layer.mLayerName,&layer);
 }
 font->mActiveListValid=false;font->Prepare();
}
void Repair(_Font* source) {
    auto* font=dynamic_cast<ImageFont*>(source);
    if(!font||!font->mFontData)return;
    auto* data=font->mFontData;
    if(data->mFontLayerMap.contains("SANDBOXFLAMELEFT"))return;
    FontLayer* main=nullptr;
    for(auto& layer:data->mFontLayerList)
        if(layer.mCharDataMap.contains(U'烤')&&layer.mCharDataMap.contains(U'陷')&&layer.mCharDataMap.contains(U'火')){main=&layer;break;}
    if(!main)return;
    const auto fire=main->mCharDataMap.at(U'火');
    const auto left=main->mCharDataMap.at(U'烤');
    const auto right=main->mCharDataMap.at(U'陷');
    if(left.mImageRect.mWidth<2||left.mImageRect.mWidth!=right.mImageRect.mWidth||left.mOffset!=right.mOffset)return;
    const int split=SandboxFontRules::RadicalSplit(left.mImageRect.mWidth,fire.mWidth,left.mOffset.mX);
    // Remove only our oversized supplemental glyph, never a complete native glyph.
    const auto existing=main->mCharDataMap.find(U'焰');
    if(existing!=main->mCharDataMap.end()&&existing->second.mImageRect.mWidth>0)return;
    for(auto& layer:data->mFontLayerList)layer.mCharDataMap.erase(U'焰');
    for(int half=0;half<2;++half){
        data->mFontLayerList.emplace_back(*main);
        auto& layer=data->mFontLayerList.back();
        layer.mLayerName=half?"SANDBOXFLAMERIGHT":"SANDBOXFLAMELEFT";
        layer.mCharDataMap.clear();
        auto glyph=half?right:left;
        glyph.mWidth=fire.mWidth;
        if(half){glyph.mImageRect.mX+=split;glyph.mImageRect.mWidth-=split;glyph.mOffset.mX+=split;}
        else glyph.mImageRect.mWidth=split;
        layer.mCharDataMap.emplace(U'焰',glyph);
        data->mFontLayerMap.emplace(layer.mLayerName,&layer);
    }
    font->mActiveListValid=false;
    font->Prepare();
}
}
void SandboxRepairFonts(){
    for(auto* font:{FONT_BRIANNETOD12,FONT_BRIANNETOD16,FONT_DWARVENTODCRAFT18,FONT_DWARVENTODCRAFT24,FONT_DWARVENTODCRAFT18YELLOW,FONT_DWARVENTODCRAFT18GREENINSET,FONT_DWARVENTODCRAFT18BRIGHTGREENINSET,FONT_HOUSEOFTERROR16,FONT_HOUSEOFTERROR28}){
        Repair(font);
        // The bullet is a missing-glyph box in this atlas. Raise its real
        // period by a quarter-cell to form the middle dot in 小·坚果.
        Supplement(font,U'·',"MIDDLEDOT",{{U'.',0,0,1,1,0,-.25f}});
        // 尢 from 优 (omit its top-right dot), and 介 from 价. Keep the
        // original bitmap strokes, cell size, baseline and character advance.
        Supplement(font,U'尬',"GA",{{U'优',.46f,0,.28f,.29f,-.40f,0},{U'优',.46f,.29f,.54f,.71f,-.40f,0},{U'价',.46f,0,.54f,1}});
        Supplement(font,U'汗',"HAN",{{U'池',0,0,.46f,1},{U'杆',.46f,0,.54f,1}});
        // 禾 from 种 and 少 from 沙 keep timing text in the native bitmap font.
        Supplement(font,U'秒',"MIAO",{{U'种',0,0,.46f,1},{U'沙',.46f,0,.54f,1}});
        // The right-hand bird radical from 鸣, without its inner eye dot.
        // Centre it inside the original cell; preserve native bitmap metrics.
        Supplement(font,U'乌',"WU",{{U'鸣',.46f,0,.54f,.4167f,-.16f,0},{U'鸣',.46f,.4167f,.08f,.0833f,-.16f,0},{U'鸣',.625f,.4167f,.375f,.0833f,-.16f,0},{U'鸣',.46f,.5f,.54f,.5f,-.16f,0}});
        for(const auto& glyph:ExactGlyphPixels::Glyphs)ExactGlyph(font,glyph.character,glyph.key,glyph.pixels);
        Supplement(font,U'罡',"GANG",{{U'四',0,0,1,0.46f},{U'正',0,0.46f,1,0.54f}});
        Supplement(font,U'梗',"GENG",{{U'样',0,0,0.46f,1},{U'硬',0.46f,0,0.54f,1}});
        Supplement(font,U'锅',"GUO",{{U'钢',0,0,0.46f,1},{U'蜗',0.46f,0,0.54f,1}});
        Supplement(font,U'甩',"SHUAI",{{U'用',0,0,1,0.55f},{U'用',0,0.55f,0.46f,0.45f},{U'电',0.46f,0.55f,0.54f,0.45f}});
        Supplement(font,U'蹭',"CENG",{{U'蹦',0,0,0.46f,1},{U'增',0.46f,0,0.54f,1}});
        Supplement(font,U'饭',"FAN",{{U'馆',0,0,0.46f,1},{U'板',0.46f,0,0.54f,1}});
        Supplement(font,U'杨',"YANG",{{U'样',0,0,0.46f,1},{U'场',0.46f,0,0.54f,1}});
        Supplement(font,U'摸',"MO",{{U'提',0,0,0.46f,1},{U'模',0.46f,0,0.54f,1}});
        Supplement(font,U'迪',"DI",{{U'边',0,0,0.46f,1},{U'边',0.46f,0.81f,0.54f,0.19f},{U'油',0.46f,0,0.54f,0.81f}});
    }
}
