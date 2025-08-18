//Copyright(C) 2025 Lost Empire Entertainment
//This program comes with ABSOLUTELY NO WARRANTY.
//This is free software, and you are welcome to redistribute it under certain conditions.
//Read LICENSE.md for more information.

#include <sstream>
#include <string>
#include <filesystem>
#include <iomanip>
#include <fstream>

#include "core/core.hpp"
#include "core/command.hpp"
#include "compression/compress.hpp"

using KalaData::Compression::Compress;

using std::stringstream;
using std::string;
using std::to_string;
using std::stringstream;
using std::filesystem::path;
using std::filesystem::exists;
using std::filesystem::remove;
using std::filesystem::is_regular_file;
using std::filesystem::is_directory;
using std::filesystem::file_size;
using std::filesystem::is_empty;
using std::filesystem::recursive_directory_iterator;
using std::fixed;
using std::setprecision;
using std::ofstream;
using std::ios;

static uint64_t GetFolderSize(const string& folderPath);

static bool CanWriteToFolder(const string& folderPath);

static string ConvertSizeToString(uint64_t size);

//5GB max file size
constexpr uint64_t maxFolderSize = 5ull * 1024 * 1024 * 1024;

namespace KalaData::Core
{
	void Command::HandleCommand(vector<string> parameters)
	{
		//remove 'KalaData> ' at the front of the parameters
		vector<string> cleanedParameters = parameters;
		cleanedParameters.erase(cleanedParameters.begin());

		if (parameters.size() == 2
			&& parameters[1] == "--v")
		{
			Command_Version();
		}

		else if (parameters.size() == 2
			&& parameters[1] == "--about")
		{
			Command_About();
			return;
		}

		else if (parameters.size() == 2
			&& parameters[1] == "--help")
		{
			Command_Help();
			return;
		}

		else if (parameters.size() == 3
			&& parameters[1] == "--help")
		{
			Command_Help_Command(parameters[2]);
			return;
		}

		else if (parameters.size() == 2
			&& parameters[1] == "--tvb")
		{
			Command_ToggleCompressionVerbosity();
		}

		else if (parameters.size() == 4
			&& parameters[1] == "--c")
		{
			Command_Compress(parameters[2], parameters[3]);
		}

		else if (parameters.size() == 4
			&& parameters[1] == "--dc")
		{
			Command_Decompress(parameters[2], parameters[3]);
		}

		else if (parameters.size() == 2
			&& parameters[1] == "--exit")
		{
			Command_Exit();
		}

		else
		{
			string command{};

			for (const auto& par : parameters)
			{
				if (par == parameters[0])
				{
					command = command + par;
				}
				else  command = command + " " + par;
			}

			string target = "KalaData.exe ";
			size_t pos = command.find(target);
			if (pos != string::npos)
			{
				command.erase(pos, target.length());
			}

			stringstream ss{};
			ss << "Unsupported command '" + command + "'! Type --help to list all commands.\n";

			KalaDataCore::PrintMessage(
				ss.str(),
				MessageType::MESSAGETYPE_ERROR);
			return;
		}
	}

	void Command::Command_Version()
	{
		KalaDataCore::PrintMessage(KALADATA_VERSION "\n");
	}

	void Command::Command_About()
	{
		stringstream ss{};

		ss << "KalaData is a custom compression and decompression tool written in C++20, "
			<< "built entirely from scratch without external dependencies.\n"
			<< "It uses a hybrid LZSS + Huffman pipeline to compress data efficiently, "
			<< "while falling back to raw or empty storage when appropriate.\n"
			<< "All data is stored in a dedicated archival format with the '.kdat' extension.\n\n"

			<< "KalaData was created by and is maintained by KalaKit, an organization owned by Lost Empire Entertainment.\n"
			<< "Official repository: 'https://github.com/KalaKit/KalaData'\n";

		KalaDataCore::PrintMessage(ss.str());
	}

