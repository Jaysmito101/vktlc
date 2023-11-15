#pragma once
#include "core/Types.hpp"

namespace tlc
{
	namespace utils
	{
		String GetExecutablePath();
		String GetExecutableDirectory();

		List<U8> ReadBinaryFie(const String& filepath);
	}
}