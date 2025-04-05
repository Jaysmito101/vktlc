#pragma once

#include "core/Core.hpp"

namespace tlc {
    namespace utils {
        F32 LevenshteinMatch(const String& str1, const String& str2);
        F32 LevenshteinSubstringMatch(const String& main, const String& searchTerm);
    }
}