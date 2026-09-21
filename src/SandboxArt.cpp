// Per-instance atlas parts; original image definitions remain untouched.
#include "SandboxArt.h"
#include "LawnApp.h"
#include "graphics/GLImage.h"
#include "graphics/Graphics.h"
#include <map>
#include <memory>
#include <string>
#include <algorithm>
namespace SandboxArt {
Sexy::Image* Image(const char* family,const char* part){
 static std::map<std::string,std::unique_ptr<Sexy::GLImage>> cache;
 const auto file=std::string("images/sandbox/")+family+"-"+part+".png";
 auto& image=cache[file];if(!image)image.reset(gLawnApp->GetImage(file));return image.get();
}
void DrawFit(Sexy::Graphics* g,Sexy::MemoryImage* image,int x,int y,int width,int height,float scale){
 // Fit visible ink, not the unequal transparent margins of native animation caches.
 static std::map<Sexy::MemoryImage*,Sexy::Rect> bounds;
 auto it=bounds.find(image);
 if(it==bounds.end()){
  auto* bits=image->GetBits();int left=image->mWidth,top=image->mHeight,right=0,bottom=0;
  for(int py=0;py<image->mHeight;++py)for(int px=0;px<image->mWidth;++px)
   if((bits[py*image->mWidth+px]>>24)>16){left=std::min(left,px);right=std::max(right,px);top=std::min(top,py);bottom=std::max(bottom,py);}
  if(left>right||top>bottom)return;
  it=bounds.emplace(image,Sexy::Rect(left,top,right-left+1,bottom-top+1)).first;
 }
 const auto& r=it->second;const float fit=std::min(float(width)/r.mWidth,float(height)/r.mHeight)*scale;
 const int w=std::max(1,int(r.mWidth*fit)),h=std::max(1,int(r.mHeight*fit));
 g->DrawImage(image,Sexy::Rect(x+(width-w)/2,y+height-h,w,h),r);
}
}
