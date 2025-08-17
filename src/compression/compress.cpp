//Copyright(C) 2025 Lost Empire Entertainment
//This program comes with ABSOLUTELY NO WARRANTY.
//This is free software, and you are welcome to redistribute it under certain conditions.
//Read LICENSE.md for more information.

#include <filesystem>
#include <fstream>
#include <vector>
#include <sstream>
#include <string>

#include "compression/compress.hpp"
#include "core/core.hpp"

using KalaData::Core::KalaDataCore;
using KalaData::Core::MessageType;

using std::filesystem::path;
using std::filesystem::create_directories;
using std::filesystem::relative;
using std::filesystem::is_regular_file;
using std::filesystem::weakly_canonical;
using std::filesystem::recursive_directory_iterator;
using std::ofstream;
using std::ifstream;
using std::ios;
using std::streamoff;
using std::istreambuf_iterator;
using std::vector;
using std::stringstream;
using std::ostringstream;
using std::string;
using std::to_string;

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

static void ForceClose(
	const string& message,
	bool type);

//Compress a single buffer into an already open stream
static vector<uint8_t> CompressBuffer(
	const vector<uint8_t>& input,
	const string& origin);

//Decompress from an already open stream into a buffer
static void DecompressBuffer(
	ifstream& in,
	vector<uint8_t> &out,
	size_t compressedSize,
	size_t originalSize,
	const string& target);

namespace KalaData::Compression
{
	void Compress::CompressToArchive(
		const string& origin,
		const string& target)
	{
		KalaDataCore::PrintMessage(
			"Starting to compress folder '" + origin + "' to archive '" + target + "'!\n",
			MessageType::MESSAGETYPE_DEBUG);

		ofstream out(target, ios::binary);
		if (!out.is_open())
		{
			ForceClose(
				"Failed to open target archive '" + target + "'!\n",
				true);

			return;
		}

		//collect all files
		vector<path> files{};
		for (auto& p : recursive_directory_iterator(origin))
		{
			if (is_regular_file(p)) files.push_back(p.path());
		}

		if (files.empty())
		{
			ForceClose(
				"Origin folder '" + origin + "' contains no valid files to compress!\n",
				true);

			return;
		}

		uint32_t fileCount = (uint32_t)files.size();
		out.write((char*)&fileCount, sizeof(uint32_t));
		if (!out.good())
		{
			ForceClose(
				"Write failure while building archive '" + target + "'!\n",
				true);

			return;
		}

		for (auto& file : files)
		{
			//relative path
			string relPath = relative(file, origin).string();
			uint32_t pathLen = (uint32_t)relPath.size();

			//read file into memory
			ifstream in(file, ios::binary);
			vector<uint8_t> raw((istreambuf_iterator<char>(in)), {});
			in.close();

			//compress directly into memory
			vector<uint8_t> compData = CompressBuffer(raw, relPath);

			uint64_t originalSize = raw.size();
			uint64_t compressedSize = compData.size();

			//safeguard: if compression is bigger or equal than original then store raw instead
			bool useCompressed = compressedSize < originalSize;
			const vector<uint8_t>& finalData = useCompressed ? compData : raw;
			uint64_t finalSize = useCompressed ? compressedSize : originalSize;

			if (!useCompressed)
			{
				stringstream ss{};
				ss << "Skipping storing compressed data for relative path '" + relPath + "' and storing as raw "
					<< "because compressed size '" + to_string(compressedSize) + "' is not smaller than original size '" + to_string(originalSize) + "'!\n";

				KalaDataCore::PrintMessage(
					ss.str(),
					MessageType::MESSAGETYPE_WARNING);
			}

			//write metadata
			out.write((char*)&pathLen, sizeof(uint32_t));
			if (!out.good())
			{
				ForceClose(
					"Path length write failure while building archive '" + target + "'!\n",
					true);

				return;
			}

			out.write(relPath.data(), pathLen);
			if (!out.good())
			{
				ForceClose(
					"Relative path write failure while building archive '" + target + "'!\n",
					true);

				return;
			}

			out.write((char*)&originalSize, sizeof(uint64_t));
			if (!out.good())
			{
				ForceClose(
					"Original size write failure while building archive '" + target + "'!\n",
					true);

				return;
			}

			out.write((char*)&finalSize, sizeof(uint64_t));
			if (!out.good())
			{
				ForceClose(
					"Final size write failure while building archive '" + target + "'!\n",
					true);

				return;
			}

			//write compressed data if it is more than 0 bytes
			if (finalSize > 0)
			{
				out.write((char*)finalData.data(), finalData.size());
				if (!out.good())
				{
					ForceClose(
						"Final data write failure while building archive '" + target + "'!\n",
						true);

					return;
				}
			}
			else
			{
				KalaDataCore::PrintMessage(
					"File '" + relPath + "' is empty, storing as 0-byte entry in archive.\n",
					MessageType::MESSAGETYPE_WARNING);
			}
		}

		KalaDataCore::PrintMessage(
			"Finished compressing folder '" + origin + "' to archive '" + target + "'!\n",
			MessageType::MESSAGETYPE_SUCCESS);
	}

