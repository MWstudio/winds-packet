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

	static unsigned short playerX, playerY;
	static unsigned short playerId;
};

