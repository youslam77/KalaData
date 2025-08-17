//Copyright(C) 2025 Lost Empire Entertainment
//This program comes with ABSOLUTELY NO WARRANTY.
//This is free software, and you are welcome to redistribute it under certain conditions.
//Read LICENSE.md for more information.

#include <iostream>

#include "core/core.hpp"

using std::cout;
using std::clog;
using std::cerr;
using std::cin;

static bool isRunning = false;

namespace KalaData::Core
{
	void KalaDataCore::Start(
		const string& origin,
		const string& target)
	{
		PrintMessage("Initializing KalaData...");

		isRunning = true;
		KalaDataCore::Update();
	}

	void KalaDataCore::Update()
	{
		PrintMessage(
			"Entered render loop...",
			MessageType::MESSAGETYPE_DEBUG);

		while (isRunning)
		{

		}

		Shutdown();
	}

	void KalaDataCore::PrintMessage(
		const string& message,
		MessageType type)
	{
		switch (type)
		{
		case MessageType::MESSAGETYPE_LOG:
			cout << message << "\n";
			break;
		case MessageType::MESSAGETYPE_WARNING:
			clog << "[WARNING] " << message << "\n";
			break;
		case MessageType::MESSAGETYPE_ERROR:
			cerr << "[ERROR] " << message << "\n";
			break;
		case MessageType::MESSAGETYPE_SUCCESS:
			cout << "[SUCCESS] " << message << "\n";
			break;
#ifdef _DEBUG
		case MessageType::MESSAGETYPE_DEBUG:
			clog << "[DEBUG] " << message << "\n";
			break;
#endif
		}
	}

	void KalaDataCore::Shutdown(ShutdownState state)
	{
		PrintMessage("Press Enter to exit...");
		cin.get();

		if (state == ShutdownState::SHUTDOWN_CRITICAL)
		{
			PrintMessage(
				"Critical KalaData shutdown!",
				MessageType::MESSAGETYPE_WARNING);

			quick_exit(EXIT_FAILURE);
			return;
		}

		PrintMessage(
			"Regular KalaData shutdown...",
			MessageType::MESSAGETYPE_DEBUG);

		exit(0);
	}
}