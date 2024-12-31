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
ThreadPool pool(16); // 스레드풀 생성
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

int j = 0;
DWORD WINAPI trans_loop(LPVOID lpParam) {
	if (j == 1) return 0;
	for (int i = 0; i < 5; i++) {
		j = 1;
		Macro::transparency();
		Sleep(200);
	}
	j = 0;
	return 0;
}

void Client::Send_Packet_Hook_Callback()
{
	// if you want to see send packets, delete the remark below

	ByteBuffer Packet((LPVOID)hooks->Outgoing_Packet_Pointer, hooks->Outgoing_Packet_Length);
	std::vector<uint8_t> data = Packet.ReadBytes(0, hooks->Outgoing_Packet_Length);
	/*std::stringstream result;
	std::copy(data.begin(), data.end(), std::ostream_iterator<int>(result, " "));
	std::string test = result.str();*/

	printf("Client is sending... : \n");
	printf("%zu: ", data.size());
	for (int i = 0; i < data.size(); i++) {
		printf("%02X ", data[i]);
	}
	printf("\n");
}


DWORD WINAPI checkPacket(LPVOID lpParam) {
	std::vector<uint8_t>* data = (std::vector<uint8_t>*)lpParam;
	size_t dataSize = data->size();
	if ((*data)[0] == 0x5 && dataSize == 0xd) {
		Macro::playerId = (*data)[4];
		printf("playerid: %d \n", Macro::playerId);
	}
	else if ((*data)[0] == 0x1d && (*data)[7] == 0x00 && (*data)[4] == Macro::playerId && (*data).size() == 25) {
		Macro::transparency();
		Sleep(300);
		Macro::shadowlessStep();
	}
	delete data;
	return 0;
};

void Client::Recv_Packet_Hook_Callback()
{
	// get HWND for winbaram.exe
	ByteBuffer HWND_Packet((LPVOID)0x0055DC3C, 4);
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
	std::vector<uint8_t> dataCopy = data;
	//std::stringstream result;
	//std::copy(data.begin(), data.end(), std::ostream_iterator<int>(result, " "));
	//std::string test = result.str();

	//int strLen = 0;
	//char nameMsg[DEFAULT_BUFLEN];
	//int y;
	
	if (data[0] == 0x08 && data[4] == 0x00 && data[5] == 0x00) {
		pool.enqueue([]() { trans_loop(nullptr); });
	}
	else if (data[0] == 0x1d && data[7] == 0x00 && Macro::isTransparency == 1 && data[4] == Macro::playerId)
		pool.enqueue(checkPacket, new std::vector<uint8_t>(dataCopy));

	else if (data[0] == 0x5 && hooks->Ingoing_Packet_Length == 0xd) {
		pool.enqueue(checkPacket, new std::vector<uint8_t>(dataCopy));
	}
	//투명 무시
	else if (data[0] == 0x1d && data[7] == 0x02 && data[4] != Macro::playerId) {
		data[7] = 0x00;
		std::memcpy((LPVOID)hooks->Ingoing_Packet_Pointer, data.data(), data.size());
		printf("투명 무시1\n");
		
	}
	else if (data[0] == 0x33 && data[12] == 0x02) {
		data[12] = 0x00;
		std::memcpy((LPVOID)hooks->Ingoing_Packet_Pointer, data.data(), data.size());
		printf("투명 무시2\n");
	}
	printf("client is receiving... : \n");
	printf("%zu: ", data.size());
	for (int i = 0; i < data.size(); i++) {
		printf("%02x ", data[i]);
	}
	printf("\n");

	//if (data[0] == 0x34 && data[1] == 0x00 && data[2] == 0x00 && data[3] == 0x00 && data[4] == 0x04) {
	//	Macro::playerId = data[data.size() - 29];
	//	printf("PlayerId :: %d\n", Macro::playerId);
	//}
	//// 3A 04 C0 FA C1 D6 00 00 00 B9 00     
	//// 3a 04 c0 fa c1 d6 00 00 00 b9 00
	//else if (data[0] == 0x3A && data[1] == 0x04 && data[2] == 0xC0 && data[3] == 0xFA && data[4] == 0xC1 && data[9] == 0xB9) {
	//	printf("PlayerId :: %d\n", Macro::playerId);
	//	Sleep(300);
	//	CreateThread(NULL, 0, curse, NULL, 0, NULL);
	//	Sleep(200);
	//	CreateThread(NULL, 0, curse, NULL, 0, NULL);
	//}

	//// 3a 06 c8 a5 b8 b6 bc fa 00 00 01 a9 00
	//else if (data[0] == 0x3A && data[1] == 0x06 && data[2] == 0xC8 && data[3] == 0xA5 && data[4] == 0xB8 && data[11] == 0xA9) {
	//	printf("PlayerId :: %d\n", Macro::playerId);
	//	Sleep(300);
	//	CreateThread(NULL, 0, necromancy, NULL, 0, NULL);
	//	Sleep(200);
	//	CreateThread(NULL, 0, curse, NULL, 0, NULL);
	//}	
}