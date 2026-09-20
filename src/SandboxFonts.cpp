// Complete the missing Chinese glyph from the player's existing bitmap font.
// The two radicals retain that font's strokes, padding, baseline and advance.
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "SandboxFonts.h"
#include "SandboxFontRules.h"
#include "Resources.h"
#include "graphics/ImageFont.h"
using namespace Sexy;
namespace {
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
    for(auto* font:{FONT_BRIANNETOD12,FONT_DWARVENTODCRAFT18,FONT_DWARVENTODCRAFT24})Repair(font);
}
