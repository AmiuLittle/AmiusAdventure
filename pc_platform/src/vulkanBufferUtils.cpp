#include "vulkanBufferUtils.hpp"
#include <map>
#include <iterator>
#include "error.hpp"
#include "globalVulkanVars.hpp"

std::map<uint32_t, uint32_t> vertexAllocations = {};

uint32_t getNextAvailableVertexSpace(uint32_t size) {
    if (vertexAllocations.empty()) {
        vertexAllocations.insert(std::pair(0, size));
        return 0;
    }
    uint32_t endIdx = 0;
    for (auto allocation = vertexAllocations.begin(); allocation != vertexAllocations.end(); ++allocation) {
        endIdx = allocation->first + allocation->second;
        if (std::next(allocation) == vertexAllocations.end()) { // last allocation
            vertexAllocations.insert(std::pair(endIdx, size));
            break;
        }
        if (std::next(allocation)->first - (allocation->first + allocation->second) > size) { // is the space between this allocation and the next allocation big enough to support this new allocation?
            vertexAllocations.insert(std::pair(endIdx, size));
            break;
        }
    }
    return endIdx;
}

void freeVertexSpaceAt(uint32_t offset) {
    if (vertexAllocations.find(offset) != vertexAllocations.end()) {
        vertexAllocations.erase(offset);
    }
}

std::map<uint32_t, uint32_t> indexAllocations = {};

uint32_t getNextAvailableIndexSpace(uint32_t size) {
    if (indexAllocations.empty()) {
        indexAllocations.insert(std::pair(0, size));
        return 0;
    }
    uint32_t endIdx = 0;
    for (auto allocation = indexAllocations.begin(); allocation != indexAllocations.end(); ++allocation) {
        endIdx = allocation->first + allocation->second;
        if (std::next(allocation) == indexAllocations.end()) { // last allocation
            indexAllocations.insert(std::pair(endIdx, size));
            break;
        }
        if (std::next(allocation)->first - (allocation->first + allocation->second) > size) { // is the space between this allocation and the next allocation big enough to support this new allocation?
            indexAllocations.insert(std::pair(endIdx, size));
            break;
        }
    }
    return endIdx;
}

void freeIndexSpaceAt(uint32_t offset) {
    if (indexAllocations.find(offset) != indexAllocations.end()) {
        indexAllocations.erase(offset);
    }
}


bool startOneTimeCmdBuffer(std::pair<VkCommandPool, VkCommandBuffer>* output) {
    VkCommandPool tempCommandPool;
    VkCommandBuffer tempCommandBuffer;
    VkCommandPoolCreateInfo commandPoolInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = deviceQueueFamilyIndices.graphicsFamily.value()
    };
    if (vkCreateCommandPool(device, &commandPoolInfo, nullptr, &tempCommandPool) != VK_SUCCESS) {
        setErr("Failed to create command pool");
        return false;
    }

    VkCommandBufferAllocateInfo commandBufferAllocInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = tempCommandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1
    };
    if (vkAllocateCommandBuffers(device, &commandBufferAllocInfo, &tempCommandBuffer) != VK_SUCCESS) {
        setErr("Failed to allocate command buffer");
        return false;
    }

    VkCommandBufferBeginInfo beginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    };

    if (vkBeginCommandBuffer(tempCommandBuffer, &beginInfo) != VK_SUCCESS) {
        setErr("Failed to begin command buffer");
        return false;
    }

    *output = std::pair(tempCommandPool, tempCommandBuffer);
    return true;
}

bool endOneTimeCmdBuffer(std::pair<VkCommandPool, VkCommandBuffer>* pair) {
    if (vkEndCommandBuffer(pair->second) != VK_SUCCESS) {
        setErr("Failed to end command buffer");
    }

    VkSubmitInfo submitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &pair->second
    };
    vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphicsQueue);

    vkFreeCommandBuffers(device, pair->first, 1, &pair->second);
    vkDestroyCommandPool(device, pair->first, nullptr);

    return true;
}

bool copyBuffer(VkBuffer src, uint32_t srcOffset, VkBuffer dst, uint32_t dstOffset, VkDeviceSize size) {
    std::pair<VkCommandPool, VkCommandBuffer> pair;
    if (!startOneTimeCmdBuffer(&pair)) {
        return false;
    }

    VkBufferCopy copy = {
        .srcOffset = srcOffset,
        .dstOffset = dstOffset,
        .size = size
    };
    vkCmdCopyBuffer(pair.second, src, dst, 1, &copy);

    if (!endOneTimeCmdBuffer(&pair)) {
        return false;
    }

    return true;
}