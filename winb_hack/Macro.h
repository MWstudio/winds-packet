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
	static DWORD WINAPI startAttack(LPVOID lpParam);
	static DWORD WINAPI startTransparency(LPVOID lpParam);
	static void consoleshowtext(const char* korStr);
	static void attack();
	static void transparency();
	static void shadowlessStep();
	static void updateSkillKey(const std::string& skillName, unsigned short& key);
	static unsigned int isCtrl;
	static unsigned short playerId;
	static unsigned short playerX, playerY;
	static unsigned short selectedPlayerId;
	static unsigned short selectedPlayerX, selectedPlayerY;
	static unsigned short map1, map2, map3;

	static unsigned short necromancyReceivedMe;
	static unsigned short necromancyReceivedSelected;
	static unsigned short curseReceivedMe;
	static unsigned short curseReceivedSelected;

	static unsigned short transparencyKey;
	static unsigned short shadowlessStepKey;

	static unsigned short isAttack;
	static unsigned short isTransparency;

};

