#pragma once
namespace Sexy { class Image;class MemoryImage;class Graphics;class SexyTransform2D; }
class Reanimation;
namespace SandboxArt {
Sexy::Image* Image(const char* family,const char* part);
// Original reanimation PNG, unchanged. Used for native-part fusion attachments.
Sexy::Image* NativeImage(const char* file);
Sexy::Image* PowerNative(const char* file,int level,int power);
Sexy::Image* WarmNative(const char* file,int level);
Sexy::Image* Phone(int damage=0);
Sexy::Image* Palm();
// Shared by attachment, card preview and projectile launch: opaque wrist
// point (6,29) of the 64x44 image sits inside the native 17x27 lip opening.
bool PalmMatrix(Reanimation*,Sexy::SexyTransform2D&);
void DrawPalm(Sexy::Graphics*,Reanimation*);
Sexy::Image* PhoneHands(const char* file);
void DrawFit(Sexy::Graphics* g,Sexy::MemoryImage* image,int x,int y,int width,int height,float scale=1.0f);
// Coordinates are relative to the caller's Graphics frame, exactly once.
void Sprite(Sexy::Graphics* g,const char* part,float cx,float cy,float width,float height,float angle=0,int alpha=255);
void Link(Sexy::Graphics* g,float x1,float y1,float x2,float y2,int frame,int alpha=220);
// Local texture point through the actual interpolated track AND attachment matrix.
bool TrackPoint(Reanimation* anim,const char* track,float width,float height,float px,float py,float& x,float& y);
}
