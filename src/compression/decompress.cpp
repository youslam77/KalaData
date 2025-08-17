//Copyright(C) 2025 Lost Empire Entertainment
//This program comes with ABSOLUTELY NO WARRANTY.
//This is free software, and you are welcome to redistribute it under certain conditions.
//Read LICENSE.md for more information.

#include <filesystem>

#include "compression/decompress.hpp"
#include "core/core.hpp"

using KalaData::Core::KalaDataCore;
using KalaData::Core::MessageType;

using std::filesystem::path;

namespace KalaData::Compression
{
	void Decompress::DecompressToFolder(
		const string& origin,
		const string& target)
	{
		string archivePath = (path(origin) / path(origin).stem()).string() + ".kdat";
		string archiveName = path(archivePath).filename().string();

		KalaDataCore::PrintMessage(
			"Starting to decompress archive '" + archivePath + "' to folder '" + target + "'!\n",
			MessageType::MESSAGETYPE_SUCCESS);
	}
}