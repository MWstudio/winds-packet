#pragma once
#include <string>
#include <thread>
#include <atomic>

class Macro
{
	static struct Point {
		int x, y;
	};

public:
	Macro();
	~Macro();

	static HWND macroHWND;
	static DWORD WINAPI startCycle(LPVOID lpParam);
	static DWORD WINAPI startDiamond(LPVOID lpParam);
	static void consoleshowtext(const char* korStr);
	static void cycle();
	static void diamondBody();
	static void test();
	static void updateSkillKey(const std::string& skillName, unsigned short& key);

	static unsigned short playerId;
	static unsigned short playerX, playerY;
	static unsigned short mobX, mobY;
	static unsigned short selectedPlayerId;
	static unsigned short selectedPlayerX, selectedPlayerY;
	static unsigned short map1, map2, map3;

	static unsigned short necromancyReceivedMe;
	static unsigned short necromancyReceivedSelected;
	static unsigned short curseReceivedMe;
	static unsigned short curseReceivedSelected;
	static unsigned short runDiamond;

	static unsigned short cycleKey;
	static unsigned short divorceKey;
	static unsigned short diamondKey;
	static unsigned short exorcismKey;

	static unsigned short isCycle;
	static unsigned short isDiamond;
	static unsigned int isCtrl;
};

