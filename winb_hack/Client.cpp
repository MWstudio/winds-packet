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

void curse(int playerId, int x, int y, int map1, int map2, int map3) {
	// 0F 0D 01 03 2A 22 00 07 00 08 00 ��
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
	printf("curse : x,y,id: %d, %d %d\n", Macro::playerX, Macro::playerY, Macro::playerId);
	return;
}

void necromancy(int playerId, int x, int y, int map1, int map2, int map3) {
	// 0F 08 01 03 2A 22 00 07 00 08 00 ��ȥ  
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
	printf("necromancy : x, y, id, map1, map2, map3: %d,%d %d %d %d %d\n", x, y, playerId, map1, map2, map3);
	return;
}

DWORD WINAPI checkPacket(LPVOID lpParam) {
	++Macro::threadCount;
	printf("threadCount : %d\n", Macro::threadCount);
	std::vector<uint8_t>* data = (std::vector<uint8_t>*)lpParam;
	size_t dataSize = data->size();
	// 0c 15 59 99 90 00 12 00 0d 02 a9 00
	if ((*data)[0] == 0x0C &&  (*data)[4] == Macro::selectedPlayerId && dataSize == 12) {
		if ((*data)[9] == 0x00) {
			Macro::selectedPlayerX = (*data)[6];
			Macro::selectedPlayerY = (*data)[8] - 1;
			printf("%s %d\n", "playerX:", Macro::selectedPlayerX);
			printf("%s %d\n", "playerY:", Macro::selectedPlayerY);
		}
		if ((*data)[9] == 0x01) {
			Macro::selectedPlayerX = (*data)[6] + 1;
			Macro::selectedPlayerY = (*data)[8];
			printf("%s %d\n", "playerX:", Macro::selectedPlayerX);
			printf("%s %d\n", "playerY:", Macro::selectedPlayerY);
		}
		if ((*data)[9] == 0x02) {
			Macro::selectedPlayerX = (*data)[6];
			Macro::selectedPlayerY = (*data)[8] + 1;
			printf("%s %d\n", "playerX:", Macro::selectedPlayerX);
			printf("%s %d\n", "playerY:", Macro::selectedPlayerY);
		}
		if ((*data)[9] == 0x03) {
			Macro::selectedPlayerX = (*data)[6] - 1;
			Macro::selectedPlayerY = (*data)[8];
			printf("%s %d\n", "playerX:", Macro::selectedPlayerX);
			printf("%s %d\n", "playerY:", Macro::selectedPlayerY);
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
			printf("selectedPlayerId :: %d\n", Macro::selectedPlayerId);
		}
	}
	// 29 3a 75 a3 78 0d 00
	// 29 3a 75 a3 78 31 00
	// 29 01 03 2b 49 31 00
	// 
	// ȥ�� 29 ?? ?? ?? unidque_id 27 00
	else if ((*data)[0] == 0x29 && (*data)[5] == 0x27) {
		if ((*data)[4] == Macro::selectedPlayerId) {
			Macro::necromancyReceivedSelected = 1;  // Set the flag to true
			while (true) {
				Sleep(300);
				necromancy(Macro::selectedPlayerId, Macro::selectedPlayerX, Macro::selectedPlayerY, Macro::map1, Macro::map2, Macro::map3);
				if (Macro::necromancyReceivedSelected == 0) {
					break;
				}
			}
		}
		else if ((*data)[4] == Macro::playerId) {
			Macro::necromancyReceivedMe = 1;  // Set the flag to true
			while (true) {
				Sleep(300);
				necromancy(Macro::playerId, Macro::playerX, Macro::playerY, Macro::map1, Macro::map2, Macro::map3);
				if (Macro::necromancyReceivedMe == 0) {
					break;
				}
			}
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
			while (true) {
				Sleep(300);
				curse(Macro::selectedPlayerId, Macro::selectedPlayerX, Macro::selectedPlayerY, Macro::map1, Macro::map2, Macro::map3);
				if (Macro::curseReceivedSelected == 0) {
					break;
				}
			}
		}
		else if ((*data)[4] == Macro::playerId) {
			Macro::curseReceivedMe = 1;  // Set the flag to true
			while (true) {
				Sleep(300);
				curse(Macro::playerId, Macro::playerX, Macro::playerY, Macro::map1, Macro::map2, Macro::map3);
				if (Macro::curseReceivedMe == 0) {
					break;
				}
			}
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

	--Macro::threadCount;
	printf("threadCount : %d\n", Macro::threadCount);
	delete data;
	ExitThread(0);
	return 0;
}

DWORD WINAPI checkSendPacket(LPVOID lpParam) {
	++Macro::threadCount;
	printf("threadCount : %d\n", Macro::threadCount);
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

	--Macro::threadCount;
	printf("threadCount : %d\n", Macro::threadCount);
	delete data;
	ExitThread(0);
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
	printf("%zu: ", data.size()); // data.size() ���
	for (int i = 0; i < data.size(); i++) {
		printf("%02X ", data[i]);
	}
	printf("\n");
	
	std::vector<uint8_t> dataCopy = data;
	if (data[0] == 0x32 && hooks->Outgoing_Packet_Length == 8) 
		CreateThread(NULL, 0, checkSendPacket, new std::vector<uint8_t>(dataCopy), 0, NULL);
	else if (data[0] == 0x06 && hooks->Outgoing_Packet_Length == 16) 
		CreateThread(NULL, 0, checkSendPacket, new std::vector<uint8_t>(dataCopy), 0, NULL);
	else if (data[0] == 0x06 && hooks->Outgoing_Packet_Length == 20) 
		CreateThread(NULL, 0, checkSendPacket, new std::vector<uint8_t>(dataCopy), 0, NULL);
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
	std::vector<uint8_t> dataCopy = data;
	
	if (data[0] == 0x0C && data[4] == Macro::selectedPlayerId && hooks->Ingoing_Packet_Length == 12)
		CreateThread(NULL, 0, checkPacket, new std::vector<uint8_t>(dataCopy), 0, NULL);
	else if (data[0] == 0x11 && hooks->Ingoing_Packet_Length == 0x7)
		CreateThread(NULL, 0, checkPacket, new std::vector<uint8_t>(dataCopy), 0, NULL);
	else if (data[0] == 0x5 && hooks->Ingoing_Packet_Length == 0xd || data[0] == 0x11 && hooks->Ingoing_Packet_Length == 0x7)
		CreateThread(NULL, 0, checkPacket, new std::vector<uint8_t>(dataCopy), 0, NULL);
	else if (data[0] == 0x34 && data[hooks->Ingoing_Packet_Length - 2] == 0x2e && data[hooks->Ingoing_Packet_Length - 3] == 0xdb && data[hooks->Ingoing_Packet_Length - 4] == 0xc0)
		CreateThread(NULL, 0, checkPacket, new std::vector<uint8_t>(dataCopy), 0, NULL);
	else if (data[0] == 0x29 && data[5] == 0x27) 
		CreateThread(NULL, 0, checkPacket, new std::vector<uint8_t>(dataCopy), 0, NULL);
	else if (data[0] == 0x29 && data[5] == 0x31) 
		CreateThread(NULL, 0, checkPacket, new std::vector<uint8_t>(dataCopy), 0, NULL);
	else if (data[0] == 0x29 && data[5] == 0x0D) 
		CreateThread(NULL, 0, checkPacket, new std::vector<uint8_t>(dataCopy), 0, NULL);
	else if (data[0] == 0x29 && data[5] == 0x16) 
		CreateThread(NULL, 0, checkPacket, new std::vector<uint8_t>(dataCopy), 0, NULL);

	//std::stringstream result;
	//std::copy(data.begin(), data.end(), std::ostream_iterator<int>(result, " "));
	//std::string test = result.str();

	//int strLen = 0;
	//char nameMsg[DEFAULT_BUFLEN];
	//int y;
	printf("client is receiving... : \n");
	printf("%zu: ", data.size()); // data.size()
	for (int i = 0; i < data.size(); i++) {
		printf("%02x ", data[i]);
	}
	printf("\n");

	//if (data[0] == 0x34 && data[1] == 0x00 && data[2] == 0x00 && data[3] == 0x00 && data[4] == 0x04) {
	//	Macro::playerId = data[data.size() - 29];
	//	printf("PlayerId :: %d\n", Macro::playerId);
	//}
	//// 3A 04 C0 FA C1 D6 00 00 00 B9 00 ����
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