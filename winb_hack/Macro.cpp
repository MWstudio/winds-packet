#include "stdafx.h"
#include "Macro.h"
#include "Hooks.h"
#include "process.h"

HWND Macro::macroHWND = NULL;

unsigned short Macro::selectedPlayerId = 0;
unsigned short Macro::playerX = 0, Macro::playerY = 0;
unsigned short Macro::playerId = 0;
unsigned short Macro::selectedPlayerX = 0, Macro::selectedPlayerY = 0;
unsigned short Macro::map1 = 0, Macro::map2 = 0;

unsigned short Macro::necromancyReceivedMe = 0;
unsigned short Macro::necromancyReceivedSelected = 0;

unsigned short Macro::curseReceivedMe = 0;
unsigned short Macro::curseReceivedSelected = 0;
// INTERVAL_DATA structure for passing datas to thread
typedef struct Data {
	double msec;
	const char* funcName;
	BOOL threadCheck = false;
}INTERVAL_DATA;

INTERVAL_DATA data;

Macro::Macro()
{
}

Macro::~Macro()
{
}