#include "render.hpp"
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <vulkan/vulkan.h>
#include <optional>
#include <vector>
#include <set>
#include <iostream>
#include <algorithm>
#include <filesystem>
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "error.hpp"
#include "fileUtil.hpp"
#include "exitfuncs.hpp"
#include "renderPrimitives.hpp"
#include "shapes.hpp"

#ifdef DEBUG
bool debugMode = true;
#else
bool debugMode = false;
#endif

struct UniformBufferObject {
    glm::mat4x4 model;
    glm::mat4x4 view;
    glm::mat4x4 proj;
};

const int MAX_FRAMES_IN_FLIGHT = 1;

SDL_Window* window = nullptr;

VkApplicationInfo appInfo = {
    .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
    .pNext = nullptr,
    .pApplicationName = "Amius Adventure",
    .applicationVersion = VK_MAKE_VERSION(0, 2, 0),
    .pEngineName = "Amius Adventure Engine",
    .engineVersion = VK_MAKE_VERSION(1, 0, 0),
    .apiVersion = VK_API_VERSION_1_4
};

VkInstance instance;
VkSurfaceKHR surface;

std::vector<const char*> instanceExtensions = {};
VkInstanceCreateInfo instanceCreateInfo = {
    .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0,
    .pApplicationInfo = &appInfo,
    .enabledLayerCount = 0,
    .ppEnabledLayerNames = nullptr,
    .enabledExtensionCount = 0,
    .ppEnabledExtensionNames = nullptr
};

VkDebugUtilsMessengerEXT debugMessenger;
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {
    
    if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        setErr("Vulkan Error: " + std::string(pCallbackData->pMessage));
    } else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        setErr("Vulkan Warning: " + std::string(pCallbackData->pMessage));
    }
    
    return VK_FALSE;
}

std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
struct QueueFamilyIndicies {
    std::optional<uint32_t> graphicsFamily = std::nullopt;
    std::optional<uint32_t> presentFamily = std::nullopt;
};
QueueFamilyIndicies deviceQueueFamilyIndices;
struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};
SwapChainSupportDetails deviceSwapChainSupportDetails;

VkDevice device;
VkQueue graphicsQueue;
VkQueue presentQueue;

VkSwapchainKHR swapChain;
std::vector<VkImage> swapChainImages;
VkFormat swapChainImageFormat;
VkExtent2D swapChainExtent;

std::vector<VkImageView> swapChainImageViews;

VkShaderModule defaultShaderModule = VK_NULL_HANDLE;
VkRenderPass renderPass = VK_NULL_HANDLE;

VkDescriptorSetLayout descriptorSetLayout;

VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
VkPipeline defaultPipeline = VK_NULL_HANDLE;

std::vector<VkFramebuffer> swapChainFramebuffers;

VmaAllocator allocator;

VkBuffer stagingBuffer;
VmaAllocation stagingAllocation;
VkBuffer vertexBuffer;
VmaAllocation vertexAllocation;
VkBuffer indexBuffer;
VmaAllocation indexAllocation;

std::vector<VkBuffer> uniformBuffers;
std::vector<VmaAllocation> uniformAllocations;
std::vector<void*> uniformBuffersMapped;

VkCommandPool commandPool = VK_NULL_HANDLE;
std::vector<VkCommandBuffer> commandBuffers;

std::vector<VkSemaphore> imageAvailableSemaphores;
std::vector<VkSemaphore> renderFinishedSemaphores;
std::vector<VkFence> inFlightFences;

VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
uint32_t currentFrame = 0;
bool framebufferResized = false;

void setFramebufferResized() {
    framebufferResized = true;
}

bool checkValidationSupport() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    if (layerCount == 0) {
        setErr("No Vulkan validation layers available");
        return false;
    }

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const auto& layer : availableLayers) {
        if (strcmp(layer.layerName, "VK_LAYER_KHRONOS_validation") == 0) {
            return true;
        }
    }

    setErr("VK_LAYER_KHRONOS_validation layer not found");
    return false;
}

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

