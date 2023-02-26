#include "stdafx.h"
#include "SocketListener.h"
#include "APIPackets.h"
#include "API.h"

std::unique_ptr<SocketListener> API::Listener;

// Register Command call backs.
const std::map<int, std::function<void(SceNetId s)>> API::APICommands =
{
	
};

void API::ListenerCallback(void* tdParam, SceNetId s, SceNetInAddr sin_addr)
{
	auto packet = Sockets::RecieveType<InitialPacket>(s);

	// Did we recieve a packet?
	if (packet == nullptr)
	{
		return;
	}

	// Validate Packet
	if (strcmp(packet->PacketMagic, "ORBIS_SUITE") || packet->PacketVersion != PACKET_VERSION)
	{
		klog("Invalid Packet with Magic '%s' and Version %i\nExpected 'ORBIS_SUITE' and %i\n", packet->PacketMagic, packet->PacketVersion, PACKET_VERSION);

		return;
	}

	// Add host to the host list.
	Events::AddHost(sin_addr.s_addr);

	// Find the command in the map
	auto it = API::APICommands.find(packet->Command);

	// Check if the command exists in the map
	if (it != API::APICommands.end())
	{
		// Call the command function with the given argument
		it->second(s);
	}
	else
	{
		klog("Command %d does not exist.\n", packet->Command);
	}
}

void API::Init()
{
	Listener = std::make_unique<SocketListener>(ListenerCallback, nullptr, API_PORT);
}

void API::Term()
{

}