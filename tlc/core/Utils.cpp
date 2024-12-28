#include "core/Utils.hpp"
#include "core/Core.hpp"

#if (defined(__GNUC__) && defined(__i386__)) || defined(__WATCOMC__) \
  || defined(_MSC_VER) || defined (__BORLANDC__) || defined (__TURBOC__)
#define M_get16bits(d) (*((const uint16_t *) (d)))
#endif

#if !defined (M_get16bits)
#define M_get16bits(d) ((((uint32_t)(((const uint8_t *)(d))[1])) << 8)\
                       +(uint32_t)(((const uint8_t *)(d))[0]) )
#endif

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

		void EnsureDirectory(const String& path) 
		{
			if(!std::filesystem::exists(path)) {
				std::filesystem::create_directories(path);
			}
		}

		List<U8> ReadBinaryFile(const String& filepath)
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

		List<U8> ReadBinaryFilePortion(const String& filepath, Size offset, Size size)
		{
			List<U8> result;

			std::ifstream file;
			file.open(filepath, std::ios::binary | std::ios::ate);
			if (file.is_open())
			{
				file.seekg(offset, std::ios::beg);
				result.resize(size);
				file.read(reinterpret_cast<char*>(result.data()), size);
				file.close();
			}
			else
			{
				log::Error("Failed to open file '{}'", filepath);
			}

			TLC_ASSERT(result.size() == size, "Failed to read the requested portion of the file");

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

		Bool PathExists(const String& path)
		{
			return std::filesystem::exists(path);
		}

		Bool IsDirectory(const String& path)
		{
			return std::filesystem::is_directory(path);
		}

		Bool MakeDirectory(const String& path)
		{
			if (!PathExists(path))
			{
				return std::filesystem::create_directory(path);
			}
			return true;
		}

		Bool RemoveDirectory(const String& path)
		{
			if (PathExists(path))
			{
				return std::filesystem::remove_all(path);
			}
			return true;
		}

		Bool RemoveFile(const String& path)
		{
			if (PathExists(path))
			{
				return std::filesystem::remove(path);
			}
			return true;
		}

		Size GetFileSize(const String& filepath)
		{
			std::ifstream file;
			file.open(filepath, std::ios::binary | std::ios::ate);
			if (file.is_open())
			{
				return file.tellg();
			}
			else
			{
				log::Error("Failed to open file '{}'", filepath);
			}
			return 0;
		}

		U32 HashBuffer(const void* dat, Size len)
		{
			const U8* data = static_cast<const U8*>(dat);
			U32 hash = static_cast<U32>(len);
			U32 tmp = 0;
			I32 rem = 0;
			if (len <= 0 || data == NULL) return 0;

			rem = len & 3;
			len >>= 2;

			/* Main loop */
			for (; len > 0; len--) {
				hash += M_get16bits(data);
				tmp = (M_get16bits(data + 2) << 11) ^ hash;
				hash = (hash << 16) ^ tmp;
				data += 2 * sizeof(U16);
				hash += hash >> 11;
			}

			/* Handle end cases */
			switch (rem) {
			case 3: hash += M_get16bits(data);
				hash ^= hash << 16;
				hash ^= ((signed char)data[sizeof(U16)]) << 18;
				hash += hash >> 11;
				break;
			case 2: hash += M_get16bits(data);
				hash ^= hash << 11;
				hash += hash >> 17;
				break;
			case 1: hash += (signed char)*data;
				hash ^= hash << 10;
				hash += hash >> 1;
			}

			/* Force "avalanching" of final 127 bits */
			hash ^= hash << 3;
			hash += hash >> 5;
			hash ^= hash << 4;
			hash += hash >> 17;
			hash ^= hash << 25;
			hash += hash >> 6;

			return hash;
		}

		U32 HashBuffer(const List<U8>& buffer)
		{ 
			return HashBuffer(buffer.data(), buffer.size());
		}
	
		List<String> SplitString(const String& str, const String& delimiter)
		{
			List<String> result;
			Size start = 0;
			Size end = str.find(delimiter);
			while (end != String::npos)
			{
				result.push_back(str.substr(start, end - start));
				start = end + delimiter.size();
				end = str.find(delimiter, start);
			}
			result.push_back(str.substr(start, end));
			return result;
		}

	}
}