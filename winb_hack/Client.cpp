#include "stdafx.h"
#include "Client.h"
#include "bytebuffer.h"
#include "tchar.h"
#include <iterator>
#include "Macro.h"

#define DEFAULT_BUFLEN 2000
#define REACHABLE_X_DIST 3
#define REACHABLE_Y_DIST 3 
#define MAX_SAME_POS 3

Client::Client()
{
	
}


Client::~Client()
{
}

// connect function hook callback
void Client::Con_Packet_Hook_Callback()
{
	printf("---- %x\n", hooks->Con_Packet_Return_Address);
	printf("----Socket: %x\n", hooks->Con_Packet_Socket);
}


void Client::Send_Packet_Hook_Callback()
{
	// if you want to see send packets, delete the remark below
	
	ByteBuffer Packet((LPVOID)hooks->Outgoing_Packet_Pointer, hooks->Outgoing_Packet_Length);
	std::vector<uint8_t> data = Packet.ReadBytes(0, hooks->Outgoing_Packet_Length);
	std::stringstream result;
	std::copy(data.begin(), data.end(), std::ostream_iterator<int>(result, " "));
	std::string test = result.str();
	printf("Client is sending... : \n");
	//printf("%zu: ", data.size()); // data.size() Ãâ·Â
	//for (int i = 0; i < data.size(); i++) {
	//	printf("%02X ", data[i]);
	//}
	//printf("\n");
	if (data[0] == 0x32 && hooks->Outgoing_Packet_Length == 8) {
		if (data[1] == 0x00) {
			Macro::playerX = data[4];
			Macro::playerY = data[6] - 1;
			printf("%s %d\n", "playerX:", Macro::playerX);
			printf("%s %d\n", "playerY:", Macro::playerY);
		}
		if (data[1] == 0x01) {
			Macro::playerX = data[4] + 1;
			Macro::playerY = data[6];
			printf("%s %d\n", "playerX:", Macro::playerX);
			printf("%s %d\n", "playerY:", Macro::playerY);
		}
		if (data[1] == 0x02) {
			Macro::playerX = data[4];
			Macro::playerY = data[6] + 1;
			printf("%s %d\n", "playerX:", Macro::playerX);
			printf("%s %d\n", "playerY:", Macro::playerY);
		}
		if (data[1] == 0x03) {
			Macro::playerX = data[4] - 1;
			Macro::playerY = data[6];
			printf("%s %d\n", "playerX:", Macro::playerX);
			printf("%s %d\n", "playerY:", Macro::playerY);
		}
	}
	if (data[0] == 0x06 && hooks->Outgoing_Packet_Length == 16) {
		if (data[1] == 0x00) {
			Macro::playerX = data[4];
			Macro::playerY = data[6] - 1;
			printf("%s %d\n", "playerX:", Macro::playerX);
			printf("%s %d\n", "playerY:", Macro::playerY);
		}
		if (data[1] == 0x01) {
			Macro::playerX = data[4] + 1;
			Macro::playerY = data[6];
			printf("%s %d\n", "playerX:", Macro::playerX);
			printf("%s %d\n", "playerY:", Macro::playerY);
		}
		if (data[1] == 0x02) {
			Macro::playerX = data[4];
			Macro::playerY = data[6] + 1;
			printf("%s %d\n", "playerX:", Macro::playerX);
			printf("%s %d\n", "playerY:", Macro::playerY);
		}
		if (data[1] == 0x03) {
			Macro::playerX = data[4] - 1;
			Macro::playerY = data[6];
			printf("%s %d\n", "playerX:", Macro::playerX);
			printf("%s %d\n", "playerY:", Macro::playerY);
		}
	}
	if (data[0] == 0x06 && hooks->Outgoing_Packet_Length == 20) {
		if (data[1] == 0x00) {
			Macro::playerX = data[4];
			Macro::playerY = data[6] - 1;
			printf("%s %d\n", "playerX:", Macro::playerX);
			printf("%s %d\n", "playerY:", Macro::playerY);
		}
		if (data[1] == 0x01) {
			Macro::playerX = data[4] + 1;
			Macro::playerY = data[6];
			printf("%s %d\n", "playerX:", Macro::playerX);
			printf("%s %d\n", "playerY:", Macro::playerY);
		}
		if (data[1] == 0x02) {
			Macro::playerX = data[4];
			Macro::playerY = data[6] + 1;
			printf("%s %d\n", "playerX:", Macro::playerX);
			printf("%s %d\n", "playerY:", Macro::playerY);
		}
		if (data[1] == 0x03) {
			Macro::playerX = data[4] - 1;
			Macro::playerY = data[6];
			printf("%s %d\n", "playerX:", Macro::playerX);
			printf("%s %d\n", "playerY:", Macro::playerY);
		}
	}
	printf("\n");
	
	//ByteBuffer Packet((LPVOID)hooks->Outgoing_Packet_Pointer, hooks->Outgoing_Packet_Length);
	//printf("Client is sending a packet... Length: %x, Type: %x\n", hooks->Outgoing_Packet_Length, Packet.ReadUint16(0));
	////	Packet.WriteBytes(0, space, 2);
	//std::vector<uint8_t> data = Packet.ReadBytes(0, hooks->Outgoing_Packet_Length);
	//std::stringstream result;
	//std::copy(data.begin(), data.end(), std::ostream_iterator<int>(result, " "));
	//std::string test = result.str();
}

