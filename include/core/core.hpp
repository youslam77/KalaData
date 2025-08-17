//Copyright(C) 2025 Lost Empire Entertainment
//This program comes with ABSOLUTELY NO WARRANTY.
//This is free software, and you are welcome to redistribute it under certain conditions.
//Read LICENSE.md for more information.

#include <string>

namespace KalaData::Core
{
	using std::string;

	enum class ShutdownState
	{
		SHUTDOWN_REGULAR,
		SHUTDOWN_CRITICAL
	};

	class KalaDataCore
	{
	public:
		static void Start();

		static void Update();

		static void Shutdown(ShutdownState state = ShutdownState::SHUTDOWN_REGULAR);
	};
}