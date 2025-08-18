//Copyright(C) 2025 Lost Empire Entertainment
//This program comes with ABSOLUTELY NO WARRANTY.
//This is free software, and you are welcome to redistribute it under certain conditions.
//Read LICENSE.md for more information.

#include <sstream>
#include <string>
#include <filesystem>
#include <iomanip>
#include <fstream>
#include <unordered_map>

#include "core.hpp"
#include "command.hpp"
#include "compress.hpp"

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
using std::unordered_map;

static uint64_t GetFolderSize(const string& folderPath);

static bool CanWriteToFolder(const string& folderPath);

static string ConvertSizeToString(uint64_t size);

struct Preset
{
	size_t window;
	size_t lookahead;
};

static const unordered_map<string, Preset> presets =
{
	{ "fastest",  { KalaData::WINDOW_SIZE_FASTEST,  KalaData::LOOKAHEAD_FASTEST  } },
	{ "fast",     { KalaData::WINDOW_SIZE_FAST,     KalaData::LOOKAHEAD_FAST     } },
	{ "balanced", { KalaData::WINDOW_SIZE_BALANCED, KalaData::LOOKAHEAD_BALANCED } },
	{ "slow",     { KalaData::WINDOW_SIZE_SLOW,     KalaData::LOOKAHEAD_SLOW     } },
	{ "archive",  { KalaData::WINDOW_SIZE_ARCHIVE,  KalaData::LOOKAHEAD_ARCHIVE  } }
};

//5GB max file size
constexpr uint64_t maxFolderSize = 5ull * 1024 * 1024 * 1024;

namespace KalaData
{
	void Command::HandleCommand(vector<string> parameters)
	{
		if (Compress::IsActive())
		{
			Core::PrintMessage(
				"Cannot write any commands while compressing or decompressing!",
				MessageType::MESSAGETYPE_ERROR);
			return;
		}

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

		else if (parameters.size() == 3
			&& parameters[1] == "-sm")
		{
			Command_SetCompressionMode(parameters[2]);
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

			Core::PrintMessage(
				ss.str(),
				MessageType::MESSAGETYPE_ERROR);
			return;
		}
	}

	void Command::Command_Version()
	{
		Core::PrintMessage(KALADATA_VERSION "\n");
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

		Core::PrintMessage(ss.str());
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

		Core::PrintMessage(ss.str());
	}

	void Command::Command_Help_Command(const string& commandName)
	{
		if (commandName == "v"
			|| commandName == "--v")
		{
			Core::PrintMessage("Prints the KalaData version\n");
		}

		else if (commandName == "about"
			|| commandName == "--about")
		{
			Core::PrintMessage("Prints the KalaData description\n");
		}

		else if (commandName == "help"
			|| commandName == "--help")
		{
			Core::PrintMessage("Lists all commands\n");
		}

		else if (commandName == "tvb"
			|| commandName == "--tvb")
		{
			stringstream ss{};

			ss << "Toggles compression verbose messages on and off.\n"
				<< "If true, then the following info is also displayed:\n\n"

				<< "individual file logs:\n"
				<< "  - compressed/decompressed file is empty\n"
				<< "  - original file size is bigger than the "
				<< "compressed file size so it will not be compressed/decompressed\n"
				<< "  - stored file size is smaller or equal than the "
				<< "compressed file size so it will be compressed/decompressed\n\n"
				
				<< "compression/decompression success log additional rows:\n"
				<< "  - compression/expansion ratio\n"
				<< "  - compression/expansion factor\n"
				<< "  - throughput\n"
				<< "  - total files\n"
				<< "  - compressed files\n"
				<< "  - raw files\n"
				<< "  - empty files\n";

			Core::PrintMessage(ss.str());
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

			Core::PrintMessage(ss.str());
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

			Core::PrintMessage(ss.str());
		}

		else if (commandName == "exit"
			|| commandName == "--exit")
		{
			Core::PrintMessage("Shuts down KalaData\n");
		}

		else
		{
			Core::PrintMessage(
				"Cannot get info about command '" + commandName + "' because it does not exist! Type '--help' to list all commands\n",
				MessageType::MESSAGETYPE_ERROR);
		}
	}

