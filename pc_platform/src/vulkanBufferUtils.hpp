#pragma once
#ifndef PC_VULKAN_BUFFER_UTILS
#include <stdint.h>
#include <vulkan/vulkan.h>
#include <utility>

uint32_t getNextAvailableVertexSpace(uint32_t size);
void freeVertexSpaceAt(uint32_t offset);
uint32_t getNextAvailableIndexSpace(uint32_t size);
void freeIndexSpaceAt(uint32_t offset);

bool startOneTimeCmdBuffer(std::pair<VkCommandPool, VkCommandBuffer>* output);
bool endOneTimeCmdBuffer(std::pair<VkCommandPool, VkCommandBuffer>* pair);
bool copyBuffer(VkBuffer src, uint32_t srcOffset, VkBuffer dst, uint32_t dstOffset, VkDeviceSize size);

#define PC_VULKAN_BUFFER_UTILS
#endif