VkShaderModule createShaderModule(const std::vector<char>& code) {
    VkShaderModuleCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .codeSize = code.size(),
        .pCode = reinterpret_cast<const uint32_t*>(code.data())
    };

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        setErr("Failed to create shader module");
        return VK_NULL_HANDLE;
    }
    
    return shaderModule;
}

bool createSwapChain() {
    // Choose surface format
    VkSurfaceFormatKHR chosenFormat = deviceSwapChainSupportDetails.formats[0];
    for (const auto& format : deviceSwapChainSupportDetails.formats) {
        if (format.format == VK_FORMAT_B8G8R8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            chosenFormat = format;
            break;
        }
    }

    // Choose surface present mode
    VkPresentModeKHR chosenPresentMode = VK_PRESENT_MODE_FIFO_KHR;
    for (const auto& presentMode : deviceSwapChainSupportDetails.presentModes) {
        if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            chosenPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
            break;
        }
    }

    // Choose swap extent
    VkExtent2D chosenSwapExtent = deviceSwapChainSupportDetails.capabilities.currentExtent;
    if (chosenSwapExtent.width == UINT32_MAX || chosenSwapExtent.height == UINT32_MAX) {
        int width, height;
        SDL_GetWindowSize(window, &width, &height);

        chosenSwapExtent.width = static_cast<uint32_t>(width);
        chosenSwapExtent.height = static_cast<uint32_t>(height);

        chosenSwapExtent.width = std::clamp(chosenSwapExtent.width, 
                                            deviceSwapChainSupportDetails.capabilities.minImageExtent.width, 
                                            deviceSwapChainSupportDetails.capabilities.maxImageExtent.width);
        chosenSwapExtent.height = std::clamp(chosenSwapExtent.height, 
                                             deviceSwapChainSupportDetails.capabilities.minImageExtent.height, 
                                             deviceSwapChainSupportDetails.capabilities.maxImageExtent.height);
    }

    // Decide on the number of images in the swap chain
    uint32_t imageCount = deviceSwapChainSupportDetails.capabilities.minImageCount + 1;
    if (deviceSwapChainSupportDetails.capabilities.maxImageCount > 0 && imageCount > deviceSwapChainSupportDetails.capabilities.maxImageCount) {
        imageCount = deviceSwapChainSupportDetails.capabilities.maxImageCount;
    }

    // Create Swap Chain
    VkSwapchainCreateInfoKHR swapChainCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .pNext = nullptr,
        .flags = 0,
        .surface = surface,
        .minImageCount = imageCount,
        .imageFormat = chosenFormat.format,
        .imageColorSpace = chosenFormat.colorSpace,
        .imageExtent = chosenSwapExtent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = nullptr,
        .preTransform = deviceSwapChainSupportDetails.capabilities.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = chosenPresentMode,
        .clipped = VK_TRUE
    };

    swapChainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

    if (deviceQueueFamilyIndices.graphicsFamily != deviceQueueFamilyIndices.presentFamily) {
        swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapChainCreateInfo.queueFamilyIndexCount = 2;
        uint32_t queueFamilyIndices[] = {deviceQueueFamilyIndices.graphicsFamily.value(), deviceQueueFamilyIndices.presentFamily.value()};
        swapChainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
    }

    if (vkCreateSwapchainKHR(device, &swapChainCreateInfo, nullptr, &swapChain) != VK_SUCCESS) {
        setErr("Failed to create swap chain");
        return false;
    }

    uint32_t swapImageCount = 0;
    vkGetSwapchainImagesKHR(device, swapChain, &swapImageCount, nullptr);
    swapChainImages.resize(swapImageCount);
    vkGetSwapchainImagesKHR(device, swapChain, &swapImageCount, swapChainImages.data());
    swapChainImageFormat = chosenFormat.format;
    swapChainExtent = chosenSwapExtent;

    return true;
}

