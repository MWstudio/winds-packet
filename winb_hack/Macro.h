#pragma once
#include <string>
#include <thread>

class Macro
{
	static struct Point {
		int x, y;
	};

public:
	Macro();
	~Macro();

	static HWND macroHWND;

	static unsigned short playerId;
	static unsigned short playerX, playerY;
	static unsigned short selectedPlayerId;
	static unsigned short selectedPlayerX, selectedPlayerY;
	static unsigned short map1, map2;

	static unsigned short necromancyReceivedMe;
	static unsigned short necromancyReceivedSelected;
	static unsigned short curseReceivedMe;
	static unsigned short curseReceivedSelected;
};

