#pragma once

#include "core/Core.hpp"

// vulkan
#define API_VERSION VK_API_VERSION_1_4 
#define VULKAN_HPP_NO_EXCEPTIONS
#define VULKAN_HPP_ASSERT_ON_RESULT
#include "vulkan/vulkan.hpp"

#define VkCritCall(x) if (x != vk::Result::eSuccess) { tlc::log::Fatal("Vulkan call failed at {}:{}: {}", __FILE__, __LINE__, vk::to_string(x)); }

#define VkCall(x) if (x != vk::Result::eSuccess) { tlc::log::Error("Vulkan call failed at {}:{}: {}", __FILE__, __LINE__, vk::to_string(x)); }