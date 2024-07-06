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

		List<U8> ReadBinaryFie(const String& filepath)
		{
			List<U8> result;

            std::ifstream file;
			file.open(filepath, std::ios::binary | std::ios::ate);
			if (file.is_open())
			{
				std::streampos size = file.tellg();
				result.resize(size);
				file.seekg(0, std::ios::beg);
				file.read(reinterpret_cast<char*>(result.data()), size);
				file.close();
			}
			else
			{
				log::Error("Failed to open file '{}'", filepath);
			}
			return result;

		}

		String ReadTextFile(const String& filepath)
		{
			String result;
			std::ifstream file;
			file.open(filepath, std::ios::in);
			
			if (file.is_open())
			{
				file.seekg(0, std::ios::end);
				result.reserve(file.tellg());
				file.seekg(0, std::ios::beg);
				result.assign((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
				file.close();
			}
			else
			{
				log::Error("Failed to open file '{}'", filepath);
			}

			return result;
		}

		// TODO: Implement a better UUID generator (Probably use the OS to generate a UUID)
		UUID GenerateUUID()
		{
			static UUID uuid = 0;
			return ++uuid;
		}

	}
}