#include "globalVulkanVars.hpp"

VkDevice device;

VkQueue graphicsQueue;
VkQueue presentQueue;

QueueFamilyIndicies deviceQueueFamilyIndices;

VmaAllocator allocator;

VkBuffer stagingBuffer;
VmaAllocation stagingAllocation;
VkBuffer vertexBuffer;
VmaAllocation vertexAllocation;
VkBuffer indexBuffer;
VmaAllocation indexAllocation;