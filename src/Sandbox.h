#pragma once
class Board;
class PlayerInfo;
namespace Sexy { class Graphics; }
extern bool gSandboxEnabled;
bool SandboxEnter();
bool SandboxExit();
bool SandboxOwnsProfile(const PlayerInfo* profile);
void SandboxStart(int map);
void SandboxTick(Board* board);
void SandboxEscaped();
extern "C" int pvz_sandbox_command(int command, int type, int col, int row);
void SandboxUIReset();
void SandboxUITick(Board* board);
bool SandboxUIHasUnsaved();
void SandboxUIFeedback(int code);
void SandboxDrawUI(Sexy::Graphics* g);
bool SandboxMouseDown(int x, int y, int clicks);
bool SandboxMouseDrag(int x, int y);
bool SandboxMouseUp();
void SandboxKeyDown(int key);
