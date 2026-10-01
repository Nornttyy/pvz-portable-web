// Per-instance atlas parts; original image definitions remain untouched.
#include "SandboxArt.h"
#include "SandboxMemeRules.h"
#include "AbstractPhonePixels.h"
#include "CactusPalmPixels.h"
#include "CleverHeadPixels.h"
#include "SandboxZombies.h"
#include "NukeShroomRules.h"
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
Sexy::Image* StinkCap(const char* file){
 static std::map<std::string,std::unique_ptr<Sexy::MemoryImage>> images;auto& out=images[file];if(out)return out.get();
 auto* source=dynamic_cast<Sexy::MemoryImage*>(NativeImage(file));if(!source)return nullptr;
 out=std::make_unique<Sexy::MemoryImage>();out->Create(source->mWidth,source->mHeight);
 const auto* in=source->GetBits();auto* bits=out->GetBits();
 for(int i=0;i<source->mWidth*source->mHeight;++i){
  const auto p=in[i];const int r=(p>>16)&255,g=(p>>8)&255,b=p&255,hi=std::max(r,b);
  // Cocoa-coloured pigment, retaining native spots, highlights, ink and alpha.
  bits[i]=(p>>24)&&r>g+4&&b>g+4&&hi>40?(p&0xff000000u)|(unsigned(hi)<<16)|(unsigned(g+(hi-g)*42/100)<<8)|unsigned(g*82/100):p;
 }
 out->BitsChanged();return out.get();
}
Sexy::Image* NauseatedImage(Sexy::Image* image){
 auto* source=dynamic_cast<Sexy::MemoryImage*>(image);if(!source)return image;
 static std::map<Sexy::Image*,std::unique_ptr<Sexy::MemoryImage>> images;auto& out=images[image];if(out)return out.get();
 out=std::make_unique<Sexy::MemoryImage>();out->Create(source->mWidth,source->mHeight);out->mNumCols=source->mNumCols;out->mNumRows=source->mNumRows;
 const auto* in=source->GetBits();auto* bits=out->GetBits();
 for(int i=0;i<source->mWidth*source->mHeight;++i){
  const auto p=in[i];const int r=(p>>16)&255,g=(p>>8)&255,b=p&255,hi=std::max({r,g,b}),lo=std::min({r,g,b});
  // Hue replacement, not a dark multiplicative wash: purple/red bodies also
  // become green. White eyes, black outlines and transparent pixels stay intact.
  bits[i]=(p>>24)&&hi>30&&hi-lo>=12?(p&0xff000000u)|(unsigned(lo+(hi-lo)*30/100)<<16)|(unsigned(hi)<<8)|unsigned(lo+(hi-lo)*18/100):p;
 }
 out->BitsChanged();return out.get();
}
Sexy::Image* WaterDrop(){
 static std::unique_ptr<Sexy::GLImage> image;if(!image)image.reset(gLawnApp->GetImage("images/waterdrop.png"));return image.get();
}
Sexy::Image* NukeNative(const char* file,int phase){
 phase=(phase%8+8)%8;static std::map<std::pair<std::string,int>,std::unique_ptr<Sexy::MemoryImage>> images;
 auto& out=images[{file,phase}];if(out)return out.get();
 auto* source=dynamic_cast<Sexy::MemoryImage*>(NativeImage(file));if(!source)return nullptr;
 out=std::make_unique<Sexy::MemoryImage>();out->Create(source->mWidth,source->mHeight);
 const auto* in=source->GetBits();auto* bits=out->GetBits();
 for(int y=0;y<source->mHeight;++y)for(int x=0;x<source->mWidth;++x){const int i=y*source->mWidth+x;bits[i]=NukeShroomRules::EnergyPixel(in[i],x,y,phase);}
 out->BitsChanged();return out.get();
}
void DrawNukeEnergy(Sexy::Graphics* g,Reanimation* body,int age){
 float x=0,y=0;bool found=false;
 for(const char* track:{"DoomShroom_head3","DoomShroom_head2","DoomShroom_head1","DoomShroom_sleepinghead"}){
  const auto file=std::string(track)+".png";auto* head=NativeImage(file.c_str());
  if(head&&TrackPoint(body,track,head->mWidth,head->mHeight,head->mWidth*.5f,5,x,y)){found=true;break;}
 }
 auto* ribbon=NativeImage("puff_3.png");if(!found||!ribbon)return;
 for(int i=0;i<3;++i){const float t=((age+i*23)%70)/70.f;
  Sexy::SexyTransform2D m;m.LoadIdentity();m.m00=(4+2*(1-t))/ribbon->mWidth;m.m11=(11+7*t)/ribbon->mHeight;
  m.m02=x+(i-1)*17+std::sin(t*5+i)*3+g->mTransX;m.m12=y-7-t*24+g->mTransY;
  PvzpBltMatrix(g,ribbon,m,g->mClipRect,Sexy::Color(100,225,60,int(150*(1-t))),g->mDrawMode,Sexy::Rect(0,0,ribbon->mWidth,ribbon->mHeight));
 }
}
Sexy::Image* GreenCone(int damage){
 damage=std::clamp(damage,0,2);static std::unique_ptr<Sexy::MemoryImage> images[3];auto& out=images[damage];if(out)return out.get();
 const char* files[]={"Zombie_cone1.png","Zombie_cone2.png","Zombie_cone3.png"};
 auto* source=dynamic_cast<Sexy::MemoryImage*>(NativeImage(files[damage]));if(!source)return nullptr;
 out=std::make_unique<Sexy::MemoryImage>();out->Create(source->mWidth,source->mHeight);
 const auto* in=source->GetBits();auto* bits=out->GetBits();
 for(int i=0;i<source->mWidth*source->mHeight;++i){
  const auto p=in[i];const int r=(p>>16)&255,g=(p>>8)&255,b=p&255;
  // Recolour orange pigment only. Native outlines, white band, alpha and
  // all three damaged silhouettes remain unchanged.
  bits[i]=(p>>24)&&r>g+12&&g>=b&&r>40?(p&0xff000000u)|(unsigned(g*.62f)<<16)|(unsigned(r*.82f)<<8)|unsigned(b*.5f+g*.22f):p;
 }
 out->BitsChanged();return out.get();
}
Sexy::Image* ConeTower(int armor){
 using namespace SandboxZombies;const int count=TowerCount(armor);if(!count)return nullptr;
 const int damage=ConeDamageStage(TowerTopHealth(armor));
 static std::map<int,std::unique_ptr<Sexy::MemoryImage>> images;auto& out=images[count*3+damage];if(out)return out.get();
 const char* files[]={"Zombie_cone1.png","Zombie_cone2.png","Zombie_cone3.png"};
 auto* whole=dynamic_cast<Sexy::MemoryImage*>(NativeImage(files[0]));
 auto* top=dynamic_cast<Sexy::MemoryImage*>(NativeImage(files[damage]));if(!whole||!top)return nullptr;
 const int width=whole->mWidth,height=whole->mHeight+TowerConeRise*(count-1);
 out=std::make_unique<Sexy::MemoryImage>();out->Create(width,height);auto* bits=out->GetBits();std::fill(bits,bits+width*height,0u);
 // Nested original cones, bottom first. Every extra cone contributes a
 // visible rim; only the exposed top cone carries partial armour damage.
 for(int part=0;part<count;++part){
  auto* source=part==count-1?top:whole;const auto* in=source->GetBits();const int dy=TowerConeRise*(count-1-part);
  for(int y=0;y<source->mHeight;++y)for(int x=0;x<std::min(width,source->mWidth);++x){
   const unsigned s=in[y*source->mWidth+x],sa=s>>24;if(!sa||y+dy>=height)continue;
   auto& d=bits[(y+dy)*width+x];const unsigned da=d>>24,a=sa+(da*(255-sa)+127)/255;
   unsigned rgb=0;for(int shift:{0,8,16}){
    const unsigned v=(((s>>shift)&255)*sa+(((d>>shift)&255)*da*(255-sa)+127)/255+a/2)/a;rgb|=v<<shift;
   }d=(a<<24)|rgb;
  }
 }
 out->BitsChanged();return out.get();
}
Sexy::Image* CleverHead(bool jawPose){
 static std::unique_ptr<Sexy::MemoryImage> images[2];auto& image=images[jawPose?1:0];
 if(!image){using namespace CleverHeadPixels;image=std::make_unique<Sexy::MemoryImage>();image->Create(Width,Height);
  const auto* pixels=jawPose?Jaw:Neutral;std::copy(pixels,pixels+Width*Height,image->GetBits());image->BitsChanged();
 }return image.get();
}
Sexy::Image* Palm(){
 static std::unique_ptr<Sexy::MemoryImage> image;if(!image){
  using namespace CactusPalmPixels;image=std::make_unique<Sexy::MemoryImage>();image->Create(Width,Height);
  std::copy(Pixels,Pixels+Width*Height,image->GetBits());image->BitsChanged();
 }return image.get();
}
bool PalmMatrix(Reanimation* anim,Sexy::SexyTransform2D& matrix){
 if(!anim||!anim->TrackExists("Cactus_lips"))return false;
 const int track=anim->FindTrackIndex("Cactus_lips");ReanimatorTransform pose;anim->GetCurrentTransform(track,&pose);
 if(pose.mFrame<0||pose.mAlpha<=0)return false;
 anim->GetTrackMatrix(track,matrix);
 // Native lip opening (5,13.5), actual opaque wrist (6,29). Register skin,
 // not transparent image bounds, while leaving the green tube/rim untouched.
 const float sx=40.f/64,sy=27.5f/44,dx=5-17*.5f+(64*.5f-6)*sx,dy=(44*.5f-29)*sy;
 matrix.m02+=matrix.m00*dx+matrix.m01*dy;matrix.m12+=matrix.m10*dx+matrix.m11*dy;
 matrix.m00*=sx;matrix.m10*=sx;matrix.m01*=sy;matrix.m11*=sy;
 return std::isfinite(matrix.m02)&&std::isfinite(matrix.m12);
}
void DrawPalm(Sexy::Graphics* g,Reanimation* anim){
 Sexy::SexyTransform2D m;if(!PalmMatrix(anim,m))return;auto* image=Palm();
 m.m02+=g->mTransX;m.m12+=g->mTransY;
 // Draw the wrist over the dark opening, not behind the opaque entire rim
 // image. The native body/tube/lips have already been drawn unchanged.
 PvzpBltMatrix(g,image,m,g->mClipRect,Sexy::Color(255,255,255),g->mDrawMode,Sexy::Rect(0,0,image->mWidth,image->mHeight));
}
Sexy::Image* Phone(int damage){
 damage=std::clamp(damage,0,2);static std::unique_ptr<Sexy::MemoryImage> images[3];auto& im=images[damage];if(im)return im.get();
 using namespace AbstractPhonePixels;im=std::make_unique<Sexy::MemoryImage>();im->Create(Width,Height);auto* out=im->GetBits();std::copy(Pixels,Pixels+Width*Height,out);
 // Cracks are a small native-resolution damage effect, not a second character redraw.
 auto line=[&](int x0,int y0,int x1,int y1){const int steps=std::max(std::abs(x1-x0),std::abs(y1-y0));for(int i=0;i<=steps;++i){const int x=x0+(x1-x0)*i/steps,y=y0+(y1-y0)*i/steps;if(x>=0&&x<Width&&y>=0&&y<Height&&(out[y*Width+x]>>24)>128)out[y*Width+x]=0xffb7c5b2;}};
 if(damage){line(45,35,50,47);line(50,47,42,58);line(50,47,66,41);}
 if(damage==2){line(50,47,29,41);line(34,43,25,54);line(52,45,61,30);}
 im->BitsChanged();return im.get();
}
Sexy::Image* PhoneHands(const char* file){
 static std::map<std::string,std::unique_ptr<Sexy::MemoryImage>> images;auto& out=images[file];if(out)return out.get();
 auto* source=dynamic_cast<Sexy::MemoryImage*>(NativeImage(file));if(!source)return nullptr;const int w=source->mWidth,h=source->mHeight;const auto* in=source->GetBits();
 out=std::make_unique<Sexy::MemoryImage>();out->Create(w,h);auto* bits=out->GetBits();std::fill(bits,bits+w*h,0u);
 // Preserve the exact original green fingers and their two-pixel outline, removing paper between them.
 for(int y=0;y<h;++y)for(int x=0;x<w;++x){const auto px=in[y*w+x];const int r=(px>>16)&255,g=(px>>8)&255,b=px&255;
  if((px>>24)>80&&g>r+8&&g>b+3)for(int yy=std::max(0,y-2);yy<std::min(h,y+3);++yy)for(int xx=std::max(0,x-2);xx<std::min(w,x+3);++xx)bits[yy*w+xx]=in[yy*w+xx];
 }
 out->BitsChanged();return out.get();
}
Sexy::Image* NativeImage(const char* file){
 static std::map<std::string,std::unique_ptr<Sexy::GLImage>> cache;
 const auto path=std::string("reanim/")+file;
 auto& image=cache[path];if(!image)image.reset(gLawnApp->GetImage(path));return image.get();
}
Sexy::Image* WarmNative(const char* file,int level){return PowerNative(file,level,180);}
Sexy::Image* PowerNative(const char* file,int level,int power){
 level=std::clamp(level,0,24);if(!level)return NativeImage(file);
 static std::map<std::pair<std::string,int>,std::unique_ptr<Sexy::MemoryImage>> cache;
 auto& image=cache[{file,level+(power-180)*25}];if(image)return image.get();
 auto* source=dynamic_cast<Sexy::MemoryImage*>(NativeImage(file));if(!source)return nullptr;
 image=std::make_unique<Sexy::MemoryImage>();image->Create(source->mWidth,source->mHeight);
 auto* out=image->GetBits();const auto* in=source->GetBits();
 for(int i=0;i<source->mWidth*source->mHeight;++i)out[i]=SandboxMemeRules::PowerPixel(in[i],level,power);
 image->BitsChanged();return image.get();
}
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
