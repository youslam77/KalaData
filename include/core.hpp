//Copyright(C) 2025 Lost Empire Entertainment
//This program comes with ABSOLUTELY NO WARRANTY.
//This is free software, and you are welcome to redistribute it under certain conditions.
//Read LICENSE.md for more information.

#include <string>

namespace KalaData
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

	class Core
	{
	public:
		//Toggle compression verbose messages on and off
		static void SetVerboseLoggingState(bool newState) { isVerboseLoggingEnabled = newState; }
		static bool IsVerboseLoggingEnabled() { return isVerboseLoggingEnabled; }

		//Runtime loop of KalaData
		static void Update();

		//Print a message to the console with your preferred type
		static void PrintMessage(
			const string& message,
			MessageType type = MessageType::MESSAGETYPE_LOG);

		//Shut down and close because this is a bad scenario and should never happen
		static void ForceClose(const string& title, const string& message);

		//Shut down KalaData, optional critical shutdown uses quick_exit
		static void Shutdown(ShutdownState state = ShutdownState::SHUTDOWN_REGULAR);
	private:
		static inline bool isVerboseLoggingEnabled = false;
	};
}