	void Command::Command_Help()
	{
		stringstream ss{};
		ss << "====================\n\n"

			<< "Notes:\n"
			<< "  - all paths are absolute and KalaData does not take relative paths.\n"
			<< "  - if you use '--help command' you must put a real command there, for example '--help c'\n\n"

			<< "Commands:\n"
			<< "  --v\n"
			<< "  --about\n"
			<< "  --help\n"
			<< "  --help command\n"
			<< "  --tvb\n"
			<< "  --c\n"
			<< "  --dc\n"
			<< "  --exit\n\n"

			<< "====================\n";

		KalaDataCore::PrintMessage(ss.str());
	}

	void Command::Command_Help_Command(const string& commandName)
	{
		if (commandName == "v"
			|| commandName == "--v")
		{
			KalaDataCore::PrintMessage("Prints the KalaData version\n");
		}

		else if (commandName == "about"
			|| commandName == "--about")
		{
			KalaDataCore::PrintMessage("Prints the KalaData description\n");
		}

		else if (commandName == "help"
			|| commandName == "--help")
		{
			KalaDataCore::PrintMessage("Lists all commands\n");
		}

		else if (commandName == "tvb"
			|| commandName == "--tvb")
		{
			KalaDataCore::PrintMessage("Toggles compression verbose messages on and off\n");
		}

		else if (commandName == "c"
			|| commandName == "--c")
		{
			stringstream ss{};

			ss << "Takes in a folder which will be compressed into a '.kdat' file inside the target path parent folder.\n\n"
				<< "Requirements and restrictions:\n\n"

				<< "Origin:\n"
				<< "  - path must exist\n"
				<< "  - path must be a folder\n"
				<< "  - folder must not be empty\n"
				<< "  - folder size must not exceed 5GB\n\n"

				<< "Target:\n"
				<< "  - path must not exist\n"
				<< "  - path must have the '.kdat' extension\n"
				<< "  - path parent folder must be writable\n";

			KalaDataCore::PrintMessage(ss.str());
		}

		else if (commandName == "dc"
			|| commandName == "--dc")
		{
			stringstream ss{};

			ss << "Takes in a compressed '.kdat' file path which will be decompressed inside the target folder.\n\n"
				<< "Requirements and restrictions:\n\n"

				<< "Origin:\n"
				<< "  - path must exist\n"
				<< "  - path must be a regular file\n"
				<< "  - path must have the '.kdat' extension\n\n"

				<< "Target:\n"
				<< "  - path must exist\n"
				<< "  - path must be a folder\n"
				<< "  - folder must be writable\n";

			KalaDataCore::PrintMessage(ss.str());
		}

		else if (commandName == "exit"
			|| commandName == "--exit")
		{
			KalaDataCore::PrintMessage("Shuts down KalaData\n");
		}

		else
		{
			KalaDataCore::PrintMessage(
				"Cannot get info about command '" + commandName + "' because it does not exist! Type '--help' to list all commands\n",
				MessageType::MESSAGETYPE_ERROR);
		}
	}

	void Command::Command_ToggleCompressionVerbosity()
	{
		bool state = Compress::IsVerboseLoggingEnabled();
		state = !state;

		Compress::SetVerboseLoggingState(state);

		string stateStr = state ? "true" : "false";

		KalaDataCore::PrintMessage(
			"Set compression verbose logging state to '" + stateStr + "'!\n");
	}

