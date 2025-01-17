// for removing errors
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <stdio.h>
#include "stdafx.h"
#include "ASM.h"
#include "bytebuffer.h"
#include "Client.h"
#include "Hooks.h"
#include "Macro.h"

#pragma comment(lib, "ws2_32.lib")

#define DEFAULT_BUFLEN 2000
#define SIZE 6

// need for hooking connect function
typedef int (WINAPI *pConnect)(SOCKET, const struct sockaddr*, int);
int WINAPI MyConnect(SOCKET, const struct sockaddr*, int);
void BeginRedirect(LPVOID newFunction);

pConnect pOrigMBAddress = NULL;
BYTE oldBytes[SIZE] = { 0 };
BYTE JMP[SIZE] = { 0 };
DWORD oldProtect, myProtect = PAGE_EXECUTE_READWRITE;
HMODULE g_hMod = NULL;

// create console in dll
void CreateConsole()
{
	FILE *acStreamOut;
	FILE *acStreamIn;
	
	// Allocate a console
	AllocConsole();

	// enable console output
	SetConsoleTitleA("testing...");
	freopen_s(&acStreamOut, "CONOUT$", "w", stdout);
	freopen_s(&acStreamIn, "CONIN$", "r", stdin);
	freopen_s(&acStreamIn, "CONIN$", "r", stderr);

	// Disable Quick Edit Mode
	//HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
	//DWORD mode;
	//GetConsoleMode(hInput, &mode);
	//mode &= ~ENABLE_QUICK_EDIT_MODE; // Disable Quick Edit Mode
	//mode |= ENABLE_EXTENDED_FLAGS;  // Set extended flags to apply changes
	//SetConsoleMode(hInput, mode);
}



void init(void)
{
	CreateConsole();
	Client* client = new Client();

	// send packet hook
	unsigned char Send_Packet_Bytes[] = { 0x8B, 0x55, 0x10, 0x8B, 0x7D, 0x0C, 0x52, 0x57 };
	int Send_Packet_Address = FindPattern(0x004A4E00, 0x004A54B0, Send_Packet_Bytes, 8);
	if (!Send_Packet_Address) { exit(0);  return; };
	ByteBuffer Buffer((LPVOID)(Send_Packet_Address + 9), 4);
	Properties::NetworkClass = Buffer.ReadUint32(0);
	Properties::Send_Packet_Original_Address = Extract_Address(Send_Packet_Address + 10, Send_Packet_Address + 11);

	PatchAddress(0x004A5008, client->hooks->Send_Packet_Hook, 0);

	// recv packet hook
	unsigned char Recv_Packet_Bytes[] = { 0x56, 0x57, 0x50 };//{ 0x8D, 0xB3, 0x08, 0x0E, 0x03, 0x00, 0x8B, 0xCB, 0x56, 0x57 }; //0x50, 0xE8, 0x32, 0x06, 0x00, 0x00};
	int Recv_Packet_Address = FindPattern(0x004A79A8, 0x004A7AE5, Recv_Packet_Bytes, 3);
	if (!Recv_Packet_Address) { exit(0); return; };
	Properties::Recv_Packet_Original_Address = Extract_Address(Recv_Packet_Address + 3, Recv_Packet_Address + 4);

	PatchAddress(0x004A7A89, client->hooks->Recv_Packet_Hook, 0);

	/*
	// connect hook version 1
	unsigned char Con_Packet_Bytes[] = { 0x6A, 0x10, 0x52, 0x50, 0xFF, 0x15 };
	int Con_Packet_Address = FindPattern(0x004A5BD8, 0x004A5C79, Con_Packet_Bytes, 6);
	if (!Con_Packet_Address) { exit(0); return FALSE; };
	printf("---- Con_Packet_Address: %x\n", Con_Packet_Address);
	//Properties::Con_Packet_Original_Address = Extract_Address(Con_Packet_Address + 4, Con_Packet_Address + 5);

	// get 4 bytes from the address 0x0052955C to integer
	ByteBuffer Packet((LPVOID)0x0052955C, 4);
	std::vector<uint8_t> data = Packet.ReadBytes(0, 4);
	unsigned char copied[4] = { "0", };
	int x;
	for (int i = 3; i >= 0; i--) {
	copied[3 - i] = (char)data[3 - i];
	}
	std::memcpy(&x, copied, 4);

	Properties::Con_Packet_Original_Address = x;
	printf("---- Con_Packet_Original_Address: %x\n", Properties::Con_Packet_Original_Address);

	PatchAddress(0x004A5C05, client->hooks->Con_Packet_Hook, 0);
	*/

	// connect hook version 2
	pOrigMBAddress = (pConnect)GetProcAddress(GetModuleHandle(L"WS2_32.dll"), "connect");
	if (pOrigMBAddress != NULL)
		BeginRedirect(MyConnect);
	else
		printf("NULL\n");

	printf("starting...\n");

}

