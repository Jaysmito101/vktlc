#pragma once

#include "core/Core.hpp"

// vulkan
#define API_VERSION VK_API_VERSION_1_4
#define VULKAN_HPP_NO_EXCEPTIONS
#define VULKAN_HPP_ASSERT_ON_RESULT
// if define this as a system header foer different compilers
#if defined(__clang__)
#pragma clang system_header
#elif defined(__GNUC__) || defined(__GNUG__)
#pragma GCC system_header
#elif defined(_MSC_VER)
#pragma warning(push, 0)
#endif

#include "vulkan/vulkan.hpp"

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

#define VkCritCall(x)                                                                             \
    if (x != vk::Result::eSuccess) {                                                              \
        tlc::log::Fatal("Vulkan call failed at {}:{}: {}", __FILE__, __LINE__, vk::to_string(x)); \
    }

#define VkCall(x)                                                                                 \
    if (x != vk::Result::eSuccess) {                                                              \
        tlc::log::Error("Vulkan call failed at {}:{}: {}", __FILE__, __LINE__, vk::to_string(x)); \
    }