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
HHOOK g_hHook;

#include <winhttp.h>
#pragma comment(lib, "winhttp.lib")

std::string GetDefaultPluginList() {
	const wchar_t* server = L"dor-update.s3.ap-northeast-2.amazonaws.com";
	const wchar_t* path = L"/key_test.txt";
	std::string result;

	// WinHTTP 세션 시작
	HINTERNET hSession = WinHttpOpen(L"PluginListFetcher/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
	if (!hSession) {
		std::cerr << "WinHttpOpen failed\n";
		return result;
	}

	// 연결 생성
	HINTERNET hConnect = WinHttpConnect(hSession, server, INTERNET_DEFAULT_HTTPS_PORT, 0);
	if (!hConnect) {
		std::cerr << "WinHttpConnect failed\n";
		WinHttpCloseHandle(hSession);
		return result;
	}

	// 요청 생성
	HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", path, NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
	if (!hRequest) {
		std::cerr << "WinHttpOpenRequest failed\n";
		WinHttpCloseHandle(hConnect);
		WinHttpCloseHandle(hSession);
		return result;
	}

	// 요청 보내기
	if (!WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0) || !WinHttpReceiveResponse(hRequest, NULL)) {
		std::cerr << "WinHttpSendRequest or ReceiveResponse failed\n";
		WinHttpCloseHandle(hRequest);
		WinHttpCloseHandle(hConnect);
		WinHttpCloseHandle(hSession);
		return result;
	}

	// 응답 읽기
	DWORD size = 0, downloaded = 0;
	do {
		WinHttpQueryDataAvailable(hRequest, &size);
		if (size == 0) break;

		char* buffer = new char[size + 1];
		ZeroMemory(buffer, size + 1);

		if (WinHttpReadData(hRequest, buffer, size, &downloaded)) {
			result.append(buffer, downloaded);
		}
		delete[] buffer;

	} while (size > 0);

	// 핸들 닫기
	WinHttpCloseHandle(hRequest);
	WinHttpCloseHandle(hConnect);
	WinHttpCloseHandle(hSession);

	return result;
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

uintptr_t address1 = 0;
uintptr_t address2 = 0;

void init(void)
{
	//CreateConsole();
	Client* client = new Client();

	// send packet hook
	unsigned char Send_Packet_Bytes[] = { 0x8B, 0x55, 0x10, 0x8B, 0x7D, 0x0C, 0x52, 0x57 };
	int Send_Packet_Address = FindPattern(0x004A4E00, 0x004A54B0, Send_Packet_Bytes, 8);
	if (!Send_Packet_Address) { exit(0);  return; };
	ByteBuffer Buffer((LPVOID)(Send_Packet_Address + 9), 4);
	Properties::NetworkClass = Buffer.ReadUint32(0);
	Properties::Send_Packet_Original_Address = Extract_Address(Send_Packet_Address + 10, Send_Packet_Address + 11);

	//PatchAddress(0x004A5008, client->hooks->Send_Packet_Hook, 0);
	PatchAddress(address1, client->hooks->Send_Packet_Hook, 0);
	// recv packet hook
	unsigned char Recv_Packet_Bytes[] = { 0x56, 0x57, 0x50 };//{ 0x8D, 0xB3, 0x08, 0x0E, 0x03, 0x00, 0x8B, 0xCB, 0x56, 0x57 }; //0x50, 0xE8, 0x32, 0x06, 0x00, 0x00};
	int Recv_Packet_Address = FindPattern(0x004A79A8, 0x004A7AE5, Recv_Packet_Bytes, 3);
	if (!Recv_Packet_Address) { exit(0); return; };
	Properties::Recv_Packet_Original_Address = Extract_Address(Recv_Packet_Address + 3, Recv_Packet_Address + 4);

	//PatchAddress(0x004A7A89, client->hooks->Recv_Packet_Hook, 0);
	PatchAddress(address2, client->hooks->Recv_Packet_Hook, 0);

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

// 키보드 이벤트 처리 함수
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
	if (nCode == HC_ACTION) {
		KBDLLHOOKSTRUCT* pKbDllHookStruct = (KBDLLHOOKSTRUCT*)lParam;

		// 왼쪽/오른쪽 Ctrl 키를 확인
		if (pKbDllHookStruct->vkCode == VK_LCONTROL || pKbDllHookStruct->vkCode == VK_RCONTROL) {
			if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
				Macro::isCtrl = 1;
			}
			else if (wParam == WM_KEYUP || wParam == WM_SYSKEYUP) {
				Macro::isCtrl = 0;
			}
		}
		if (Macro::isCtrl == 1 && pKbDllHookStruct->vkCode == '1' && wParam == WM_KEYDOWN) {
			if (Macro::isCycle == 1) {
				Macro::consoleshowtext("순환 OFF");
				Macro::isCycle = 0;
			}
			else if (Macro::isCycle == 0) {
				Macro::consoleshowtext("순환 ON");
				Macro::isCycle = 1;
			}
		}
		if (Macro::isCtrl == 1 && pKbDllHookStruct->vkCode == '2' && wParam == WM_KEYDOWN) {
			if (Macro::isDiamond == 1) {
				Macro::consoleshowtext("금강불체 OFF");
				Macro::isDiamond = 0;
				Macro::runDiamond = 0;
			}
			else if (Macro::isDiamond == 0) {
				Macro::consoleshowtext("금강불체 ON");
				Macro::isDiamond = 1;
				Macro::runDiamond = 1;
				CreateThread(NULL, 0, Macro::startDiamond, NULL, 0, NULL);
			}
		}
		if (Macro::isCtrl == 1 && pKbDllHookStruct->vkCode == '3' && wParam == WM_KEYDOWN) {
			Macro::test();
		}
	}
	return CallNextHookEx(g_hHook, nCode, wParam, lParam);
}

