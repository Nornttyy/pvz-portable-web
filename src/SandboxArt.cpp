// Per-instance atlas parts; original image definitions remain untouched.
#include "SandboxArt.h"
#include "LawnApp.h"
#include "graphics/GLImage.h"
#include "graphics/Graphics.h"
#include "PvzpLib/Reanimator.h"
#include "PvzpLib/PvzpCommon.h"
#include <map>
#include <memory>
#include <string>
#include <algorithm>
#include <cmath>
namespace SandboxArt {
Sexy::Image* Image(const char* family,const char* part){
 static std::map<std::string,std::unique_ptr<Sexy::GLImage>> cache;
 const auto file=std::string("images/sandbox/")+family+"-"+part+".png";
 auto& image=cache[file];if(!image)image.reset(gLawnApp->GetImage(file));return image.get();
}
void Sprite(Sexy::Graphics* g,const char* part,float cx,float cy,float width,float height,float angle,int alpha){
 auto* image=Image("vfx",part);if(!image||width<=0||height<=0||alpha<=0)return;
 const float c=std::cos(angle),s=std::sin(angle),sx=width/image->mWidth,sy=height/image->mHeight;
 Sexy::SexyTransform2D m;m.LoadIdentity();
 m.m00=c*sx;m.m10=s*sx;m.m01=-s*sy;m.m11=c*sy;
 m.m02=cx+g->mTransX;m.m12=cy+g->mTransY;
 PvzpBltMatrix(g,image,m,g->mClipRect,Sexy::Color(255,255,255,std::clamp(alpha,0,255)),g->mDrawMode,Sexy::Rect(0,0,image->mWidth,image->mHeight));
}
void Link(Sexy::Graphics* g,float x1,float y1,float x2,float y2,int frame,int alpha){
 const float dx=x2-x1,dy=y2-y1,len=std::hypot(dx,dy);if(len<1)return;
 // Short tiled sections preserve the lightning's thickness instead of stretching a whole atlas.
 const int count=std::max(1,int(std::ceil(len/38)));const float angle=std::atan2(dy,dx);
 for(int i=0;i<count;++i){const float t=(i+0.5f)/count;
  Sprite(g,frame%2?"link-1":"link-0",x1+dx*t,y1+dy*t,len/count+2,9,angle,alpha);
 }
}
bool TrackPoint(Reanimation* a,const char* track,float width,float height,float px,float py,float& x,float& y){
 if(!a||!a->TrackExists(track))return false;
 const int index=a->FindTrackIndex(track);ReanimatorTransform pose;a->GetCurrentTransform(index,&pose);
 if(pose.mFrame<0||pose.mAlpha<=0)return false;
 Sexy::SexyTransform2D matrix;a->GetTrackMatrix(index,matrix);
 px-=width*0.5f;py-=height*0.5f;
 x=matrix.m00*px+matrix.m01*py+matrix.m02;y=matrix.m10*px+matrix.m11*py+matrix.m12;
 return std::isfinite(x)&&std::isfinite(y);
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
