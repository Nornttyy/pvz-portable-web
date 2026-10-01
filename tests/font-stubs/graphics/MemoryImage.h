#pragma once
#include <vector>
#include <cstdint>
namespace Sexy {
struct MemoryImage {int mWidth=0,mHeight=0;std::vector<uint32_t> bits;
 void Create(int w,int h){mWidth=w;mHeight=h;bits.resize(w*h);}
 uint32_t* GetBits(){return bits.data();}void BitsChanged(){}
};
}