	void Command::Command_SetCompressionMode(const string& mode)
	{
		auto it = presets.find(mode);
		if (it != presets.end())
		{
			Core::PrintMessage(
				"Compression mode '" + mode + "' does not exist!\n",
				MessageType::MESSAGETYPE_ERROR);

			return;
		}

		Compress::SetWindowSize(it->second.window);
		Compress::SetLookAhead(it->second.lookahead);

		Core::PrintMessage(
			"Set compression mode to '" + mode + "'!\n",
			MessageType::MESSAGETYPE_SUCCESS);
	}

	void Command::Command_ToggleCompressionVerbosity()
	{
		bool state = Compress::IsVerboseLoggingEnabled();
		state = !state;

		Compress::SetVerboseLoggingState(state);

		string stateStr = state ? "true" : "false";

		Core::PrintMessage(
			"Set compression verbose logging state to '" + stateStr + "'!\n",
			MessageType::MESSAGETYPE_SUCCESS);
	}

	void Command::Command_Compress(
		const string& origin,
		const string& target)
	{
		if (!exists(origin))
		{
			Core::PrintMessage(
				"Origin path '" + origin + "' does not exist!\n",
				MessageType::MESSAGETYPE_ERROR);

			return;
		}

		if (!is_directory(origin))
		{
			Core::PrintMessage(
				"Origin '" + origin + "' must be a directory!\n",
				MessageType::MESSAGETYPE_ERROR);

			return;
		}

		if (is_empty(origin))
		{
			Core::PrintMessage(
				"Origin '" + origin + "' must not be an empty folder!\n",
				MessageType::MESSAGETYPE_ERROR);

			return;
		}

		uint64_t originSize = GetFolderSize(origin);
		if (originSize > maxFolderSize)
		{
			string convertedOriginSize = ConvertSizeToString(originSize);

			Core::PrintMessage(
				"Origin '" + origin + "' size '" + convertedOriginSize + "' exceeds max allowed size '5.00GB'!\n",
				MessageType::MESSAGETYPE_ERROR);

			return;
		}

		if (exists(target))
		{
			Core::PrintMessage(
				"Target '" + target + "' already exists!\n",
				MessageType::MESSAGETYPE_ERROR);

			return;
		}

		if (path(target).extension().string() != ".kdat")
		{
			Core::PrintMessage(
				"Target path '" + target + "' must have the '.kdat' extension!\n",
				MessageType::MESSAGETYPE_ERROR);

			return;
		}

		string targetParentFolder = path(target).parent_path().string();
		if (!CanWriteToFolder(targetParentFolder))
		{
			Core::PrintMessage(
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
			Core::PrintMessage(
				"Origin path '" + origin + "' does not exist!\n",
				MessageType::MESSAGETYPE_ERROR);

			return;
		}

		if (!is_regular_file(origin))
		{
			Core::PrintMessage(
				"Origin '" + origin + "' must be a regular file!\n",
				MessageType::MESSAGETYPE_ERROR);

			return;
		}

		if (path(origin).extension().string() != ".kdat")
		{
			Core::PrintMessage(
				"Origin '" + origin + "' must have the '.kdat' extension!\n",
				MessageType::MESSAGETYPE_ERROR);

			return;
		}

		if (!exists(target))
		{
			Core::PrintMessage(
				"Target folder '" + target + "' does not exist!\n",
				MessageType::MESSAGETYPE_ERROR);

			return;
		}

		if (!is_directory(target))
		{
			Core::PrintMessage(
				"Target '" + target + "' must be a folder!\n",
				MessageType::MESSAGETYPE_ERROR);

			return;
		}

		string targetParentFolder = path(target).parent_path().string();
		if (!CanWriteToFolder(targetParentFolder))
		{
			Core::PrintMessage(
				"Insufficient permissions to write to target parent folder '" + targetParentFolder + "'!\n",
				MessageType::MESSAGETYPE_ERROR);

			return;
		}

		Compress::DecompressToFolder(origin, target);
	}

	void Command::Command_Exit()
	{
		Core::Shutdown();
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