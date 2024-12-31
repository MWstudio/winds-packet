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

void curse(int playerId, int x, int y, int map1, int map2, int map3) {
	// 0F 0D 01 03 2A 22 00 07 00 08 00   
	if (x == 0 && y == 0) {
		return;
	}
	if (playerId == 0) {
		return;

	}
	int size = 11;
	char packet[11] = { 0x0F, 0x0D, 0x01, 0x03, 0x2A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	packet[1] = Macro::exorcismKey;
	packet[2] = map1;
	packet[3] = map2;
	packet[4] = map3;
	packet[5] = playerId;
	packet[7] = x;
	packet[9] = y;
	unsigned char sendpacket[100] = { "0", };
	Hooks::LoadEncrypt(packet, size, Hooks::encrypted);
	sendpacket[0] = 0xAA;
	sendpacket[1] = 0x00;
	sendpacket[2] = size + 0x1;
	for (int i = 3; i < size + 4; i++)
		sendpacket[i] = Hooks::encrypted[i - 3];
	int result = send(Hooks::Con_Packet_Socket, (const char*)sendpacket, size + 4, 0);
	if (result == SOCKET_ERROR) {
		printf("Send failed with error: %d\n", WSAGetLastError());
		return;
	}
	return;
}

void necromancy(int playerId, int x, int y, int map1, int map2, int map3) {
	// 0F 08 01 03 2A 22 00 07 00 08 00   ?  
	// 0F 08 01 03 2B 14 00 09 00 0A 00
	// 0F 08 0D 4A 9E AA 00 06 00 06 00
	if (x == 0 && y == 0) {
		return;
	}
	if (playerId == 0) {
		return;
	}
	int size = 11;
	char packet[11] = { 0x0F, 0x08, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	packet[1] = Macro::divorceKey;
	packet[2] = map1;
	packet[3] = map2;
	packet[4] = map3;
	packet[5] = playerId;
	packet[7] = x;
	packet[9] = y;

	unsigned char sendpacket[100] = { "0", };
	Hooks::LoadEncrypt(packet, size, Hooks::encrypted);
	sendpacket[0] = 0xAA;
	sendpacket[1] = 0x00;
	sendpacket[2] = size + 0x1;
	for (int i = 3; i < size + 4; i++)
		sendpacket[i] = Hooks::encrypted[i - 3];
	int result = send(Hooks::Con_Packet_Socket, (const char*)sendpacket, size + 4, 0);
	if (result == SOCKET_ERROR) {
		printf("Send failed with error: %d\n", WSAGetLastError());
		return;
	}
	return;
}

void armed(int playerId, int x, int y, int map1, int map2, int map3) {
	if (x == 0 && y == 0) {
		return;
	}
	if (playerId == 0) {
		return;
	}
	int size = 11;
	char packet[11] = { 0x0F, 0x08, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	packet[1] = Macro::armedKey;
	packet[2] = map1;
	packet[3] = map2;
	packet[4] = map3;
	packet[5] = playerId;
	packet[7] = x;
	packet[9] = y;

	unsigned char sendpacket[100] = { "0", };
	Hooks::LoadEncrypt(packet, size, Hooks::encrypted);
	sendpacket[0] = 0xAA;
	sendpacket[1] = 0x00;
	sendpacket[2] = size + 0x1;
	for (int i = 3; i < size + 4; i++)
		sendpacket[i] = Hooks::encrypted[i - 3];
	int result = send(Hooks::Con_Packet_Socket, (const char*)sendpacket, size + 4, 0);
	if (result == SOCKET_ERROR) {
		printf("Send failed with error: %d\n", WSAGetLastError());
		return;
	}
	return;
}

void protect(int playerId, int x, int y, int map1, int map2, int map3) {
	if (x == 0 && y == 0) {
		return;
	}
	if (playerId == 0) {
		return;
	}
	int size = 11;
	char packet[11] = { 0x0F, 0x08, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	packet[1] = Macro::protectKey;
	packet[2] = map1;
	packet[3] = map2;
	packet[4] = map3;
	packet[5] = playerId;
	packet[7] = x;
	packet[9] = y;

	unsigned char sendpacket[100] = { "0", };
	Hooks::LoadEncrypt(packet, size, Hooks::encrypted);
	sendpacket[0] = 0xAA;
	sendpacket[1] = 0x00;
	sendpacket[2] = size + 0x1;
	for (int i = 3; i < size + 4; i++)
		sendpacket[i] = Hooks::encrypted[i - 3];
	int result = send(Hooks::Con_Packet_Socket, (const char*)sendpacket, size + 4, 0);
	if (result == SOCKET_ERROR) {
		printf("Send failed with error: %d\n", WSAGetLastError());
		return;
	}
	return;
}
DWORD WINAPI checkPacket(LPVOID lpParam) {
	std::vector<uint8_t>* data = (std::vector<uint8_t>*)lpParam;
	size_t dataSize = data->size();
	// 0c 15 59 99 90 00 12 00 0d 02 a9 00
	if ((*data)[0] == 0x0C &&  (*data)[4] == Macro::selectedPlayerId && dataSize == 12) {
		if ((*data)[9] == 0x00) {
			Macro::selectedPlayerX = (*data)[6];
			Macro::selectedPlayerY = (*data)[8] - 1;
			//printf("%s %d\n", "playerX:", Macro::selectedPlayerX);
			//printf("%s %d\n", "playerY:", Macro::selectedPlayerY);
		}
		if ((*data)[9] == 0x01) {
			Macro::selectedPlayerX = (*data)[6] + 1;
			Macro::selectedPlayerY = (*data)[8];
			//printf("%s %d\n", "playerX:", Macro::selectedPlayerX);
			//printf("%s %d\n", "playerY:", Macro::selectedPlayerY);
		}
		if ((*data)[9] == 0x02) {
			Macro::selectedPlayerX = (*data)[6];
			Macro::selectedPlayerY = (*data)[8] + 1;
			//printf("%s %d\n", "playerX:", Macro::selectedPlayerX);
			//printf("%s %d\n", "playerY:", Macro::selectedPlayerY);
		}
		if ((*data)[9] == 0x03) {
			Macro::selectedPlayerX = (*data)[6] - 1;
			Macro::selectedPlayerY = (*data)[8];
			//printf("%s %d\n", "playerX:", Macro::selectedPlayerX);
			//printf("%s %d\n", "playerY:", Macro::selectedPlayerY);
		}
	}
	else if ((*data)[0] == 0x11 && dataSize == 0x7) {
		Macro::map1 = (*data)[1];
		Macro::map2 = (*data)[2];
		Macro::map3 = (*data)[3];
	}
	//it's me
	else if ((*data)[0] == 0x5 && dataSize == 0xd || (*data)[0] == 0x11 && dataSize == 0x7) {
		Macro::playerId = (*data)[4];
	}

	//select player id
	else if ((*data)[0] == 0x34 && (*data)[(*data).size() - 2] == 0x2e && (*data)[(*data).size() - 3] == 0xdb && (*data)[(*data).size() - 4] == 0xc0) {
		int id = 0;
		for (size_t i = 0; i <= dataSize - 4; ++i) {
			if ((*data)[i] == 0x00 && (*data)[i + 1] == 0x00 &&
				(*data)[i + 2] == 0x00 && (*data)[i + 3] == 0x00) {
				// 가장 마지막 패턴을 찾기 위해 index를 계속 갱신
				id = (i + 6 < dataSize) ? (*data)[i - 3] : 0;
			}
		}
		
		if (Macro::playerId != id && id != 0) {
			Macro::selectedPlayerId = id;
			printf("selectedPlayerId : %d\n", Macro::selectedPlayerId);
		}
	}
	// 29 3a 75 a3 78 0d 00
	// 29 3a 75 a3 78 31 00
	// 29 01 03 2b 49 31 00
	// 
	// ?   29 ?? ?? ?? unidque_id 27 00
	else if ((*data)[0] == 0x29 && (*data)[5] == 0x27) {
		if ((*data)[4] == Macro::selectedPlayerId) {
			Macro::necromancyReceivedSelected = 1;  // Set the flag to true
			for (int i = 0; i<10; i++ ) {
				Sleep(300);
				necromancy(Macro::selectedPlayerId, Macro::selectedPlayerX, Macro::selectedPlayerY, Macro::map1, Macro::map2, Macro::map3);
				if (Macro::necromancyReceivedSelected == 0) {
					break;
				}
			}
			Macro::necromancyReceivedSelected = 0;
		}
		else if ((*data)[4] == Macro::playerId) {
			Macro::necromancyReceivedMe = 1;  // Set the flag to true
			for (int i = 0; i<10; i++ ) {
				Sleep(300);
				necromancy(Macro::playerId, Macro::playerX, Macro::playerY, Macro::map1, Macro::map2, Macro::map3);
				if (Macro::necromancyReceivedMe == 0) {
					break;
				}
			}
			Macro::necromancyReceivedMe = 0;
		}
	}

	else if ((*data)[0] == 0x29 && (*data)[5] == 0x31) {
		if ((*data)[4] == Macro::selectedPlayerId) {
			Macro::necromancyReceivedSelected = 0;  // Set the flag to true
		}
		else if ((*data)[4] == Macro::playerId) {
			Macro::necromancyReceivedMe = 0;  // Set the flag to true
		}
	}

	// 29 01 03 ?? unidque_id 0d 00
	else if ((*data)[0] == 0x29 && (*data)[5] == 0x0D) {
		if ((*data)[4] == Macro::selectedPlayerId) {
			Macro::curseReceivedSelected = 1;  // Set the flag to true
			for (int i = 0; i<10; i++ ) {
				Sleep(300);
				curse(Macro::selectedPlayerId, Macro::selectedPlayerX, Macro::selectedPlayerY, Macro::map1, Macro::map2, Macro::map3);
				if (Macro::curseReceivedSelected == 0) {
					break;
				}
			}
			Macro::curseReceivedSelected = 0;
		}
		else if ((*data)[4] == Macro::playerId) {
			Macro::curseReceivedMe = 1;  // Set the flag to true
			for (int i = 0; i<10; i++ ) {
				Sleep(300);
				curse(Macro::playerId, Macro::playerX, Macro::playerY, Macro::map1, Macro::map2, Macro::map3);
				if (Macro::curseReceivedMe == 0) {
					break;
				}
			}
			Macro::curseReceivedMe = 0;
		}
	}

	else if ((*data)[0] == 0x29  && (*data)[5] == 0x16) {
		if ((*data)[4] == Macro::selectedPlayerId) {
			Macro::curseReceivedSelected = 0;  // Set the flag to true
		}
		else if ((*data)[4] == Macro::playerId) {
			Macro::curseReceivedMe = 0;  // Set the flag to true
		}
	}

	else if ((*data)[0] == 0x29 && (*data)[5] == 0x0A) {
		if ((*data)[4] == Macro::selectedPlayerId) {
			Sleep(100);
			protect(Macro::selectedPlayerId, Macro::playerX, Macro::playerY, Macro::map1, Macro::map2, Macro::map3);
			Sleep(400);
			armed(Macro::selectedPlayerId, Macro::playerX, Macro::playerY, Macro::map1, Macro::map2, Macro::map3);
		}
		else if ((*data)[4] == Macro::playerId) {
			Sleep(100);
			protect(Macro::playerId, Macro::playerX, Macro::playerY, Macro::map1, Macro::map2, Macro::map3);
			Sleep(400);
			armed(Macro::playerId, Macro::playerX, Macro::playerY, Macro::map1, Macro::map2, Macro::map3);
		}
	}
	else if ((*data)[0] == 0x0A && (*data)[1] == 0x03 && dataSize == 19) {
		Macro::diamondBody();
	}

	delete data;
	return 0;
}

DWORD WINAPI checkSendPacket(LPVOID lpParam) {
	std::vector<uint8_t>* data = (std::vector<uint8_t>*)lpParam;
	size_t dataSize = data->size();
	if ((*data)[0] == 0x32 && dataSize == 8) {
		if ((*data)[1] == 0x00) {
			Macro::playerX = (*data)[4];
			Macro::playerY = (*data)[6] - 1;
			//printf("%s %d\n", "playerX:", Macro::playerX);
			//printf("%s %d\n", "playerY:", Macro::playerY);
		}
		if ((*data)[1] == 0x01) {
			Macro::playerX = (*data)[4] + 1;
			Macro::playerY = (*data)[6];
			//printf("%s %d\n", "playerX:", Macro::playerX);
			//printf("%s %d\n", "playerY:", Macro::playerY);
		}
		if ((*data)[1] == 0x02) {
			Macro::playerX = (*data)[4];
			Macro::playerY = (*data)[6] + 1;
			//printf("%s %d\n", "playerX:", Macro::playerX);
			//printf("%s %d\n", "playerY:", Macro::playerY);
		}
		if ((*data)[1] == 0x03) {
			Macro::playerX = (*data)[4] - 1;
			Macro::playerY = (*data)[6];
			//printf("%s %d\n", "playerX:", Macro::playerX);
			//printf("%s %d\n", "playerY:", Macro::playerY);
		}
	}
	else if ((*data)[0] == 0x06 && dataSize == 16) {
		if ((*data)[1] == 0x00) {
			Macro::playerX = (*data)[4];
			Macro::playerY = (*data)[6] - 1;
			//printf("%s %d\n", "playerX:", Macro::playerX);
			//printf("%s %d\n", "playerY:", Macro::playerY);
		}
		if ((*data)[1] == 0x01) {
			Macro::playerX = (*data)[4] + 1;
			Macro::playerY = (*data)[6];
			//printf("%s %d\n", "playerX:", Macro::playerX);
			//printf("%s %d\n", "playerY:", Macro::playerY);
		}
		if ((*data)[1] == 0x02) {
			Macro::playerX = (*data)[4];
			Macro::playerY = (*data)[6] + 1;
			//printf("%s %d\n", "playerX:", Macro::playerX);
			//printf("%s %d\n", "playerY:", Macro::playerY);
		}
		if ((*data)[1] == 0x03) {
			Macro::playerX = (*data)[4] - 1;
			Macro::playerY = (*data)[6];
			//printf("%s %d\n", "playerX:", Macro::playerX);
			//printf("%s %d\n", "playerY:", Macro::playerY);
		}
	}
	else if ((*data)[0] == 0x06 && dataSize == 20) {
		if ((*data)[1] == 0x00) {
			Macro::playerX = (*data)[4];
			Macro::playerY = (*data)[6] - 1;
			//printf("%s %d\n", "playerX:", Macro::playerX);
			//printf("%s %d\n", "playerY:", Macro::playerY);
		}
		if ((*data)[1] == 0x01) {
			Macro::playerX = (*data)[4] + 1;
			Macro::playerY = (*data)[6];
			//printf("%s %d\n", "playerX:", Macro::playerX);
			//printf("%s %d\n", "playerY:", Macro::playerY);
		}
		if ((*data)[1] == 0x02) {
			Macro::playerX = (*data)[4];
			Macro::playerY = (*data)[6] + 1;
			//printf("%s %d\n", "playerX:", Macro::playerX);
			//printf("%s %d\n", "playerY:", Macro::playerY);
		}
		if ((*data)[1] == 0x03) {
			Macro::playerX = (*data)[4] - 1;
			Macro::playerY = (*data)[6];
			//printf("%s %d\n", "playerX:", Macro::playerX);
			//printf("%s %d\n", "playerY:", Macro::playerY);
		}
	}

	delete data;
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
	//printf("Client is sending... : \n");
	//printf("%zu: ", data.size()); // data.size()    
	//for (int i = 0; i < data.size(); i++) {
	//	printf("%02X ", data[i]);
	//}
	//printf("\n");
	std::vector<uint8_t> dataCopy = data;
	if (data[0] == 0x32 && hooks->Outgoing_Packet_Length == 8) 
		pool.enqueue(checkSendPacket, new std::vector<uint8_t>(dataCopy));
	else if (data[0] == 0x06 && hooks->Outgoing_Packet_Length == 16) 
		pool.enqueue(checkSendPacket, new std::vector<uint8_t>(dataCopy));
	else if (data[0] == 0x06 && hooks->Outgoing_Packet_Length == 20) 
		pool.enqueue(checkSendPacket, new std::vector<uint8_t>(dataCopy));
}

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

	//Macro::consoleshowtext("packet receive!!");
	// get packets
	ByteBuffer Packet((LPVOID)hooks->Ingoing_Packet_Pointer, hooks->Ingoing_Packet_Length);
	std::vector<uint8_t> data = Packet.ReadBytes(0, hooks->Ingoing_Packet_Length);
	std::vector<uint8_t> dataCopy = data;

	/*if (data[0] == 0x08 && hooks->Ingoing_Packet_Length == 49) {
		data[8] = 90;
		std::memcpy((LPVOID)hooks->Ingoing_Packet_Pointer, data.data(), data.size());
	}*/

	//1d 01 03 2a a1 00 00 02 00 59 00 63 00 00 15 00 0c 00 04 04 bc db c6 c8 00
	/*if (data[0] == 0x1d && data[7] == 0x02 && data.size() == 25) {
		data[7] = 0x00;
		std::memcpy((LPVOID)hooks->Ingoing_Packet_Pointer, data.data(), data.size());
	}

	if (data[0] == 0x33 && data[12] == 0x02 && data.size() == 30) {
		data[12] = 0x00;
		std::memcpy((LPVOID)hooks->Ingoing_Packet_Pointer, data.data(), data.size());
	}*/

	if (data[0] == 0x0C && data[4] == Macro::selectedPlayerId && hooks->Ingoing_Packet_Length == 12) {
		pool.enqueue(checkPacket, new std::vector<uint8_t>(dataCopy));
	}
	else if (data[0] == 0x11 && hooks->Ingoing_Packet_Length == 0x7) {
		pool.enqueue(checkPacket, new std::vector<uint8_t>(dataCopy));
	}
	else if (data[0] == 0x5 && hooks->Ingoing_Packet_Length == 0xd || data[0] == 0x11 && hooks->Ingoing_Packet_Length == 0x7) {
		pool.enqueue(checkPacket, new std::vector<uint8_t>(dataCopy));
	}
	else if (Macro::isCtrl == 1 && data[0] == 0x34 && data[hooks->Ingoing_Packet_Length - 2] == 0x2e && data[hooks->Ingoing_Packet_Length - 3] == 0xdb && data[hooks->Ingoing_Packet_Length - 4] == 0xc0) {
		pool.enqueue(checkPacket, new std::vector<uint8_t>(dataCopy));
	}
	else if (data[0] == 0x29 && data[5] == 0x27) {
		if (data[4] == Macro::selectedPlayerId || data[4] == Macro::playerId) {
			pool.enqueue(checkPacket, new std::vector<uint8_t>(dataCopy));
		}
	}
	else if (data[0] == 0x29 && data[5] == 0x31) {
		if (data[4] == Macro::selectedPlayerId || data[4] == Macro::playerId) {
			pool.enqueue(checkPacket, new std::vector<uint8_t>(dataCopy));
		}
	}
	else if (data[0] == 0x29 && data[5] == 0x0D) {
		if (data[4] == Macro::selectedPlayerId || data[4] == Macro::playerId) {
			pool.enqueue(checkPacket, new std::vector<uint8_t>(dataCopy));
		}
	}
	else if (data[0] == 0x29 && data[5] == 0x16) {
		if (data[4] == Macro::selectedPlayerId || data[4] == Macro::playerId) {
			pool.enqueue(checkPacket, new std::vector<uint8_t>(dataCopy));
		}
	}
	else if (data[0] == 0x29 && data[5] == 0x0A) {
		if (data[4] == Macro::selectedPlayerId || data[4] == Macro::playerId) {
			pool.enqueue(checkPacket, new std::vector<uint8_t>(dataCopy));
		}
	}
	else if (data[0] == 0x0A && data[1] == 0x03 && hooks->Ingoing_Packet_Length == 19 && Macro::isDiamond == 1 && Macro::runDiamond == 0) {
		pool.enqueue(checkPacket, new std::vector<uint8_t>(dataCopy));
		Macro::runDiamond = 1;
	}
	else if (data[0] == 0x29 && data[5] == 0x32 && data[4] == Macro::playerId && Macro::isDiamond == 1) {
		Macro::runDiamond = 0;
	}

	//std::stringstream result;
	//std::copy(data.begin(), data.end(), std::ostream_iterator<int>(result, " "));
	//std::string test = result.str();

	//int strLen = 0;
	//char nameMsg[DEFAULT_BUFLEN];
	//int y;
	//printf("client is receiving... : \n");
	//printf("%zu: ", data.size()); // data.size()
	//for (int i = 0; i < data.size(); i++) {
	//	printf("%02x ", data[i]);
	//}
	//printf("\n");
}