	void Compress::DecompressToFolder(
		const string& origin,
		const string& target)
	{
		KalaDataCore::PrintMessage(
			"Starting to decompress archive '" + origin + "' to folder '" + target + "'!\n",
			MessageType::MESSAGETYPE_DEBUG);

		ifstream in(origin, ios::binary);
		if (!in.is_open())
		{
			ForceClose(
				"Failed to open origin archive '" + origin + "'!\n",
				false);

			return;
		}

		uint32_t fileCount{};
		in.read((char*)&fileCount, sizeof(uint32_t));
		if (fileCount > 100000)
		{
			ForceClose(
				"Archive '" + origin + "' reports an absurd file count (corrupted?)!\n",
				false);

			return;
		}

		if (fileCount == 0)
		{
			ForceClose(
				"Archive '" + origin + "' contains no valid files to decompress!\n",
				true);

			return;
		}

		for (uint32_t i = 0; i < fileCount; i++)
		{
			uint32_t pathLen{};
			if (!in.read((char*)&pathLen, sizeof(uint32_t)))
			{
				ForceClose(
					"Failed to read path length from archive '" + origin + "'!\n",
					false);

				return;
			}

			string relPath(pathLen, '\0');
			if (!in.read(relPath.data(), pathLen))
			{
				ForceClose(
					"Failed to read relative path of length '" + to_string(pathLen) + "' from archive '" + origin + "'!\n",
					false);

				return;
			}

			uint64_t originalSize{};
			uint64_t compressedSize{};
			if (!in.read((char*)&originalSize, sizeof(uint64_t)))
			{
				ForceClose(
					"Failed to read original size for file '" + relPath + "' from archive '" + origin + "'!\n",
					false);

				return;
			}

			if (!in.read((char*)&compressedSize, sizeof(uint64_t)))
			{
				ForceClose(
					"Failed to read compressed size for file '" + relPath + "' from archive '" + origin + "'!\n",
					false);

				return;
			}

			path outPath = path(target) / relPath;
			create_directories(outPath.parent_path());

			//path traversal check
			auto absTarget = weakly_canonical(target);
			auto absOut = weakly_canonical(outPath);

			if (absOut.string().find(absTarget.string()) != 0)
			{
				ForceClose(
					"Archive '" + origin + "' contains invalid path '" + relPath + "' (path traveral attempt)!",
					false);

				return;
			}

			//prepare output buffer
			vector<uint8_t> data{};
			auto compressedStart = in.tellg();

			//decompress
			DecompressBuffer(
				in,
				data,
				compressedSize,
				originalSize,
				origin);

			//sanity check
			if (data.size() != originalSize)
			{
				stringstream ss{};
				ss << "Decompressed archive file '" << target << "' size '" << to_string(data.size())
					<< "does not match original size '" << to_string(originalSize) + "'!\n";

				ForceClose(
					ss.str(),
					false);

				return;
			}

			//write file
			ofstream outFile(outPath, ios::binary);
			outFile.write((char*)data.data(), data.size());
			if (!outFile.good())
			{
				ForceClose(
					"Write failure while decompiling archive '" + origin + "'!\n",
					false);
				return;
			}

			outFile.close();

			//advance past compressed chunk in archive
			in.seekg(compressedStart);
			in.seekg((streamoff)compressedSize, ios::cur);
		}

		KalaDataCore::PrintMessage(
			"Finished decompressing archive '" + origin + "' to folder '" + target + "'!\n",
			MessageType::MESSAGETYPE_SUCCESS);
	}
}

void ForceClose(
	const string& message,
	bool type)
{
	string title = type 
		? "Compression error"
		: "Decompression error";

	KalaDataCore::ForceClose(title, message);
}