// 전역 변수
HWND hEditControl;  // 입력 받을 Edit Control

// 패킷 전송 함수
void sendCustomPacket() {
	int size = 3;
	char packet[3] = { 0x0F, 0x02, 0x00 };	

	unsigned char sendpacket[100] = { "0", };
	Hooks::LoadEncrypt(packet, size, Hooks::encrypted);
	sendpacket[0] = 0xAA;
	sendpacket[1] = 0x00;
	sendpacket[2] = size + 0x1;
	for (int i = 3; i < size + 4; i++)
		sendpacket[i] = Hooks::encrypted[i - 3];

	send(Hooks::Con_Packet_Socket, (const char*)sendpacket, size + 4, 0);

}

void attack() { // 13 00 기본공격
	int size = 2;
	char packet[2] = { 0x13, 0x00 };
	unsigned char sendpacket[100] = { "0", };
	Hooks::LoadEncrypt(packet, size, Hooks::encrypted);
	sendpacket[0] = 0xAA;
	sendpacket[1] = 0x00;
	sendpacket[2] = size + 0x1;
	for (int i = 3; i < size + 4; i++)
		sendpacket[i] = Hooks::encrypted[i - 3];

	send(Hooks::Con_Packet_Socket, (const char*)sendpacket, size + 4, 0);
}

DWORD WINAPI PacketSendThread(LPVOID lpParam) {
	printf("start thread");
	while (true) {
		attack();
		Sleep(300);
	}
	return 0;
}

void startPacketSending() {
	// 새 스레드 생성하여 비동기적으로 패킷 전송
	CreateThread(NULL, 0, PacketSendThread, NULL, 0, NULL);
}

// 윈도우 프로시저 함수
#define BUTTON_ID 1  // 버튼 ID 정의
#define BUTTON_ID2 2  // 버튼 ID 정의
HWND hwndButton;  // 버튼 1
HWND hwndButton2; // 버튼 2


// const char*를 LPCWSTR로 변환하는 함수
LPCWSTR ConvertToLPCWSTR(const char* str) {
	int length = strlen(str) + 1;
	wchar_t* wideStr = new wchar_t[length];
	MultiByteToWideChar(CP_ACP, 0, str, length, wideStr, length);
	return wideStr;
}

// Global variables for button handle and window class
HWND hButton;
HINSTANCE hInstance;


#include <winsock2.h>
#include <windows.h>
#include <iostream>
#include <thread>

#pragma comment(lib, "ws2_32.lib")

const char* SERVER_IP = "127.0.0.1";
const int SERVER_PORT = 9698;
SOCKET clientSocket;


