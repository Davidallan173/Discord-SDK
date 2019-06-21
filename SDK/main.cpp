#define _CRT_SECURE_NO_WARNINGS

#include <array>
#include <cassert>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <thread>
#include <vector>

#include "discord.h"


struct DiscordState {
	discord::User currentUser;

	std::unique_ptr<discord::Core> core;
};

discord::Core* core{};
DiscordState state{};
auto result = discord::Core::Create(APPLICATION_ID, DiscordCreateFlags_Default, &core);

void setup() {
	state.core.reset(core);
	if (!state.core) {
		std::cout << "Failed to instantiate discord core! (err " << static_cast<int>(result)
			<< ")\n";
		std::exit(-1);
	}

	state.core->SetLogHook(
		discord::LogLevel::Debug, [](discord::LogLevel level, const char* message) {
		std::cerr << "Log(" << static_cast<uint32_t>(level) << "): " << message << "\n";
	});

	state.core->ActivityManager().RegisterCommand("PATH TO LAUNCHER/GAME HERE");

	state.core->ActivityManager().OnActivityJoin.Connect(
		[](const char* secret) { std::cout << "Join " << secret << "\n"; });
	state.core->ActivityManager().OnActivitySpectate.Connect(
		[](const char* secret) { std::cout << "Spectate " << secret << "\n"; });
	state.core->ActivityManager().OnActivityJoinRequest.Connect([](discord::User const& user) {
		std::cout << "Join Request " << user.GetUsername() << "\n";
	});
	state.core->ActivityManager().OnActivityInvite.Connect(
		[](discord::ActivityActionType, discord::User const& user, discord::Activity const&) {
		std::cout << "Invite " << user.GetUsername() << "\n";
	});
	state.core->LobbyManager().OnLobbyUpdate.Connect(
		[](std::int64_t lobbyId) { std::cout << "Lobby update " << lobbyId << "\n"; });

	state.core->LobbyManager().OnLobbyDelete.Connect(
		[](std::int64_t lobbyId, std::uint32_t reason) {
		std::cout << "Lobby delete " << lobbyId << " (reason: " << reason << ")\n";
	});

	state.core->LobbyManager().OnMemberConnect.Connect(
		[](std::int64_t lobbyId, std::int64_t userId) {
		std::cout << "Lobby member connect " << lobbyId << " userId " << userId << "\n";
	});

	state.core->LobbyManager().OnMemberUpdate.Connect(
		[](std::int64_t lobbyId, std::int64_t userId) {
		std::cout << "Lobby member update " << lobbyId << " userId " << userId << "\n";
	});

	state.core->LobbyManager().OnMemberDisconnect.Connect(
		[](std::int64_t lobbyId, std::int64_t userId) {
		std::cout << "Lobby member disconnect " << lobbyId << " userId " << userId << "\n";
	});

	state.core->LobbyManager().OnLobbyMessage.Connect([&](std::int64_t lobbyId,
		std::int64_t userId,
		std::uint8_t* payload,
		std::uint32_t payloadLength) {
		std::vector<uint8_t> buffer{};
		buffer.resize(payloadLength);
		memcpy(buffer.data(), payload, payloadLength);
		std::cout << "Lobby message " << lobbyId << " from " << userId << " of length "
			<< payloadLength << " bytes.\n";
	});

	state.core->LobbyManager().OnSpeaking.Connect(
		[&](std::int64_t, std::int64_t userId, bool speaking) {
		std::cout << "User " << userId << " is " << (speaking ? "" : "NOT ") << "speaking.\n";
	});
}

void voiceConnect(discord::Result result) {
}

discord::Activity activity{};

discord::Activity GenActivity(const char * Detail, const char * Gmstate, const char * Smallimg, const char * smallTxt,
	const char * LrgImg, const char * LrgTxt, uint32_t MaxSize, uint32_t currentSize) {
	discord::Activity activity{};
	DiscordActivityParty Party{};
	activity.SetDetails(Detail);
	activity.SetState(Gmstate);
	activity.GetAssets().SetSmallImage(Smallimg);
	activity.GetAssets().SetSmallText(smallTxt);
	activity.GetAssets().SetLargeImage(LrgImg);
	activity.GetAssets().SetLargeText(LrgTxt);;
	activity.GetParty().SetId("");
	activity.GetParty().GetSize().SetMaxSize(MaxSize);
	activity.GetParty().GetSize().SetCurrentSize(currentSize);
	activity.SetType(discord::ActivityType::Playing);
	return activity;
}

void SetPresence(discord::Activity Activ) {
	state.core->ActivityManager().UpdateActivity(Activ, [](discord::Result result) {
		std::cout << ((result == discord::Result::Ok) ? "Succeeded" : "Failed")
			<< " updating activity!\n";
	});
}

int interrupted;
int main(int, char *argv[]) {
	setup();
	std::signal(SIGINT, [](int) { interrupted = true; });
	discord::Activity Data = GenActivity("Testing", "", "", "", "city-central", "Testing", 0, 0);
	SetPresence(Data);
	do {
		state.core->RunCallbacks();
		std::this_thread::sleep_for(std::chrono::milliseconds(16));
	} while (!interrupted);
};

extern "C" {

	__declspec(dllexport) void DLLMain() {
		setup();
		state.core->RunCallbacks();
	}

	__declspec(dllexport) void DoCallbacks() {
		state.core->RunCallbacks();
	}

	__declspec(dllexport) void SetData(char* Detail, char* Gmstate, char* Smallimg, char* smallTxt,
		char* LrgImg,
		char* LrgTxt, int32_t Max, int32_t current) {
		//discord::Activity Data = GenActivity(Detail, Gmstate, Smallimg, smallTxt, LrgImg, LrgTxt);
		discord::Activity Data = GenActivity(Detail, Gmstate, Smallimg, smallTxt, LrgImg, LrgTxt, Max, current);
		SetPresence(Data);
	}


}