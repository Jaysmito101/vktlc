#pragma once

#include "core/Core.hpp"

// vulkan
#include <vulkan/vulkan.hpp>

#define VkCritCall(x) if (x != vk::Result::eSuccess) { tlc::log::Fatal("Vulkan call failed at {}:{}: {}", __FILE__, __LINE__, vk::to_string(x)); }

#define VkCall(x) if (x != vk::Result::eSuccess) { tlc::log::Error("Vulkan call failed at {}:{}: {}", __FILE__, __LINE__, vk::to_string(x)); }