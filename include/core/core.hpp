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

	enum class MessageType
	{
		MESSAGETYPE_LOG,
		MESSAGETYPE_DEBUG,
		MESSAGETYPE_WARNING,
		MESSAGETYPE_ERROR,
		MESSAGETYPE_SUCCESS
	};

	class Color
	{
	public:
		static inline const char* reset = "\033[0m";
		static inline const char* red = "\033[31m";
		static inline const char* green = "\033[32m";
		static inline const char* yellow = "\033[33m";
		static inline const char* blue = "\033[34m";
	};

	class KalaDataCore
	{
	public:
		static void Update();

		static void PrintMessage(
			const string& message,
			MessageType type = MessageType::MESSAGETYPE_LOG);

		static void Shutdown(ShutdownState state = ShutdownState::SHUTDOWN_REGULAR);
	};
}