vector<uint8_t> CompressBuffer(
	const vector<uint8_t>& input,
	const string& origin)
{
	vector<uint8_t> output{};

	if (input.empty()) return output;

	size_t pos = 0;

	while (pos < input.size())
	{
		size_t bestLength = 0;
		size_t bestOffset = 0;
		size_t start = (pos > WINDOW_SIZE) ? (pos - WINDOW_SIZE) : 0;

		//search backwards in window
		for (size_t i = start; i < pos; i++)
		{
			size_t length = 0;

			while (length < LOOKAHEAD
				&& pos + length < input.size()
				&& input[i + length] == input[pos + length])
			{
				length++;
			}

			if (length > bestLength
				&& length >= MIN_MATCH)
			{
				bestLength = length;
				bestOffset = pos - i;
			}
		}

		if (bestLength >= MIN_MATCH)
		{
			if (bestOffset >= UINT16_MAX)
			{
				ForceClose(
					"Offset too large for file '" + origin + "' during compressing (data window exceeded)!\n",
					true);

				return {};
			}

			uint8_t flag = 0;
			output.push_back(flag);

			uint16_t offset16 = (uint16_t)bestOffset;
			output.insert(output.end(),
				reinterpret_cast<uint8_t*>(&offset16),
				reinterpret_cast<uint8_t*>(&offset16) + sizeof(uint16_t));

			if (bestLength > UINT8_MAX)
			{
				ForceClose(
					"Match length too large for file '" + origin + "' during compressing (overflow)!\n",
					true);

				return {};
			}

			uint8_t len8 = (uint8_t)bestLength;
			output.push_back(len8);

			pos += bestLength;
		}
		else
		{
			uint8_t flag = 1;
			output.push_back(flag);
			output.push_back(input[pos]);
			pos++;
		}
	}

	if (output.empty())
	{
		ForceClose(
			"Compression produced empty output for file '" + origin + "' (unexpected)!\n",
			true);
	}

	return output;
}

void DecompressBuffer(
	ifstream& in,
	vector<uint8_t>& out,
	size_t compressedSize,
	size_t originalSize,
	const string& target)
{
	vector<uint8_t> buffer{};
	buffer.reserve(originalSize);

	size_t bytesRead = 0;

	while (bytesRead < compressedSize)
	{
		uint8_t flag{};
		if (!in.read((char*)&flag, 1))
		{
			ForceClose(
				"Unexpected end of archive while reading flag in '" + target + "'!\n",
				false);

			return;
		}
		bytesRead += 1;

		if (flag == 1) //literal
		{
			uint8_t c{};
			if (!in.read((char*)&c, 1))
			{
				ForceClose(
					"Unexpected end of archive while reading literal in '" + target + "'!\n",
					false);

				return;
			}
			bytesRead += 1;
			buffer.push_back(c);
		}
		else //reference
		{
			uint16_t offset{};
			uint8_t length{};

			if (!in.read((char*)&offset, sizeof(uint16_t))
				|| !in.read((char*)&length, sizeof(uint8_t)))
			{
				ForceClose(
					"Unexpected end of archive while reading reference in '" + target + "'!\n",
					false);

				return;
			}
			bytesRead += sizeof(uint16_t) + sizeof(uint8_t);

			if (offset == 0
				|| offset > buffer.size())
			{
				ForceClose(
					"Invalid offset in archive '" + target + "' (corruption suspected)!\n",
					false);

				return;
			}

			size_t start = buffer.size() - offset;
			for (size_t i = 0; i < length; i++)
			{
				if (buffer.size() >= originalSize)
				{
					stringstream ss{};

					ss << "Decompressed size '" + to_string(buffer.size()) << "' "
						<< "exceeds expected size '" + to_string(originalSize) << "' "
						<< "while reading archive '" + target + "'!\n";

					ForceClose(
						ss.str(),
						false);

					return;
				}
				buffer.push_back(buffer[start + i]);
			}
		}
	}

	if (buffer.size() != originalSize) 
	{
		stringstream ss{};
		ss << "Decompressed size '" << to_string(buffer.size())
			<< "' does not match expected size '" << to_string(originalSize)
			<< "' for archive '" << target << "' (possible corruption)!\n";

		ForceClose(
			ss.str(),
			false);
	}

	//hand decompressed data back to caller
	out = move(buffer);
}