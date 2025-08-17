//Copyright(C) 2025 Lost Empire Entertainment
//This program comes with ABSOLUTELY NO WARRANTY.
//This is free software, and you are welcome to redistribute it under certain conditions.
//Read LICENSE.md for more information.

#include <iostream>
#include <sstream>
#include <iterator>
#include <vector>

#include "core/core.hpp"
#include "core/command.hpp"

using std::cout;
using std::clog;
using std::cerr;
using std::cin;
using std::istringstream;
using std::istream_iterator;
using std::vector;

static bool isRunning = false;

namespace KalaData::Core
{
	void KalaDataCore::Update()
	{
		isRunning = true;

		while (isRunning)
		{
			cout << "KalaData> ";

			string input;
			getline(cin, input);

			istringstream iss(input);
			vector<string> tokens
			{
				istream_iterator<string>{iss},
				istream_iterator<string>{}
			};
			tokens.insert(tokens.begin(), "KalaData.exe");

			Command::HandleCommand(tokens);
		}
	}

	void KalaDataCore::PrintMessage(
		const string& message,
		MessageType type)
	{
		switch (type)
		{
		case MessageType::MESSAGETYPE_LOG:
			cout << Color::reset << message << "\n";
			break;
		case MessageType::MESSAGETYPE_WARNING:
			clog << Color::yellow << "[WARNING] " << message << Color::reset << "\n";
			break;
		case MessageType::MESSAGETYPE_ERROR:
			cerr << Color::red << "[ERROR] " << message << Color::reset << "\n";
			break;
		case MessageType::MESSAGETYPE_SUCCESS:
			cout << Color::green << "[SUCCESS] " << message << Color::reset << "\n";
			break;
#ifdef _DEBUG
		case MessageType::MESSAGETYPE_DEBUG:
			clog << Color::blue << "[DEBUG] " << message << Color::reset << "\n";
			break;
#endif
		}
	}

	void KalaDataCore::Shutdown(ShutdownState state)
	{
		if (state == ShutdownState::SHUTDOWN_CRITICAL)
		{
			PrintMessage(
				"Critical KalaData shutdown!\n",
				MessageType::MESSAGETYPE_WARNING);

			quick_exit(EXIT_FAILURE);
			return;
		}

		PrintMessage(
			"KalaData has shut down normally.\n",
			MessageType::MESSAGETYPE_DEBUG);

		exit(0);
	}
}