	void Command::Command_Compress(
		const string& origin,
		const string& target)
	{
		if (!exists(origin))
		{
			KalaDataCore::PrintMessage(
				"Origin path '" + origin + "' does not exist!\n",
				MessageType::MESSAGETYPE_ERROR);

			return;
		}

		if (!is_directory(origin))
		{
			KalaDataCore::PrintMessage(
				"Origin '" + origin + "' must be a directory!\n",
				MessageType::MESSAGETYPE_ERROR);

			return;
		}

		if (is_empty(origin))
		{
			KalaDataCore::PrintMessage(
				"Origin '" + origin + "' must not be an empty folder!\n",
				MessageType::MESSAGETYPE_ERROR);

			return;
		}

		uint64_t originSize = GetFolderSize(origin);
		if (originSize > maxFolderSize)
		{
			string convertedOriginSize = ConvertSizeToString(originSize);

			KalaDataCore::PrintMessage(
				"Origin '" + origin + "' size '" + convertedOriginSize + "' exceeds max allowed size '5.00GB'!\n",
				MessageType::MESSAGETYPE_ERROR);

			return;
		}

		if (exists(target))
		{
			KalaDataCore::PrintMessage(
				"Target '" + target + "' already exists!\n",
				MessageType::MESSAGETYPE_ERROR);

			return;
		}

		if (path(target).extension().string() != ".kdat")
		{
			KalaDataCore::PrintMessage(
				"Target path '" + target + "' must have the '.kdat' extension!\n",
				MessageType::MESSAGETYPE_ERROR);

			return;
		}

		string targetParentFolder = path(target).parent_path().string();
		if (!CanWriteToFolder(targetParentFolder))
		{
			KalaDataCore::PrintMessage(
				"Insufficient permissions to write to target parent folder '" + targetParentFolder + "'!\n",
				MessageType::MESSAGETYPE_ERROR);

			return;
		}

		Compress::CompressToArchive(origin, target);
	}

	void Command::Command_Decompress(
		const string& origin,
		const string& target)
	{
		if (!exists(origin))
		{
			KalaDataCore::PrintMessage(
				"Origin path '" + origin + "' does not exist!\n",
				MessageType::MESSAGETYPE_ERROR);

			return;
		}

		if (!is_regular_file(origin))
		{
			KalaDataCore::PrintMessage(
				"Origin '" + origin + "' must be a regular file!\n",
				MessageType::MESSAGETYPE_ERROR);

			return;
		}

		if (path(origin).extension().string() != ".kdat")
		{
			KalaDataCore::PrintMessage(
				"Origin '" + origin + "' must have the '.kdat' extension!\n",
				MessageType::MESSAGETYPE_ERROR);

			return;
		}

		if (!exists(target))
		{
			KalaDataCore::PrintMessage(
				"Target folder '" + target + "' does not exist!\n",
				MessageType::MESSAGETYPE_ERROR);

			return;
		}

		if (!is_directory(target))
		{
			KalaDataCore::PrintMessage(
				"Target '" + target + "' must be a folder!\n",
				MessageType::MESSAGETYPE_ERROR);

			return;
		}

		string targetParentFolder = path(target).parent_path().string();
		if (!CanWriteToFolder(targetParentFolder))
		{
			KalaDataCore::PrintMessage(
				"Insufficient permissions to write to target parent folder '" + targetParentFolder + "'!\n",
				MessageType::MESSAGETYPE_ERROR);

			return;
		}

		Compress::DecompressToFolder(origin, target);
	}

	void Command::Command_Exit()
	{
		KalaDataCore::Shutdown();
	}
}

uint64_t GetFolderSize(const string& folderPath)
{
	uint64_t size{};

	for (auto& p : recursive_directory_iterator(folderPath))
	{
		if (is_regular_file(p)) size += file_size(p);
	}

	return size;
}

bool CanWriteToFolder(const string& folderPath)
{
	try
	{
		path testFile = path(folderPath) / ".kaladata_write_access_test";
		ofstream writeTest(testFile.string(), ios::out | ios::trunc);

		if (!writeTest.is_open()) return false;

		writeTest << "test";
		writeTest.close();

		remove(testFile);

		return true;
	}
	catch (...)
	{
		return false;
	}
}

string ConvertSizeToString(uint64_t size)
{
	stringstream ss{};
	ss << fixed
		<< setprecision(2)
		<< static_cast<double>(size) / (1024ull * 1024 * 1024)
		<< "GB";

	return ss.str();
}