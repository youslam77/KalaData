//Copyright(C) 2025 Lost Empire Entertainment
//This program comes with ABSOLUTELY NO WARRANTY.
//This is free software, and you are welcome to redistribute it under certain conditions.
//Read LICENSE.md for more information.

#include <iostream>

#include "core/core.hpp"

using std::cout;

static bool isRunning = false;

namespace KalaData::Core
{
	void KalaDataCore::Start()
	{
		cout << "Initializing KalaData...\n";

		isRunning = true;
		KalaDataCore::Update();
	}

	void KalaDataCore::Update()
	{
		cout << "Entered runtime loop...\n";

		while (isRunning)
		{

		}

		Shutdown();
	}

	void KalaDataCore::Shutdown(ShutdownState state)
	{
		if (state == ShutdownState::SHUTDOWN_CRITICAL)
		{
			cout << "Critical KalaData shutdown!\n";

			quick_exit(EXIT_FAILURE);
			return;
		}

		cout << "Shutting down KalaData...\n";

		exit(0);
	}
}