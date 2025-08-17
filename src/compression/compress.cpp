//Copyright(C) 2025 Lost Empire Entertainment
//This program comes with ABSOLUTELY NO WARRANTY.
//This is free software, and you are welcome to redistribute it under certain conditions.
//Read LICENSE.md for more information.

#include <filesystem>
#include <fstream>
#include <vector>
#include <sstream>

#include "compression/compress.hpp"
#include "core/core.hpp"

using KalaData::Core::KalaDataCore;
using KalaData::Core::MessageType;

using std::filesystem::path;
using std::filesystem::create_directories;
using std::filesystem::relative;
using std::filesystem::is_regular_file;
using std::filesystem::recursive_directory_iterator;
using std::ofstream;
using std::ifstream;
using std::ios;
using std::streamoff;
using std::istreambuf_iterator;
using std::vector;
using std::stringstream;
using std::ostringstream;

constexpr size_t WINDOW_SIZE = 4096; //4KB sliding window
constexpr size_t LOOKAHEAD = 18;     //Max match length
constexpr size_t MIN_MATCH = 3;

struct Token
{
	bool isLiteral;
	uint8_t literal;
	uint16_t offset;
	uint8_t length;
};

static void CompressBuffer(const vector<uint8_t>&, ofstream& out);

static vector<uint8_t> DecompressBuffer(ifstream& in, size_t compressedSize);

namespace KalaData::Compression
{
	void Compress::CompressToArchive(
		const string& origin,
		const string& target)
	{
		KalaDataCore::PrintMessage(
			"Starting to compress folder '" + origin + "' to archive '" + target + "'!\n",
			MessageType::MESSAGETYPE_DEBUG);
	}

	void Compress::DecompressToFolder(
		const string& origin,
		const string& target)
	{
		KalaDataCore::PrintMessage(
			"Starting to decompress archive '" + origin + "' to folder '" + target + "'!\n",
			MessageType::MESSAGETYPE_DEBUG);
	}
}

void CompressBuffer(const vector<uint8_t>&, ofstream& out)
{

}

vector<uint8_t> DecompressBuffer(ifstream& in, size_t compressedSize)
{

}