bool createImageViews() {
    swapChainImageViews.resize(swapChainImages.size());
    for (size_t i = 0; i < swapChainImages.size(); i++) {
        VkImageViewCreateInfo viewCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .image = swapChainImages[i],
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = swapChainImageFormat,
            .components = {
                .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                .a = VK_COMPONENT_SWIZZLE_IDENTITY
            },
            .subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1
            }
        };

        if (vkCreateImageView(device, &viewCreateInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS) {
            setErr("Failed to create image view for swap chain image " + std::to_string(i));
            return false;
        }
    }

    return true;
}

bool createFramebuffers() {
    swapChainFramebuffers.resize(swapChainImageViews.size());
    for (size_t i = 0; i < swapChainImageViews.size(); i++) {
        VkImageView attachments[] = {swapChainImageViews[i]};

        VkFramebufferCreateInfo framebufferInfo = {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .renderPass = renderPass,
            .attachmentCount = 1,
            .pAttachments = attachments,
            .width = swapChainExtent.width,
            .height = swapChainExtent.height,
            .layers = 1
        };

        if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
            setErr("Failed to create framebuffer for swap chain image " + std::to_string(i));
            return false;
        }
    }

    return true;
}

void cleanupSwapChain() {
    for (auto framebuffer : swapChainFramebuffers) {
        vkDestroyFramebuffer(device, framebuffer, nullptr);
    }
    swapChainFramebuffers.clear();

    for (auto imageView : swapChainImageViews) {
        vkDestroyImageView(device, imageView, nullptr);
    }
    swapChainImageViews.clear();

    vkDestroySwapchainKHR(device, swapChain, nullptr);
}

bool recreateSwapchain() {
    vkDeviceWaitIdle(device);

    cleanupSwapChain();

    if (!createSwapChain()) {
        setErr("Failed to create swapchain: " + getErr());
        return false;
    }
    if (!createImageViews()) {
        setErr("Failed to create image views: " + getErr());
        return false;
    }
    if (!createFramebuffers()) {
        setErr("Failed to create framebuffers: " + getErr());
        return false;
    }

    return true;
}

uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter &  (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    exitWithErrorWindow("Unable to find a memory type");
    return UINT32_MAX;
}

bool copyBuffer(VkBuffer src, VkBuffer dst, VkDeviceSize size) {
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

    VkBufferCopy copy = {
        .srcOffset = 0,
        .dstOffset = 0,
        .size = size
    };
    vkCmdCopyBuffer(tempCommandBuffer, src, dst, 1, &copy);

    if (vkEndCommandBuffer(tempCommandBuffer) != VK_SUCCESS) {
        setErr("Failed to end command buffer");
    }

    VkSubmitInfo submitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &tempCommandBuffer
    };
    vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphicsQueue);

    vkFreeCommandBuffers(device, tempCommandPool, 1, &tempCommandBuffer);
    vkDestroyCommandPool(device, tempCommandPool, nullptr);


    return true;
}

