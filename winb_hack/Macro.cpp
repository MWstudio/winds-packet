#include <windows.h>
#include <direct.h> // _mkdir
#include "stdafx.h"
#include "Macro.h"
#include "Hooks.h"
#include "process.h"
#include <shlobj.h> // SHGetFolderPath �Լ�
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

// Ű ������Ʈ
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
		Sleep(300);
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
	//PostMessage(hwnd, WM_USER + 3, (WPARAM)text, length + 6);
	PostMessage(Macro::macroHWND, WM_USER + 3, (WPARAM)text, length + 6);
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

void Macro::transparency() { //����
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

void Macro::shadowlessStep() { //��������
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





