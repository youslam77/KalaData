//Copyright(C) 2025 Lost Empire Entertainment
//This program comes with ABSOLUTELY NO WARRANTY.
//This is free software, and you are welcome to redistribute it under certain conditions.
//Read LICENSE.md for more information.

#include <vector>
#include <string>

namespace KalaData::Core
{
	using std::vector;
	using std::string;

	class Command
	{
	public:
		//Controls how a command is handled
		static void HandleCommand(vector<string> parameters);

		//Print version to console
		static void Command_Version();

		//Print KalaData info to console
		static void Command_Info();

		//List all commands in console
		static void Command_Help();

		//Print info about selected command in console
		static void Command_Help_Command(const string& commandName);

		//Toggles compression verbose messages on and off
		static void Command_ToggleCompressionVerbosity();

		//Compression pre-checks
		static void Command_Compress(
			const string& origin,
			const string& target);

		//Decompression pre-checks
		static void Command_Decompress(
			const string& origin,
			const string& target);

		//Shuts down KalaData
		static void Command_Exit();
	};
}