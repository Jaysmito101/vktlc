#pragma once

#if defined(_WIN32) || defined(_WIN64)
#define PLATFORM_WINDOWS
#elif defined(__unix__) || defined(__unix) || defined(__linux__)
#define PLATFORM_LINUX
#elif defined(__APPLE__) || defined(__MACH__)
#error "Apple platforms are not supported!"
#endif

#if defined(PLATFORM_WINDOWS)
// windows includes
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

#elif defined(PLATFORM_LINUX)
// linux includes
#include <sys/ioctl.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

// define TLC_DEBUG
#ifdef NDEBUG
#define TLC_RELEASE
#else
#define TLC_DEBUG
#endif

// std includes
#include <algorithm>
#include <atomic>
#include <cassert>
#include <cctype>
#include <chrono>
#include <cmath>
#include <condition_variable>
#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <functional>
#include <future>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <ranges>
#include <sstream>
#include <stack>
#include <string>
#include <thread>
#include <type_traits>
#include <typeinfo>
#include <unordered_map>
#include <utility>
#include <vector>

// core includes
#include "core/EventManager.hpp"
#include "core/Logger.hpp"
#include "core/Types.hpp"
#include "core/UUID.hpp"
#include "core/Utils.hpp"

// glm
#pragma warning(disable : 4201)
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

// assert

#ifdef TLC_DEBUG
#define TLC_ASSERT(condition, message)                                                                        \
    {                                                                                                         \
        if (!(condition)) {                                                                                   \
            log::Error("Assertion failed: {0} in {1} at {2}:{3}", message, __FUNCTION__, __FILE__, __LINE__); \
            __debugbreak();                                                                                   \
        }                                                                                                     \
    }
#else
#define TLC_ASSERT(condition, message)
#endif
