#pragma once
namespace Sexy { class Image;class MemoryImage;class Graphics; }
namespace SandboxArt {
Sexy::Image* Image(const char* family,const char* part);
void DrawFit(Sexy::Graphics* g,Sexy::MemoryImage* image,int x,int y,int width,int height,float scale=1.0f);
}
