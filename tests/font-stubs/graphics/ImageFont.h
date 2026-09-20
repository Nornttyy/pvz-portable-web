// State-only framework double for exercising the real SandboxFonts.cpp repair.
// Native compilation and WASM initialization are checked separately.
#pragma once
#include <list>
#include <map>
#include <string>
namespace Sexy {
struct Point {int mX=0,mY=0;bool operator==(const Point&)const=default;};
struct Rect {int mX=0,mY=0,mWidth=0,mHeight=0;bool operator==(const Rect&)const=default;};
struct CharData {Rect mImageRect;Point mOffset;int mWidth=0;};
struct FontLayer {
    std::string mLayerName;
    std::map<char32_t,CharData> mCharDataMap;
    int mImage=0,mAscent=0,mHeight=0,mPointSize=0;
};
struct FontData {std::list<FontLayer> mFontLayerList;std::map<std::string,FontLayer*> mFontLayerMap;};
class _Font {public:virtual~_Font()=default;};
class ImageFont:public _Font {
public:FontData* mFontData=nullptr;bool mActiveListValid=true;int prepared=0;
    void Prepare(){++prepared;mActiveListValid=true;}
};
}
