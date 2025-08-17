//Copyright(C) 2025 Lost Empire Entertainment
//This program comes with ABSOLUTELY NO WARRANTY.
//This is free software, and you are welcome to redistribute it under certain conditions.
//Read LICENSE.md for more information.

#include <sstream>
#include <string>
#include <filesystem>
#include <iomanip>
#include <fstream>

#include "core/command.hpp"
#include "core/core.hpp"

using KalaData::Core::KalaDataCore;
using KalaData::Core::ShutdownState;
using KalaData::Core::MessageType;

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

//
// COMMANDS
//

static void Command_Info();

static void Command_Help();

static void Command_Help_Command(const string& commandName);

static void Command_Compress(
	const string& origin,
	const string& target);

static void Command_Decompress(
	const string& origin,
	const string& target);

//
// UTILS
//

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
			&& (parameters[1] == "--v"
			|| parameters[1] == "--version"))
		{
			KalaDataCore::PrintMessage("KalaData version: 0.0.1 Alpha\n");
		}

		else if (parameters.size() == 2
			&& parameters[1] == "--info")
		{
			Command_Info();
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

		else if (parameters.size() == 4
			&& (parameters[1] == "--c"
			|| parameters[1] == "--compress"))
		{
			Command_Compress(parameters[2], parameters[3]);
		}

		else if (parameters.size() == 4
			&& (parameters[1] == "--dc"
			|| parameters[1] == "--decompress"))
		{
			Command_Decompress(parameters[2], parameters[3]);
		}

		else if (parameters.size() == 2
			&& parameters[1] == "--exit")
		{
			KalaDataCore::Shutdown();
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
}

//
// COMMANDS
//

void Command_Info()
{
	stringstream ss{};
	ss << "KalaData is a lightweight C++ 20 data compressor and decompressor for the '.kdat' format. "
		<< "KDat is aimed for games but can be used for regular software too.\n";

	KalaDataCore::PrintMessage(ss.str());
}

void Command_Help()
{
	stringstream ss{};
	ss << "====================\n\n"

		<< "Notes:\n"
		<< "  - each command starts with the program name 'KalaData.exe' as its first parameter\n"
		<< "  - all paths are absolute and KalaData does not take relative paths.\n\n"

		<< "Commands:\n"
		<< "  --version                  - prints the KalaData version\n"
		<< "  --v                        - same as above\n"
		<< "  --info                     - prints general info about KalaData\n"
		<< "  --help                     - lists all commands\n"
		<< "  --help x                   - prints additional info about selected command 'x'\n"
		<< "  --compress origin target   - compresses folder 'origin' into a '.kdat' archive into folder 'target'\n"
		<< "  --c origin target          - same as above\n"
		<< "  --decompress origin target - decompresses file 'origin' into folder 'target'\n"
		<< "  --dc origin target         - same as above\n"
		<< "  --exit                     - shuts down KalaData\n\n"

		<< "====================\n";

	KalaDataCore::PrintMessage(ss.str());
}

void Command_Help_Command(const string& commandName)
{
	if (commandName == "version"
		|| commandName == "--version"
		|| commandName == "v"
		|| commandName == "--v")
	{
		KalaDataCore::PrintMessage("Prints the KalaData version\n");
	}

	else if (commandName == "info"
		|| commandName == "--info")
	{
		KalaDataCore::PrintMessage("Prints general info about KalaData\n");
	}

	else if (commandName == "help"
		|| commandName == "--help")
	{
		KalaDataCore::PrintMessage("Lists all commands\n");
	}

	else if (commandName == "compress"
		|| commandName == "--compress"
		|| commandName == "c"
		|| commandName == "--c")
	{
		stringstream ss{};

		ss << "Takes in a folder which will be compressed into a '.kdat' file inside the target folder.\n\n"
			<< "Requirements and restrictions:\n"
			<< "  - Origin path must exist\n"
			<< "  - Origin must be a folder\n"
			<< "  - Origin folder must not be empty\n"
			<< "  - Origin folder size must not exceed 5GB\n"
			<< "  - Target path must exist\n"
			<< "  - Target path must be a folder\n"
			<< "  - Target folder root must not contain file with the same name as Origin folder\n"
			<< "  - Target folder must be writable\n";

		KalaDataCore::PrintMessage(ss.str());
	}

	else if (commandName == "decompress"
		|| commandName == "--decompress"
		|| commandName == "dc"
		|| commandName == "--dc")
	{
		stringstream ss{};

		ss << "Takes in a folder which will be compressed into a '.kdat' file inside the target folder.\n\n"
			<< "Requirements and restrictions:\n"
			<< "  - Origin must exist\n"
			<< "  - Origin must be a regular file\n"
			<< "  -	Origin must have the '.kdat' extension\n"
			<< "  - Target path must exist\n"
			<< "  - Target path must be a folder"
			<< "  - Target folder root must not contain file with the same name as Origin\n"
			<< "  - Target folder must be writable\n";

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

void Command_Compress(
	const string& origin,
	const string& target)
{
	if (!exists(origin))
	{
		KalaDataCore::PrintMessage(
			"Origin '" + origin + "' does not exist!\n",
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
			"Target '" + target + "' must be a directory!\n",
			MessageType::MESSAGETYPE_ERROR);

		return;
	}

	string convertedName = path(origin).stem().string() + ".kdat";

	if (exists(path(target) / convertedName))
	{
		stringstream ss{};
		ss << "Cannot pack target '" + convertedName + "' because a file "
			<< "with the same name already exists in the target folder '" + target + "'\n";

		KalaDataCore::PrintMessage(
			ss.str(),
			MessageType::MESSAGETYPE_ERROR);
		return;
	}

	if (!CanWriteToFolder(target))
	{
		KalaDataCore::PrintMessage(
			"Insufficient permissions to write to target folder '" + target + "'!\n",
			MessageType::MESSAGETYPE_ERROR);

		return;
	}

	KalaDataCore::PrintMessage(
		"Ready to compress folder '" + origin + "' to folder '" + target + "'!\n",
		MessageType::MESSAGETYPE_SUCCESS);
}

void Command_Decompress(
	const string& origin,
	const string& target)
{
	if (!exists(origin))
	{
		KalaDataCore::PrintMessage(
			"Origin '" + origin + "' does not exist!\n",
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
			"Target '" + target + "' must be a directory!\n",
			MessageType::MESSAGETYPE_ERROR);

		return;
	}

	string convertedName = path(origin).stem().string();

	if (exists(path(target) / convertedName))
	{
		stringstream ss{};
		ss << "Cannot unpack origin file '" + convertedName + "' because a file "
			<< "with the same name already exists in the target folder '" + target + "'\n";

		KalaDataCore::PrintMessage(
			ss.str(),
			MessageType::MESSAGETYPE_ERROR);
		return;
	}

	if (!CanWriteToFolder(target))
	{
		KalaDataCore::PrintMessage(
			"Insufficient permissions to write to target folder '" + target + "'!\n",
			MessageType::MESSAGETYPE_ERROR);

		return;
	}

	KalaDataCore::PrintMessage(
		"Ready to decompress archive '" + convertedName + "' to folder '" + target + "'!\n",
		MessageType::MESSAGETYPE_SUCCESS);
}

//
// UTILS
//

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