bool initGfx(SDL_Window* win) {
    window = win;
    // VK INSTANCE CREATION
    uint32_t extensionCount = 0;
    const char* const* requiredExtensions = SDL_Vulkan_GetInstanceExtensions(&extensionCount);
    for (uint32_t i = 0; i < extensionCount; i++) {
        instanceExtensions.emplace_back(requiredExtensions[i]);
    }
    
    #ifdef __APPLE__ // some extra vulkan stuff to make MoltenVK work
    instanceExtensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    instanceCreateInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
    #endif

    if (debugMode) {
        if (!checkValidationSupport()) {
            return false;
        }
        instanceExtensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        instanceCreateInfo.enabledLayerCount = 1;
        const char* validationLayer = "VK_LAYER_KHRONOS_validation";
        instanceCreateInfo.ppEnabledLayerNames = &validationLayer;
    }

    instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size());
    instanceCreateInfo.ppEnabledExtensionNames = instanceExtensions.data();
    
    if(vkCreateInstance(&instanceCreateInfo, nullptr, &instance) != VK_SUCCESS) {
        setErr("Failed to create Vulkan instance");
        return false;
    };

    // VK SURFACE CREATION
    if (!SDL_Vulkan_CreateSurface(window, instance, nullptr, &surface)) {
        setErr("Failed to create Vulkan surface: " + std::string(SDL_GetError()));
        return false;
    }

    // VK DEBUG MESSENGER CREATION
    if (debugMode) {
        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
            .pNext = nullptr,
            .flags = 0,
            .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
            .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
            .pfnUserCallback = debugCallback,
            .pUserData = nullptr
        };

        if (CreateDebugUtilsMessengerEXT(instance, &debugCreateInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
            setErr("Failed to create Vulkan debug messenger");
            return false;
        }
    }

    // PICK A PHYSICAL DEVICE
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
    if (deviceCount == 0) {
        setErr("Failed to find GPUs with Vulkan support");
        return false;
    }

    for (const auto& device : devices) {
        VkPhysicalDeviceProperties deviceProperties;
        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceProperties(device, &deviceProperties);
        vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
        
        // Check Queue Families
        QueueFamilyIndicies queueFamilyIndices;
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
        
        int i = 0;
        for (const auto& queueFamily : queueFamilies) {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                queueFamilyIndices.graphicsFamily = i;
            }
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
            if (presentSupport) {
                queueFamilyIndices.presentFamily = i;
            }
            if (queueFamilyIndices.graphicsFamily.has_value() && queueFamilyIndices.presentFamily.has_value()) {
                break; // Found both graphics and present families
            }
            i++;
        }
        if (!queueFamilyIndices.graphicsFamily.has_value()) {
            setErr("No graphics queue found");
            continue; // No graphics queue found, skip this device
        }
        if (!queueFamilyIndices.presentFamily.has_value()) {
            setErr("No present queue found");
            continue;
        }
        
        // Check Extensions
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        std::set<std::string> requiredExtensionsSet(deviceExtensions.begin(), deviceExtensions.end());
        for (const auto& extension : availableExtensions) {
            requiredExtensionsSet.erase(extension.extensionName);
        }
        
        if (!requiredExtensionsSet.empty()) {
            setErr("One or more required extensions were not met");
            continue;
        }

        // Check SwapChain support
        SwapChainSupportDetails details = {};
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);
        
        uint32_t formatCount = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
        if (formatCount != 0) {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
        }

        uint32_t presentModeCount = 0;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
        if (presentModeCount != 0) {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
        }

        if (details.formats.empty() || details.presentModes.empty()) {
            setErr("SwapChain support inadequate");
            continue;
        }

        physicalDevice = device;
        deviceQueueFamilyIndices = queueFamilyIndices;
        deviceSwapChainSupportDetails = details;
        break;
    }
    
    if (physicalDevice == VK_NULL_HANDLE) {
        setErr("Failed to find a suitable GPU: " + getErr());
        return false;
    }
    
    
    // CREATE LOGICAL DEVICE
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {deviceQueueFamilyIndices.graphicsFamily.value(), deviceQueueFamilyIndices.presentFamily.value()};

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .queueFamilyIndex = queueFamily,
            .queueCount = 1,
            .pQueuePriorities = &queuePriority
        };
        queueCreateInfos.push_back(queueCreateInfo);
    }
    VkPhysicalDeviceFeatures deviceFeatures = {};
    VkDeviceCreateInfo deviceCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
        .pQueueCreateInfos = queueCreateInfos.data(),
        .enabledLayerCount = 0,
        .ppEnabledLayerNames = nullptr,
        .enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()),
        .ppEnabledExtensionNames = deviceExtensions.data(),
        .pEnabledFeatures = &deviceFeatures
    };

    if (debugMode) {
        deviceCreateInfo.enabledLayerCount = 1;
        const char* validationLayer = "VK_LAYER_KHRONOS_validation";
        deviceCreateInfo.ppEnabledLayerNames = &validationLayer;
    }

    if (vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device) != VK_SUCCESS) {
        setErr("Failed to create Vulkan device");
        return false;
    }

    vkGetDeviceQueue(device, deviceQueueFamilyIndices.graphicsFamily.value(), 0, &graphicsQueue);
    vkGetDeviceQueue(device, deviceQueueFamilyIndices.presentFamily.value(), 0, &presentQueue);

    // CREATE SWAP CHAIN
    if (!createSwapChain()) {
        setErr("Failed to create swap chain: " + getErr());
        return false;
    }

    // CREATE IMAGE VIEWS
    if (!createImageViews()) {
        setErr("Failed to create image views: " + getErr());
        return false;
    }

    // CREATE RENDER PASS
    VkAttachmentDescription colorAttachment = {
        .format = swapChainImageFormat,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
    };
    VkAttachmentReference colorAttachmentRef = {
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };
    VkSubpassDescription subpass = {
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachmentRef
    };
    VkSubpassDependency dependency = {
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcAccessMask = 0,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        .dependencyFlags = 0
    };

    VkRenderPassCreateInfo renderPassInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .attachmentCount = 1,
        .pAttachments = &colorAttachment,
        .subpassCount = 1,
        .pSubpasses = &subpass,
        .dependencyCount = 1,
        .pDependencies = &dependency
    };
    if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
        setErr("Failed to create render pass");
        return false;
    }

    // CREATE DESCRIPTOR SET LAYOUT
    VkDescriptorSetLayoutBinding uboLayoutBinding = {
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
        .pImmutableSamplers = nullptr
    };

    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = 1,
        .pBindings = &uboLayoutBinding
    };
    if (vkCreateDescriptorSetLayout(device, &descriptorSetLayoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
        setErr("Could not create descriptor set layout");
        return false;
    }

    // CREATE GRAPHICS PIPELINE
    // A new graphics pipeline should be created for each shader program we intend to use, but right now we are just gonna use one
    // Compile Shaders
    std::vector<char> shaderCode = readFile("data/gfx/shaders/shader.spv");
    if (shaderCode.empty()) {
        setErr("Failed to read shader file: " + getErr());
        return false;
    }
    defaultShaderModule = createShaderModule(shaderCode);
    if (defaultShaderModule == VK_NULL_HANDLE) {
        setErr("Failed to create shader module: " + getErr());
        return false;
    }

    // Shader Stage Info
    VkPipelineShaderStageCreateInfo vertexShaderStageInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .module = defaultShaderModule,
        .pName = "main",
        .pSpecializationInfo = nullptr
    };
    VkPipelineShaderStageCreateInfo fragmentShaderStageInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module = defaultShaderModule,
        .pName = "main",
        .pSpecializationInfo = nullptr
    };

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertexShaderStageInfo, fragmentShaderStageInfo};

    // Dynamic State
    std::vector<VkDynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };
    
    VkPipelineDynamicStateCreateInfo dynamicState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
        .pDynamicStates = dynamicStates.data()
    };

    auto bindingDesc = Vertex::getBindingDescription();
    auto attrDescs = Vertex::getAttributeDescriptions();

    // Vertex Input State
    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &bindingDesc,
        .vertexAttributeDescriptionCount = static_cast<uint32_t>(attrDescs.size()),
        .pVertexAttributeDescriptions = attrDescs.data()
    };

    // Input Assembly State
    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE
    };

    // Viewport
    VkViewport viewport = {
        .x = 0.0f,
        .y = 0.0f,
        .width = static_cast<float>(swapChainExtent.width),
        .height = static_cast<float>(swapChainExtent.height),
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };
    VkRect2D scissor = {
        .offset = {0, 0},
        .extent = swapChainExtent
    };

    VkPipelineViewportStateCreateInfo viewportState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .viewportCount = 1,
        .pViewports = &viewport,
        .scissorCount = 1,
        .pScissors = &scissor
    };

    // Rasterization State
    VkPipelineRasterizationStateCreateInfo rasterizer = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .frontFace = VK_FRONT_FACE_CLOCKWISE,
        .depthBiasEnable = VK_FALSE,
        .depthBiasConstantFactor = 0.0f,
        .depthBiasClamp = 0.0f,
        .depthBiasSlopeFactor = 0.0f,
        .lineWidth = 1.0f
    };

    // Multisample State
    VkPipelineMultisampleStateCreateInfo multisampling = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        .sampleShadingEnable = VK_FALSE,
        .minSampleShading = 1.0f,
        .pSampleMask = nullptr,
        .alphaToCoverageEnable = VK_FALSE,
        .alphaToOneEnable = VK_FALSE
    };

    // Color Blend State
    VkPipelineColorBlendAttachmentState colorBlendAttachment = {
        .blendEnable = VK_TRUE,
        .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
        .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        .colorBlendOp = VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
        .alphaBlendOp = VK_BLEND_OP_ADD,
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
    };
    VkPipelineColorBlendStateCreateInfo colorBlending = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .logicOpEnable = VK_FALSE,
        .logicOp = VK_LOGIC_OP_COPY,
        .attachmentCount = 1,
        .pAttachments = &colorBlendAttachment,
        .blendConstants = {0.0f, 0.0f, 0.0f, 0.0f}
    };

    // Pipeline Layout
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .setLayoutCount = 1,
        .pSetLayouts = &descriptorSetLayout,
        .pushConstantRangeCount = 0,
        .pPushConstantRanges = nullptr
    };
    if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
        setErr("Failed to create pipeline layout");
        return false;
    }

    // Pipeline Creation
    VkGraphicsPipelineCreateInfo pipelineInfo = {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .stageCount = 2,
        .pStages = shaderStages,
        .pVertexInputState = &vertexInputInfo,
        .pInputAssemblyState = &inputAssembly,
        .pTessellationState = nullptr,
        .pViewportState = &viewportState,
        .pRasterizationState = &rasterizer,
        .pMultisampleState = &multisampling,
        .pDepthStencilState = nullptr,
        .pColorBlendState = &colorBlending,
        .pDynamicState = &dynamicState,
        .layout = pipelineLayout,
        .renderPass = renderPass,
        .subpass = 0,
        .basePipelineHandle = VK_NULL_HANDLE,
        .basePipelineIndex = -1
    };
    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &defaultPipeline) != VK_SUCCESS) {
        setErr("Failed to create graphics pipeline");
        return false;
    }

    // FRAMEBUFFER CREATION
    if (!createFramebuffers()) {
        setErr("Failed to create framebuffers: " + getErr());
        return false;
    }

    // VMA ALLOCATOR CREATION
    VmaVulkanFunctions vmaVulkanFunctions = {
        .vkGetInstanceProcAddr = vkGetInstanceProcAddr,
        .vkGetDeviceProcAddr = vkGetDeviceProcAddr
    };
    VmaAllocatorCreateInfo vmaAllocatorInfo = {
        .flags = VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT,
        .physicalDevice = physicalDevice,
        .device = device,
        .pVulkanFunctions = &vmaVulkanFunctions,
        .instance = instance,
        .vulkanApiVersion = appInfo.apiVersion
    };
    if (vmaCreateAllocator(&vmaAllocatorInfo, &allocator) != VK_SUCCESS) {
        setErr("Failed to create VMA instance");
        return false;
    }

    // BUFFER CREATION
    // Staging
    VkBufferCreateInfo stagingBufferInfo {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .size = sizeof(rectangleVerticies[0]) * rectangleVerticies.size(),
        .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };
    VmaAllocationCreateInfo stagingAllocInfo {
        .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
        .usage = VMA_MEMORY_USAGE_AUTO,
        .requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    };
    if (vmaCreateBuffer(allocator, &stagingBufferInfo, &stagingAllocInfo, &stagingBuffer, &stagingAllocation, nullptr) != VK_SUCCESS) {
        setErr("Failed to allocate staging buffer memory");
        return false;
    }
    // TODO: Get rid of this when dynamically managing the vertex buffer! OLD!
    vmaCopyMemoryToAllocation(allocator, rectangleVerticies.data(), stagingAllocation, 0, (size_t)stagingBufferInfo.size);

    // Vertex
    VkBufferCreateInfo vertexBufferInfo {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .size = sizeof(rectangleVerticies[0]) * rectangleVerticies.size(),
        .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };
    VmaAllocationCreateInfo vertexAllocInfo {
        .flags = 0,
        .usage = VMA_MEMORY_USAGE_AUTO,
        .requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    };
    if (vmaCreateBuffer(allocator, &vertexBufferInfo, &vertexAllocInfo, &vertexBuffer, &vertexAllocation, nullptr) != VK_SUCCESS) {
        setErr("Failed to allocate vertex buffer memory");
        return false;
    }

    // TODO: Get rid of this when dynamically managing vertex buffer! OLD!
    copyBuffer(stagingBuffer, vertexBuffer, (VkDeviceSize)stagingBufferInfo.size);

    // Index
    VkBufferCreateInfo indexBufferInfo {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .size = sizeof(rectangleIndices[0]) * rectangleIndices.size(),
        .usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };
    VmaAllocationCreateInfo indexAllocInfo {
        .flags = 0,
        .usage = VMA_MEMORY_USAGE_AUTO,
        .requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    }; 
    if (vmaCreateBuffer(allocator, &indexBufferInfo, &indexAllocInfo, &indexBuffer, &indexAllocation, nullptr)  != VK_SUCCESS) {
        setErr("Failed to allocate index buffer memory");
        return false;
    }

    // TODO: Get rid of this when dynamically managing index buffer! OLD!
    vmaCopyMemoryToAllocation(allocator, rectangleIndices.data(), stagingAllocation, 0, (size_t)indexBufferInfo.size);
    copyBuffer(stagingBuffer, indexBuffer, (size_t)indexBufferInfo.size);

    // Uniform
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    uniformAllocations.resize(MAX_FRAMES_IN_FLIGHT);
    uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VkBufferCreateInfo uniformBufferInfo {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .size = bufferSize,
            .usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE
        };
        VmaAllocationCreateInfo uniformAllocInfo {
            .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
            .usage = VMA_MEMORY_USAGE_AUTO,
            .requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        }; 
        if (vmaCreateBuffer(allocator, &uniformBufferInfo, &uniformAllocInfo, &uniformBuffers[i], &uniformAllocations[i], nullptr) != VK_SUCCESS) {
            setErr("Failed to create Uniform Buffers");
            return false;
        }
        vmaMapMemory(allocator, uniformAllocations[i], &uniformBuffersMapped[i]);
    }

    // COMMAND POOL CREATION
    VkCommandPoolCreateInfo commandPoolInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = deviceQueueFamilyIndices.graphicsFamily.value()
    };
    if (vkCreateCommandPool(device, &commandPoolInfo, nullptr, &commandPool) != VK_SUCCESS) {
        setErr("Failed to create command pool");
        return false;
    }

    // COMMAND BUFFERS CREATION
    commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    VkCommandBufferAllocateInfo commandBufferAllocInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = static_cast<uint32_t>(commandBuffers.size())
    };
    if (vkAllocateCommandBuffers(device, &commandBufferAllocInfo, commandBuffers.data()) != VK_SUCCESS) {
        setErr("Failed to allocate command buffer");
        return false;
    }

    // SYNC OBJECTS CREATION
    imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
    VkSemaphoreCreateInfo semaphoreInfo = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0
    };
    VkFenceCreateInfo fenceInfo = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT
    };
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
            setErr("Failed to create image available semaphore");
            return false;
        }
    }

    return true;
}

