#include "stdafx.h"
#include "SocketListener.h"
#include "APIPackets.h"
#include "API.h"
#include "Target.h"
#include "Debug.h"
#include "System.h"
#include "Library.h"

std::unique_ptr<SocketListener> API::Listener;

// Register Command call backs.
const std::map<int, std::function<void(SceNetId s)>> API::APICommands =
{
	// Apps Commands
	//API_APPS_CHECK_VER,
	//API_APPS_GET_DB,
	//API_APPS_GET_INFO_STR,
	//API_APPS_STATUS,
	//API_APPS_START,
	//API_APPS_STOP,
	//API_APPS_SUSPEND,
	//API_APPS_RESUME,
	//API_APPS_DELETE,
	//API_APPS_SET_VISIBILITY,
	//API_APPS_GET_VISIBILITY,

	// Debug Commands
	{ API_DBG_ATTACH, Debug::Attach },
	{ API_DBG_DETACH, Debug::Detach },
	{ API_DBG_GET_CURRENT, Debug::Current },
	{ API_DBG_READ, [](SceNetId s) { Debug::RWMemory(s, false); } },
	{ API_DBG_WRITE, [](SceNetId s) { Debug::RWMemory(s, true); } },
	//API_DBG_KILL,
	//API_DBG_BREAK,
	//API_DBG_RESUME,
	//API_DBG_SIGNAL,
	//API_DBG_STEP,
	//API_DBG_STEP_OVER,
	//API_DBG_STEP_OUT,
	//API_DBG_GET_CALLSTACK,
	//API_DBG_GET_REG,
	//API_DBG_SET_REG,
	//API_DBG_GET_FREG,
	//API_DBG_SET_FREG,
	//API_DBG_GET_DBGREG,
	//API_DBG_SET_DBGREG,

	// Library Commands
	{ API_DBG_LOAD_LIBRARY, Library::LoadLibrary },
	{ API_DBG_UNLOAD_LIBRARY, Library::UnloadLibrary },
	{ API_DBG_RELOAD_LIBRARY, Library::ReloadLibrary },
	{ API_DBG_LIBRARY_LIST, Library::GetLibraryList },

	// Target Commands
	{ API_TARGET_INFO, Target::SendTargetInfo },
	{ API_TARGET_RESTMODE, [](SceNetId) { ChangeSystemState(NewSystemState::Suspend); } },
	{ API_TARGET_SHUTDOWN, [](SceNetId) { ChangeSystemState(NewSystemState::Shutdown); } },
	{ API_TARGET_REBOOT, [](SceNetId) { ChangeSystemState(NewSystemState::Reboot); } },
	{ API_TARGET_NOTIFY, Target::DoNotify },
	{ API_TARGET_BUZZER, Target::DoBuzzer },
	{ API_TARGET_SET_LED, Target::SetConsoleLED },
	{ API_TARGET_SET_SETTINGS, Target::SetSettings },
	{ API_TARGET_GET_PROC_LIST, Target::ProcList },
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
		// Send failed to verify packet.
		Sockets::SendInt(s, 0);

		klog("Invalid Packet with Magic '%s' and Version %i\nExpected 'ORBIS_SUITE' and %i\n", packet->PacketMagic, packet->PacketVersion, PACKET_VERSION);

		return;
	}

	// Send successfully verified packet.
	Sockets::SendInt(s, 1);

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