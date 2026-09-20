#pragma once
#include <string>
#include "SandboxUIRules.h"
namespace Sexy {class Graphics;}
void SandboxDrawButton(Sexy::Graphics* g,SandboxUIRules::Box b,const std::string& label,bool down,bool hover,bool large=false);
