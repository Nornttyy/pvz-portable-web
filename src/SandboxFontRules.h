// Bitmap cells include padding: never scale ink to fill their whole rectangle.
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once
#include <algorithm>
namespace SandboxFontRules {
constexpr int RadicalSplit(int cellWidth,int advance,int offsetX) {
    return std::clamp(-offsetX+(advance*42+50)/100,1,cellWidth-1);
}
}
