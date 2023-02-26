#pragma once
#define PACKET_VERSION 5

struct InitialPacket
{
	char PacketMagic[12];
	int PacketVersion;
	int Command;
};