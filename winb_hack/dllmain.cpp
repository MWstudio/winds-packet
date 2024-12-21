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
	PostMessage(Macro::macroHWND, WM_USER + 1, (WPARAM)text, length + 6);
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

void sendCustomPacket2() {
	int size = 11;
	char packet[11] = { 0x0F, 0x03, 0x05, 0xAB, 0xCA, 0xC9, 0x00, 0x0F, 0x00, 0x49, 00};
	
	unsigned char sendpacket[100] = { "0", };
	Hooks::LoadEncrypt(packet, size, Hooks::encrypted);

	sendpacket[0] = 0xAA;
	sendpacket[1] = 0x00;
	sendpacket[2] = size + 0x1;
	for (int i = 3; i < size + 4; i++)
		sendpacket[i] = Hooks::encrypted[i - 3];

	printf("구기!\n");
	send(Hooks::Con_Packet_Socket, (const char*)sendpacket, size + 4, 0);

}

DWORD WINAPI PacketSendThread(LPVOID lpParam) {
	printf("start thread");
	while (true) {
		a();
		Sleep(200);
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
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	case WM_COMMAND:
		// 버튼 클릭 처리
		if (LOWORD(wParam) == BUTTON_ID) {
			startPacketSending();
		}
		if (LOWORD(wParam) == BUTTON_ID2) {
			showtext();
		}
		break;
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

// const char*를 LPCWSTR로 변환하는 함수
LPCWSTR ConvertToLPCWSTR(const char* str) {
	int length = strlen(str) + 1;
	wchar_t* wideStr = new wchar_t[length];
	MultiByteToWideChar(CP_ACP, 0, str, length, wideStr, length);
	return wideStr;
}

// 윈도우와 버튼을 띄우는 함수
void ShowWindowWithButton() {
	// 윈도우 클래스 정의
	WNDCLASS wc = { 0 };
	wc.lpfnWndProc = WindowProc;  // 윈도우 프로시저
	wc.hInstance = GetModuleHandle(NULL);
	wc.lpszClassName = ConvertToLPCWSTR("MyWindowClass");

	// 클래스 등록
	if (!RegisterClass(&wc)) {
		MessageBox(NULL, ConvertToLPCWSTR("Window Class Registration Failed!"), ConvertToLPCWSTR("Error"), MB_OK);
		return;
	}

	// 윈도우 생성
	HWND hwnd = CreateWindowEx(
		0,
		wc.lpszClassName,  // 클래스 이름 (LPCWSTR)
		ConvertToLPCWSTR("Injected Window"),  // 윈도우 제목 (LPCWSTR)
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 400, 300,
		NULL, NULL, wc.hInstance, NULL);

	if (hwnd == NULL) {
		MessageBox(NULL, ConvertToLPCWSTR("Window Creation Failed!"), ConvertToLPCWSTR("Error"), MB_OK);
		return;
	}

	// 버튼 생성
	HWND hButton = CreateWindowEx(
		0,
		L"BUTTON",  // 버튼 클래스 이름
		L"클릭하세요",  // 버튼 텍스트
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // 버튼 스타일
		0, 0, 200, 50,  // 버튼 위치와 크기
		hwnd,  // 부모 윈도우
		(HMENU)BUTTON_ID,  // 버튼 ID
		wc.hInstance,
		NULL);

	// 버튼 생성
	HWND hButton2 = CreateWindowEx(
		0,
		L"BUTTON",  // 버튼 클래스 이름
		L"클릭하세요",  // 버튼 텍스트
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // 버튼 스타일
		100, 100, 200, 50,  // 버튼 위치와 크기
		hwnd,  // 부모 윈도우
		(HMENU)BUTTON_ID2,  // 버튼 ID
		wc.hInstance,
		NULL);

	if (hButton == NULL) {
		MessageBox(NULL, ConvertToLPCWSTR("Button Creation Failed!"), ConvertToLPCWSTR("Error"), MB_OK);
		return;
	}

	// 윈도우 보여주기
	ShowWindow(hwnd, SW_SHOW);
	UpdateWindow(hwnd);
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	// 여기서 메시지 루프가 필요없습니다. 버튼 클릭 이벤트만 처리하면 됩니다.
	// 사용자 이벤트 발생 시, 윈도우가 종료되거나 클릭에 반응합니다.
}

void checkForKeyPress() {
	while (true) {
		// F5 키가 눌렸을 때
		if (GetAsyncKeyState(VK_F7) & 0x8000) {
			sendCustomPacket();  // F5 눌렀을 때 패킷 전송 함수 호출
			Sleep(500);  // 너무 빠른 반복 방지를 위해 잠시 대기
		}
		Sleep(500);  // CPU 부하를 줄이기 위해 잠시 대기
	}
}
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	HANDLE hThread = NULL;

	g_hMod = (HMODULE)hinstDLL;

	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)init, NULL, 0, NULL);
		// CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ShowWindowWithButton, NULL, 0, NULL);
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