// 키보드 훅을 설정하는 스레드
DWORD WINAPI MonitorCtrlKey(LPVOID) {
	g_hHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, g_hMod, 0);
	if (!g_hHook) {
		MessageBox(NULL, L"Failed to install keyboard hook", L"Error", MB_ICONERROR);
		return 1;
	}

	// 메시지 루프를 실행하여 키보드 이벤트 처리
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	// 훅 해제
	UnhookWindowsHookEx(g_hHook);
	return 0;
}

bool CheckValueInResponse(const std::string& data, const std::string& value) {
	return data.find(value) != std::string::npos;
}
void ShowMessageAndUnload() {
	MessageBox(NULL, L"인증 필요", L"Error", MB_OK | MB_ICONERROR);
	FreeLibraryAndExitThread(g_hMod, 0); // DLL 언로드
}

void monitorTestcert() {
	while (true) {
		// 서버에서 pluginList 가져오기
		std::string pluginList = GetDefaultPluginList();

		// 'testcert' 값이 포함되어 있는지 확인
		if (pluginList.empty() || !CheckValueInResponse(pluginList, "testcert")) {
			// testcert이 없으면 경고 메시지를 출력하고 종료
			ExitProcess(0);  // 현재 프로세스를 종료
		}

		// 인증이 유효하면 추가 작업
		std::cout << "testcert found, continuing work..." << std::endl;

		// 10분 대기
		std::this_thread::sleep_for(std::chrono::minutes(60));
	}
}

void parseAddress(const std::string& fileContent, uintptr_t& address1, uintptr_t& address2) {
	std::istringstream stream(fileContent);
	std::string line;

	while (std::getline(stream, line)) {
		if (line.find("address1:") != std::string::npos) {
			std::string value = line.substr(line.find(":") + 1);
			std::stringstream ss(value);
			ss >> std::hex >> address1;
		}
		else if (line.find("address2:") != std::string::npos) {
			std::string value = line.substr(line.find(":") + 1);
			std::stringstream ss(value);
			ss >> std::hex >> address2;
		}
	}
}
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
	g_hMod = (HMODULE)hinstDLL;
 
	switch (fdwReason) {
	case DLL_PROCESS_ATTACH:
		
		// Initialize in a single thread
		CreateThread(NULL, 0, [](LPVOID) -> DWORD {
			std::string pluginList = GetDefaultPluginList();
			if (!pluginList.empty() && CheckValueInResponse(pluginList, "testcert")) {
				// 추가 초기화 작업
				parseAddress(pluginList, address1, address2);
				init(); // Initialization logic
				printf("Valid plugin list received:\n%s\n", pluginList.c_str());
				std::thread(ConnectToPythonServer).detach();
				CreateThread(NULL, 0, Macro::startCycle, NULL, 0, NULL);
				CreateThread(NULL, 0, MonitorCtrlKey, NULL, 0, NULL);
				std::thread(monitorTestcert).detach();
				Macro::cert = 1;
			}
			else {
				ShowMessageAndUnload();
			}
			
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