DWORD WINAPI curse(LPVOID lpParam) { // 0F 0D 01 03 2A 22 00 07 00 08 00 Åð¸¶
	printf("curse : x,y,id: %d, %d %d\n", Macro::playerX, Macro::playerY, Macro::playerId);
	int size = 11;
	char packet[11] = { 0x0F, 0x0D, 0x01, 0x03, 0x2A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	if (Macro::playerX == 0 && Macro::playerX == 0) {
		CloseHandle(GetCurrentThread());
		return 0;
	}
	if (Macro::playerId == 0) {
		CloseHandle(GetCurrentThread());
		return 0;
	}

	packet[5] = Macro::playerId;
	packet[7] = Macro::playerX;
	packet[9] = Macro::playerY;
	unsigned char sendpacket[100] = { "0", };
	Hooks::LoadEncrypt(packet, size, Hooks::encrypted);
	sendpacket[0] = 0xAA;
	sendpacket[1] = 0x00;
	sendpacket[2] = size + 0x1;
	for (int i = 3; i < size + 4; i++)
		sendpacket[i] = Hooks::encrypted[i - 3];
	send(Hooks::Con_Packet_Socket, (const char*)sendpacket, size + 4, 0);
	CloseHandle(GetCurrentThread());
	return 0;
}

DWORD WINAPI necromancy(LPVOID lpParam) { // 0F 08 01 03 2A 22 00 07 00 08 00 ÆÄÈ¥
	printf("necromancy : x,y,id: %d, %d %d\n", Macro::playerX, Macro::playerY, Macro::playerId);
	int size = 11;
	char packet[11] = { 0x0F, 0x08, 0x01, 0x03, 0x2A, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00 };
	if (Macro::playerX == 0 && Macro::playerX == 0) {
		CloseHandle(GetCurrentThread());
		return 0;
	}
	if (Macro::playerId == 0) {
		CloseHandle(GetCurrentThread());
		return 0;
	}

	packet[5] = Macro::playerId;
	packet[7] = Macro::playerX;
	packet[9] = Macro::playerY;
	unsigned char sendpacket[100] = { "0", };
	Hooks::LoadEncrypt(packet, size, Hooks::encrypted);
	sendpacket[0] = 0xAA;
	sendpacket[1] = 0x00;
	sendpacket[2] = size + 0x1;
	for (int i = 3; i < size + 4; i++)
		sendpacket[i] = Hooks::encrypted[i - 3];
	send(Hooks::Con_Packet_Socket, (const char*)sendpacket, size + 4, 0);
	CloseHandle(GetCurrentThread());
	return 0;
}

void Client::Recv_Packet_Hook_Callback()
{
	// get HWND for winbaram.exe
	ByteBuffer HWND_Packet((LPVOID)0x0019FC8C, 4);
	std::vector<uint8_t> HWND_data = HWND_Packet.ReadBytes(0, 4);
	unsigned char copied[4] = { "0", };
	int x;
	for (int i = 3; i >= 0; i--) {
		copied[3 - i] = (char)HWND_data[3 - i];
	}
	std::memcpy(&x, copied, 4);
	Macro::macroHWND = (HWND)x;

	// get packets
	ByteBuffer Packet((LPVOID)hooks->Ingoing_Packet_Pointer, hooks->Ingoing_Packet_Length);
	std::vector<uint8_t> data = Packet.ReadBytes(0, hooks->Ingoing_Packet_Length);
	std::stringstream result;
	std::copy(data.begin(), data.end(), std::ostream_iterator<int>(result, " "));
	std::string test = result.str();

	int strLen = 0;
	char nameMsg[DEFAULT_BUFLEN];
	int y;

	printf("%zu: ", data.size()); // data.size() Ãâ·Â
	for (int i = 0; i < data.size(); i++) {
		printf("%02x ", data[i]);
	}
	printf("\n");

	if (data[0] == 0x34 && data[1] == 0x00 && data[2] == 0x00 && data[3] == 0x00 && data[4] == 0x04) {
		Macro::playerId = data[data.size() - 29];
		printf("PlayerId :: %d\n", Macro::playerId);
	}
	// 3A 04 C0 FA C1 D6 00 00 00 B9 00 ÀúÁÖ
	// 3a 04 c0 fa c1 d6 00 00 00 b9 00
	else if (data[0] == 0x3A && data[1] == 0x04 && data[2] == 0xC0 && data[3] == 0xFA && data[4] == 0xC1 && data[9] == 0xB9) {
		// printf("PlayerId :: %d\n", Macro::playerId);
		Sleep(300);
		CreateThread(NULL, 0, curse, NULL, 0, NULL);
		Sleep(200);
		CreateThread(NULL, 0, curse, NULL, 0, NULL);
	}

	// 3a 06 c8 a5 b8 b6 bc fa 00 00 01 a9 00
	else if (data[0] == 0x3A && data[1] == 0x06 && data[2] == 0xC8 && data[3] == 0xA5 && data[4] == 0xB8 && data[11] == 0xA9) {
		// printf("PlayerId :: %d\n", Macro::playerId);
		Sleep(300);
		CreateThread(NULL, 0, necromancy, NULL, 0, NULL);
		Sleep(200);
		CreateThread(NULL, 0, curse, NULL, 0, NULL);
	}	
}

