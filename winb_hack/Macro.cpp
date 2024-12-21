#include "stdafx.h"
#include "Macro.h"
#include "Hooks.h"
#include "process.h"

HWND Macro::macroHWND = NULL;

unsigned short Macro::playerX = 0, Macro::playerY = 0;
unsigned short Macro::playerId = 0;

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