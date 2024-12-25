#include <windows.h>
#include <direct.h> // _mkdir
#include "stdafx.h"
#include "Macro.h"
#include "Hooks.h"
#include "process.h"
#include <shlobj.h> // SHGetFolderPath 함수
#include <fstream>
#include <sstream>
#include <iostream>
#include <atomic>
#include <thread>
#include <map>
#include <chrono>
HWND Macro::macroHWND = NULL;

unsigned short Macro::selectedPlayerId = 0;
unsigned short Macro::playerX = 0, Macro::playerY = 0;
unsigned short Macro::playerId = 0;
unsigned short Macro::selectedPlayerX = 0, Macro::selectedPlayerY = 0;
unsigned short Macro::map1 = 0, Macro::map2 = 0, Macro::map3;

unsigned short Macro::necromancyReceivedMe = 0;
unsigned short Macro::necromancyReceivedSelected = 0;

unsigned short Macro::curseReceivedMe = 0;
unsigned short Macro::curseReceivedSelected = 0;

unsigned short Macro::transparencyKey = 0;
unsigned short Macro::shadowlessStepKey = 0;

unsigned short Macro::isAttack = 0;
unsigned short Macro::isTransparency = 0;

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

// 키 업데이트
void Macro::updateSkillKey(const std::string& skillName, unsigned short& key) {
	printf("Update Skill Key");
	if (skillName == "transparency") {
		Macro::transparencyKey = key;
		printf("%d Update transparency\n", Macro::transparencyKey);
	}
	else if (skillName == "shadowlessStep") {
		Macro::shadowlessStepKey = key;
		printf("%d Update shadowlessStep\n", Macro::shadowlessStepKey);
	}

	else {
		printf("Unknown skill name: %s\n", skillName.c_str());
	}
}

DWORD WINAPI Macro::startAttack(LPVOID lpParam) {
	while (true) {
		if (isAttack == 1)
			attack();
		Sleep(100);
	}
}

DWORD WINAPI Macro::startTransparency(LPVOID lpParam) {
	while (true) {
		if (isTransparency == 1) {
			transparency();
			Sleep(200);
			shadowlessStep();
		}
		Sleep(100);
	}
}

void Macro::attack() {
	// 43 01 00 00 00 00 00
	int size = 2;
	char packet[2] = { 0x13, 0x00};
	unsigned char sendpacket[100] = { "0", };
	Hooks::LoadEncrypt(packet, size, Hooks::encrypted);
	sendpacket[0] = 0xAA;
	sendpacket[1] = 0x00;
	sendpacket[2] = size + 0x1;
	for (int i = 3; i < size + 4; i++)
		sendpacket[i] = Hooks::encrypted[i - 3];


	send(Hooks::Con_Packet_Socket, (const char*)sendpacket, size + 4, 0);
	/*int size = 2;
	char packet[2] = { 0x13, 0x00};
	unsigned char sendpacket[100] = { "0", };
	Hooks::LoadEncrypt(packet, size, Hooks::encrypted);
	sendpacket[0] = 0xAA;
	sendpacket[1] = 0x00;
	sendpacket[2] = size + 0x1;
	for (int i = 3; i < size + 4; i++)
		sendpacket[i] = Hooks::encrypted[i - 3];


	send(Hooks::Con_Packet_Socket, (const char*)sendpacket, size + 4, 0);*/

}

void Macro::transparency() { //투명
	int size = 3;
	char packet[3] = { 0x0F, 0x00, 0x00 };
	packet[1] = Macro::transparencyKey;
	unsigned char sendpacket[100] = { "0", };
	Hooks::LoadEncrypt(packet, size, Hooks::encrypted);
	sendpacket[0] = 0xAA;
	sendpacket[1] = 0x00;
	sendpacket[2] = size + 0x1;
	for (int i = 3; i < size + 4; i++)
		sendpacket[i] = Hooks::encrypted[i - 3];


	send(Hooks::Con_Packet_Socket, (const char*)sendpacket, size + 4, 0);
}

void Macro::shadowlessStep() { //무영보법
	int size = 3;
	char packet[3] = { 0x0F, 0x00, 0x00 };
	packet[1] = Macro::shadowlessStepKey;
	unsigned char sendpacket[100] = { "0", };
	Hooks::LoadEncrypt(packet, size, Hooks::encrypted);
	sendpacket[0] = 0xAA;
	sendpacket[1] = 0x00;
	sendpacket[2] = size + 0x1;
	for (int i = 3; i < size + 4; i++)
		sendpacket[i] = Hooks::encrypted[i - 3];


	send(Hooks::Con_Packet_Socket, (const char*)sendpacket, size + 4, 0);
}





