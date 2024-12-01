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
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#endif

// define TLC_DEBUG
#ifdef NDEBUG
#define TLC_RELEASE
#else
#define TLC_DEBUG
#endif



// std includes
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <memory>
#include <functional>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <future>
#include <type_traits>
#include <typeinfo>
#include <utility>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cctype>
#include <cstring>
#include <ctime>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <filesystem>

// core includes
#include "core/UUID.hpp"
#include "core/Types.hpp"
#include "core/Logger.hpp"
#include "core/Utils.hpp"
#include "core/EventManager.hpp"

// glm
#pragma warning(disable : 4201)
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

// assert

#ifdef TLC_DEBUG
#define TLC_ASSERT(condition, message) { \
	if (!(condition)) { \
		log::Error("Assertion failed: {0} in {1} at {2}:{3}", message, __FUNCTION__, __FILE__, __LINE__); \
		__debugbreak(); \
	} \
}
#else
#define TLC_ASSERT(condition, message)
#endif


