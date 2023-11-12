#include "core/Utils.hpp"
#include "core/Core.hpp"

namespace tlc
{
	namespace utils
	{
		String tlc::utils::GetExecutablePath()
		{
#if defined(PLATFORM_WINDOWS)
			static CHAR buffer[2048];
			::GetModuleFileNameA(NULL, buffer, 2048);
			return String(buffer);
#else
			static I8 buffer[2048];
			readlink("/proc/self/exe", buffer, 2048);
			return String(buffer);
#endif
		}

		String GetExecutableDirectory()
		{
			String path = GetExecutablePath();
			return path.substr(0, path.find_last_of("\\/"));
		}

	}
}