//Copyright(C) 2025 Lost Empire Entertainment
//This program comes with ABSOLUTELY NO WARRANTY.
//This is free software, and you are welcome to redistribute it under certain conditions.
//Read LICENSE.md for more information.

#include <string>

namespace KalaData::Compression
{
	using std::string;

	class Compress
	{
	public:
		//Toggle showing detailed info:
		//  - compressed relative file is equal or bigger than raw
		//  - saved relative file is empty
		static void SetVerboseLoggingState(bool newState) { isVerboseLoggingEnabled = newState; }
		static bool IsVerboseLoggingEnabled() { return isVerboseLoggingEnabled; }

		//Compresses selected folder straight to .kdat archive inside target folder,
		//skips all safety checks that are handled in the Command class for the Compress command
		static void CompressToArchive(
			const string& origin,
			const string& target);

		//Decompresses selected .kdat archive straight to selected target folder,
		//skips all safety checks that are handled in the Command class for the Decompress command
		static void DecompressToFolder(
			const string& origin,
			const string& target);
	private:
		static inline bool isVerboseLoggingEnabled = false;
	};
}