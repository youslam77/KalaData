//Copyright(C) 2025 Lost Empire Entertainment
//This program comes with ABSOLUTELY NO WARRANTY.
//This is free software, and you are welcome to redistribute it under certain conditions.
//Read LICENSE.md for more information.

#include <string>
#include <sstream>
#include <filesystem>
#include <iomanip>

#include "core/core.hpp"

using KalaData::Core::KalaDataCore;
using KalaData::Core::MessageType;
using KalaData::Core::ShutdownState;

using std::string;
using std::to_string;
using std::stringstream;
using std::filesystem::path;
using std::filesystem::exists;
using std::filesystem::is_regular_file;
using std::filesystem::is_directory;
using std::filesystem::file_size;
using std::filesystem::is_empty;
using std::filesystem::recursive_directory_iterator;
using std::fixed;
using std::setprecision;

static void IncorrectUsageError(const string& message);

static uint64_t GetFolderSize(const string& folderPath);

static string ConvertSizeToString(uint64_t size);

//5GB max file size
constexpr uint64_t maxFolderSize = 5ull * 1024 * 1024 * 1024;

int main(int argc, char* argv[])
{
	if (argc < 3)
	{
		stringstream ss{};
		ss << "Cannot have less than 3 parameters! "
			<< "First parameter must always be 'KalaData.exe', "
			<< "second parameter must be origin folder "
			<< "and third parameter must be target folder!";

		IncorrectUsageError(ss.str());
		return 1;
	}

	string origin = argv[1];
	string target = argv[2];

	if (!exists(origin))
	{
		IncorrectUsageError("Origin '" + origin + "' does not exist!");
		return 1;
	}

	if (!is_directory(origin))
	{
		IncorrectUsageError("Origin '" + origin + "' must be a directory!");
		return 1;
	}

	if (is_empty(origin))
	{
		IncorrectUsageError("Origin '" + origin + "' must not be an empty folder!");
		return 1;
	}

	uint64_t originSize = GetFolderSize(origin);
	if (originSize > maxFolderSize)
	{
		string convertedOriginSize = ConvertSizeToString(originSize);

		IncorrectUsageError("Origin '" + origin + "' size '" + convertedOriginSize + "' exceeds max allowed size '5.00GB'!");
		return 1;
	}

	if (!exists(target))
	{
		IncorrectUsageError("Target folder '" + target + "' does not exist!");
		return 1;
	}

	if (!is_directory(target))
	{
		IncorrectUsageError("Target '" + target + "' must be a directory!");
		return 1;
	}

	string convertedName = path(origin).stem().string() + ".kdat";

	if (exists(path(target) / convertedName))
	{
		stringstream ss{};
		ss << "Cannot pack target '" + convertedName + "' because a file"
			<< " with the same name already exists in the target folder '" + target + "'";

		IncorrectUsageError(ss.str());
		return 1;
	}

	KalaDataCore::Start(origin, target);
}

void IncorrectUsageError(const string& message)
{
	KalaDataCore::PrintMessage(message);
	KalaDataCore::Shutdown(ShutdownState::SHUTDOWN_CRITICAL);
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

string ConvertSizeToString(uint64_t size)
{
	stringstream ss{};
	ss << fixed
		<< setprecision(2)
		<< static_cast<double>(size) / (1024ull * 1024 * 1024)
		<< "GB";

	return ss.str();
}