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

//#pragma comment(lib, "ws2_32.lib")

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

unsigned char* korStrToHex(const char* korStr) {
	size_t charCount = strlen(korStr);
	static unsigned char buffer[BUFSIZ] = { 0, };

	for (size_t i = 0; i < charCount; i++) {
		unsigned char cc = *((unsigned char*)korStr + i);
		buffer[i] = cc;
	}
	return buffer;
}

void showtext() {
	static unsigned char text[BUFSIZ] = { 0, };
	const char* korStr = "ㅇㅇ";
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
	if (IsWindow(Macro::macroHWND)) {
		printf("macroHWND is valid.\n");
	}
	else {
		printf("macroHWND is invalid.\n");
	}
	PostMessage(Macro::macroHWND, WM_USER + 3, (WPARAM)text, length + 6);
}

void say() {
	// 하드코딩된 문자열
	const char* korStr = "ㅇㅇ";

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

void a() { // 0F 06 00 금강불체
	int size = 3;
	char packet[3] = { 0x0F, 0x06, 0x00 };
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

void test() {
	MessageBox(NULL, L"Test function called!", L"Information", MB_OK | MB_ICONINFORMATION);
}

void HandleClient(HANDLE hPipe) {
	char buffer[1024];
	DWORD bytesRead;

	while (true) {
		// 클라이언트로부터 메시지 수신
		BOOL success = ReadFile(hPipe, buffer, sizeof(buffer) - 1, &bytesRead, NULL);
		if (!success || bytesRead == 0) {
			std::cout << "Client disconnected." << std::endl;
			break;
		}

		buffer[bytesRead] = '\0'; // 문자열 끝 추가
		std::string message(buffer);

		std::cout << "Received: " << message << std::endl;

		// 특정 명령어 처리
		if (message == "CALL_TEST") {
			test(); // Python에서 test() 호출 요청
		}
		else {
			// 응답 전송
			std::string response = "ACK: " + message;
			DWORD bytesWritten;
			WriteFile(hPipe, response.c_str(), response.length(), &bytesWritten, NULL);
		}
	}

	// 파이프 닫기
	FlushFileBuffers(hPipe);
	DisconnectNamedPipe(hPipe);
	CloseHandle(hPipe);
}

void PipeServer() {
	const wchar_t* PIPE_NAME = L"\\\\.\\pipe\\TestPipe";

	while (true) {
		// Named Pipe 생성
		HANDLE hPipe = CreateNamedPipe(
			PIPE_NAME,
			PIPE_ACCESS_DUPLEX,
			PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
			PIPE_UNLIMITED_INSTANCES, // 무제한 클라이언트 지원
			1024 * 16,
			1024 * 16,
			0,
			NULL);

		if (hPipe == INVALID_HANDLE_VALUE) {
			std::cerr << "Failed to create pipe. Error: " << GetLastError() << std::endl;
			return;
		}

		std::cout << "Waiting for Python client to connect..." << std::endl;

		// 클라이언트 연결 대기
		if (ConnectNamedPipe(hPipe, NULL) || GetLastError() == ERROR_PIPE_CONNECTED) {
			// 클라이언트 처리 스레드 생성
			std::thread(HandleClient, hPipe).detach();
		}
		else {
			CloseHandle(hPipe);
		}
	}
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
	g_hMod = (HMODULE)hinstDLL;

	switch (fdwReason) {
	case DLL_PROCESS_ATTACH:
		// Initialize in a single thread
		CreateThread(NULL, 0, [](LPVOID) -> DWORD {
			init(); // Initialization logic
			// std::thread(PipeServer).detach();
			return 0;
			}, NULL, 0, NULL);
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