void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex, VkPipeline pipeline = defaultPipeline) {
    VkCommandBufferBeginInfo beginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = 0,
        .pInheritanceInfo = nullptr
    };

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        setErr("Failed to begin command buffer");
        return;
    }

    VkRenderPassBeginInfo renderPassInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .pNext = nullptr,
        .renderPass = renderPass,
        .framebuffer = swapChainFramebuffers[imageIndex],
        .renderArea = {
            .offset = {0, 0},
            .extent = swapChainExtent
        },
        .clearValueCount = 1,
        .pClearValues = &clearColor
    };

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

    VkBuffer vertexBuffers[] = {vertexBuffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

    vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT16);

    VkViewport viewport = {
        .x = 0.0f,
        .y = 0.0f,
        .width = static_cast<float>(swapChainExtent.width),
        .height = static_cast<float>(swapChainExtent.height),
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor = {
        .offset = {0, 0},
        .extent = swapChainExtent
    };
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(rectangleIndices.size()), 1, 0, 0, 0);

    vkCmdEndRenderPass(commandBuffer);
    
    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        setErr("Failed to end command buffer");
        return;
    }
}

void gfxUpdate(AmiusAdventure::Scene::Scene* scene) {

    vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex = 0;
    VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        if (!recreateSwapchain()) {
            exitWithErrorWindow("Failed to recreate swapchain: " + getErr());
        }
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        exitWithErrorWindow("failed to aquire swapchain image");
    }

    vkResetFences(device, 1, &inFlightFences[currentFrame]);
    vkResetCommandBuffer(commandBuffers[currentFrame], 0);

    recordCommandBuffer(commandBuffers[currentFrame], imageIndex);

    // process uniforms
    // TODO: Do once for each object
    UniformBufferObject ubo {};
    ubo.model = glm::mat4x4(1.0f); // TODO: Update model per drawn asset
    ubo.view = scene->ctx.camera->getTransform();
    ubo.proj = glm::perspective(scene->ctx.camera->fovY, swapChainExtent.width / (float) swapChainExtent.height, scene->ctx.camera->zNear, scene->ctx.camera->zFar);
    ubo.proj[1][1] *= -1;
    memcpy(uniformBuffersMapped[currentFrame], &ubo, sizeof(ubo));

    VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
    VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
    VkSubmitInfo submitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext = nullptr,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = waitSemaphores,
        .pWaitDstStageMask = (VkPipelineStageFlags[]){VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT},
        .commandBufferCount = 1,
        .pCommandBuffers = &commandBuffers[currentFrame],
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = signalSemaphores
    };

    if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
        exitWithErrorWindow("Failed to submit draw command buffer");
    }

    VkSwapchainKHR swapChains[] = {swapChain};
    VkPresentInfoKHR presentInfo = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .pNext = nullptr,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = signalSemaphores,
        .swapchainCount = 1,
        .pSwapchains = swapChains,
        .pImageIndices = &imageIndex,
        .pResults = nullptr
    };

    result = vkQueuePresentKHR(presentQueue, &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
        if (!recreateSwapchain()) {
            exitWithErrorWindow("Failed to recreate swapchain: " + getErr());
        }
        framebufferResized = false;
    }
    else if (result != VK_SUCCESS) {
        exitWithErrorWindow("failed to aquire swapchain image");
    }

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void gfxQuit() {
    vkDeviceWaitIdle(device);
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
        vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
        vkDestroyFence(device, inFlightFences[i], nullptr);
    }
    vkFreeCommandBuffers(device, commandPool, 1, commandBuffers.data());
    vkDestroyCommandPool(device, commandPool, nullptr);
    vmaDestroyBuffer(allocator, stagingBuffer, stagingAllocation);
    vmaDestroyBuffer(allocator, vertexBuffer, vertexAllocation);
    vmaDestroyBuffer(allocator, indexBuffer, indexAllocation);
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vmaUnmapMemory(allocator, uniformAllocations[i]);
        vmaDestroyBuffer(allocator, uniformBuffers[i], uniformAllocations[i]);
    }
    vmaDestroyAllocator(allocator);
    cleanupSwapChain();
    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
    vkDestroyPipeline(device, defaultPipeline, nullptr);
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    vkDestroyRenderPass(device, renderPass, nullptr);
    vkDestroyShaderModule(device, defaultShaderModule, nullptr);
    vkDestroyDevice(device, nullptr);
    SDL_Vulkan_DestroySurface(instance, surface, nullptr);
    if (debugMode) {
        DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    }
    vkDestroyInstance(instance, nullptr);
}