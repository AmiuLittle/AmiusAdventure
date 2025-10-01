#pragma once
#ifndef PC_GLOBAL_VULKAN_VARS
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <optional>

extern VkDevice device;

extern VkQueue graphicsQueue;
extern VkQueue presentQueue;

struct QueueFamilyIndicies {
    std::optional<uint32_t> graphicsFamily = std::nullopt;
    std::optional<uint32_t> presentFamily = std::nullopt;
};
extern QueueFamilyIndicies deviceQueueFamilyIndices;

extern VmaAllocator allocator;

extern VkBuffer stagingBuffer;
extern VmaAllocation stagingAllocation;
extern VkBuffer vertexBuffer;
extern VmaAllocation vertexAllocation;
extern VkBuffer indexBuffer;
extern VmaAllocation indexAllocation;

#define PC_GLOBAL_VULKAN_VARS
#endif