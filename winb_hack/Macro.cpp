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
unsigned short Macro::mobX = 0, Macro::mobY = 0;
unsigned short Macro::playerId = 0;
unsigned short Macro::selectedPlayerX = 0, Macro::selectedPlayerY = 0;
unsigned short Macro::map1 = 0, Macro::map2 = 0, Macro::map3;

unsigned short Macro::necromancyReceivedMe = 0;
unsigned short Macro::necromancyReceivedSelected = 0;

unsigned short Macro::curseReceivedMe = 0;
unsigned short Macro::curseReceivedSelected = 0;

unsigned short Macro::cycleKey = 0;
unsigned short Macro::divorceKey = 0;
unsigned short Macro::diamondKey = 0;
unsigned short Macro::exorcismKey = 0;

unsigned short Macro::isCycle = 0;
unsigned short Macro::isDiamond = 0;
unsigned int Macro::isCtrl = 0;

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
	if (skillName == "cycle") {
		Macro::cycleKey = key;
		printf("%d Update cycle\n", Macro::cycleKey);
	}
	else if (skillName == "divorce") {
		Macro::divorceKey = key;
		printf("%d Update divorce\n", Macro::divorceKey);
	}
	else if (skillName == "diamond") {
		Macro::diamondKey = key;
		printf("%d Update diamond\n", Macro::diamondKey);
	}
	else if (skillName == "exorcism") {
		Macro::exorcismKey = key;
		printf("%d Update exorcism\n", Macro::exorcismKey);
	}
	else {
		printf("Unknown skill name: %s\n", skillName.c_str());
	}
}
unsigned char* korStrToHex(const char* korStr) {
	size_t charCount = strlen(korStr);
	static unsigned char buffer[BUFSIZ] = { 0, };

	for (size_t i = 0; i < charCount; i++) {
		unsigned char cc = *((unsigned char*)korStr + i);
		buffer[i] = cc;
	}
	return buffer;
}
void say() {
	// 하드코딩된 문자열
	const char* korStr = "나는빠박이다";

	unsigned char* hex = korStrToHex(korStr);
	int length = strlen(korStr);
	int i;
	int size = length + 4;

	// 패킷 초기화
	char packet[100] = { 0xe, 0x0, 0x5, 0x2f, 0x0, };

	// 문자열 길이를 패킷에 반영
	packet[2] = length;
	for (i = 3; i < length + 4; i++) {
		packet[i] = hex[i - 3];
	}

	unsigned char sendpacket[100] = { "0", };

	// 암호화 처리
	Hooks::LoadEncrypt(packet, size, Hooks::encrypted);

	sendpacket[0] = 0xAA;
	sendpacket[1] = 0x00;
	sendpacket[2] = size + 0x1;
	for (int i = 3; i < size + 4; i++) {
		sendpacket[i] = Hooks::encrypted[i - 3];
	}

	// 패킷 전송
	send(Hooks::Con_Packet_Socket, (const char*)sendpacket, size + 4, 0);
}

void Macro::test() {
	consoleshowtext("나는 빡빡이다");
}


void Macro::consoleshowtext(const char* korStr) {
	static unsigned char text[BUFSIZ] = { 0, };
	unsigned char* hex = korStrToHex(korStr);
	int length = strlen(korStr);
	int i;
	text[0] = 0x0A;
	text[1] = 0x00;
	text[2] = 0x00;
	text[3] = length + 1;

	for (i = 4; i < length + 5; i++)
		text[i] = hex[i - 4];
	text[i] = 0x00;

	PostMessage(Macro::macroHWND, WM_USER + 3, (WPARAM)text, length + 6);
}

DWORD WINAPI Macro::startCycle(LPVOID lpParam) {
	while (true) {
		if (isCycle == 1) {
			cycle();
		}
		Sleep(200);
	}
}

DWORD WINAPI Macro::startDiamond(LPVOID lpParam) {
	while (true) {
		if (isDiamond == 1)
			diamondBody();
		Sleep(500);
	}
}

void Macro::cycle() { //순환 0F 0A 00
	int size = 3;
	char packet[3] = { 0x0F, 0x0A, 0x00 };
	packet[1] = Macro::cycleKey;
	unsigned char sendpacket[100] = { "0", };
	Hooks::LoadEncrypt(packet, size, Hooks::encrypted);
	sendpacket[0] = 0xAA;
	sendpacket[1] = 0x00;
	sendpacket[2] = size + 0x1;
	for (int i = 3; i < size + 4; i++)
		sendpacket[i] = Hooks::encrypted[i - 3];


	send(Hooks::Con_Packet_Socket, (const char*)sendpacket, size + 4, 0);

}

void Macro::diamondBody() { //금강불체 0F 06 00 
	int size = 3;
	char packet[3] = { 0x0F, 0x06, 0x00 };
	packet[1] = Macro::diamondKey;
	unsigned char sendpacket[100] = { "0", };
	Hooks::LoadEncrypt(packet, size, Hooks::encrypted);
	sendpacket[0] = 0xAA;
	sendpacket[1] = 0x00;
	sendpacket[2] = size + 0x1;
	for (int i = 3; i < size + 4; i++)
		sendpacket[i] = Hooks::encrypted[i - 3];


	send(Hooks::Con_Packet_Socket, (const char*)sendpacket, size + 4, 0);
}






