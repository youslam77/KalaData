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
		static void HandleCommand(vector<string> parameters);
	};
}