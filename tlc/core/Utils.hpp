#pragma once
#include "core/Types.hpp"
#include "core/Uuid.hpp"

namespace tlc
{
	namespace utils
	{
		String GetExecutablePath();
		String GetExecutableDirectory();

		void EnsureDirectory(const String& path);

		List<U8> ReadBinaryFile(const String& filepath);
		List<U8> ReadBinaryFilePortion(const String& filepath, Size offset, Size size);

		String ReadTextFile(const String& filepath);	
		Bool PathExists(const String& path);
		Bool IsDirectory(const String& path);
		Bool MakeDirectory(const String& path);
		Bool RemoveDirectory(const String& path);
		Bool RemoveFile(const String& path);

		Size GetFileSize(const String& filepath);

		U32 HashBuffer(const void* buffer, Size size);

		U32 HashBuffer(const List<U8>& buffer);
		List<String> SplitString(const String& str, const String& delimiter);
	}
}