// 소켓 통신
void CommunicateWithServer() {
	char buffer[1024];
	while (true) {
		memset(buffer, 0, sizeof(buffer));
		int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
		if (bytesReceived > 0) {
			buffer[bytesReceived] = '\0';
			std::string message(buffer);

			std::cout << "Received from Python: " << message << std::endl;

			// 메시지 처리
			size_t delimPos = message.find(':');
			if (delimPos != std::string::npos) {
				std::string skillName = message.substr(0, delimPos);
				std::string keyStr = message.substr(delimPos + 1);

				try {
					unsigned short keyValue = static_cast<unsigned short>(std::stoul(keyStr));
					Macro::updateSkillKey(skillName, keyValue);
				}
				catch (const std::exception& e) {
					printf("Failed to parse key for %s: %s\n", skillName.c_str(), e.what());
				}
			}

			// 특정 메시지 처리
			if (message == "start cycle") {
				Macro::isCycle = 1;
			}
			if (message == "stop cycle") {
				Macro::isCycle = 0;
	/*			std::string response = "call cycle successfully.";
				send(clientSocket, response.c_str(), response.size(), 0);*/
			}
			if (message == "start diamond") {
				Macro::isDiamond = 1;
			}
			if (message == "stop diamond") {
				Macro::isDiamond = 0;
			}
		}
		else if (bytesReceived == 0) {
			printf("Connection closed by client.\n");
			break;
		}
		else {
			printf("recv failed. Error: %d\n", WSAGetLastError());
			break;
		}
	}
	closesocket(clientSocket);
}

// Python 서버에 연결
void ConnectToPythonServer() {
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (clientSocket == INVALID_SOCKET) {
		std::cerr << "Socket creation failed. Error: " << WSAGetLastError() << std::endl;
		return;
	}

	sockaddr_in serverAddr = {};
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	inet_pton(AF_INET, SERVER_IP, &serverAddr.sin_addr);

	std::cout << "Connecting to Python server..." << std::endl;
	if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
		std::cerr << "Connection failed. Error: " << WSAGetLastError() << std::endl;
		closesocket(clientSocket);
		return;
	}

	std::cout << "Connected to Python server." << std::endl;
	CommunicateWithServer();
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
	g_hMod = (HMODULE)hinstDLL;

	switch (fdwReason) {
	case DLL_PROCESS_ATTACH:
		// Initialize in a single thread
		CreateThread(NULL, 0, [](LPVOID) -> DWORD {
			init(); // Initialization logic
			std::thread(ConnectToPythonServer).detach();
			CreateThread(NULL, 0, Macro::startCycle, NULL, 0, NULL);
			CreateThread(NULL, 0, Macro::startDiamond, NULL, 0, NULL);
			return 0;
			}, NULL, 0, NULL);
		break;
	case DLL_PROCESS_DETACH:
		closesocket(clientSocket);
		WSACleanup();
		break;
	}

	return TRUE;
}


void BeginRedirect(LPVOID newFunction)
{
	BYTE tempJMP[SIZE] = { 0xE9, 0x90, 0x90, 0x90, 0x90, 0xC3 };
	memcpy(JMP, tempJMP, SIZE);
	DWORD JMPSize = ((DWORD)newFunction - (DWORD)pOrigMBAddress - 5);
	VirtualProtect((LPVOID)pOrigMBAddress, SIZE,
		PAGE_EXECUTE_READWRITE, &oldProtect);
	memcpy(oldBytes, pOrigMBAddress, SIZE);
	memcpy(&JMP[1], &JMPSize, 4);
	memcpy(pOrigMBAddress, JMP, SIZE);
	VirtualProtect((LPVOID)pOrigMBAddress, SIZE, oldProtect, NULL);
}

int  WINAPI MyConnect(SOCKET sock, const struct sockaddr *addr, int length)
{
	int strLen = 0;
	char nameMsg[DEFAULT_BUFLEN];

	VirtualProtect((LPVOID)pOrigMBAddress, SIZE, myProtect, NULL);
	memcpy(pOrigMBAddress, oldBytes, SIZE);

	int retValue = connect(sock, addr, length);
	Hooks::Con_Packet_Socket = sock;

	memcpy(pOrigMBAddress, JMP, SIZE);
	VirtualProtect((LPVOID)pOrigMBAddress, SIZE, oldProtect, NULL);

	return retValue;
}