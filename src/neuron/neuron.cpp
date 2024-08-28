#include "neuron.hpp"

#include <GLFW/glfw3.h>

#include <spdlog/spdlog.h>

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE;

namespace neuron {
    static Context *context;

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                        const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData) {

        // TODO: debug output

        switch (messageSeverity) {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            spdlog::debug("[validation] {}", pCallbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            spdlog::info("[validation] {}", pCallbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            spdlog::warn("[validation] {}", pCallbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            spdlog::error("[validation] {}", pCallbackData->pMessage);
            break;
        }

        return VK_FALSE;
    }

    void init(const Settings &settings) {
        glfwInit();
        VULKAN_HPP_DEFAULT_DISPATCHER.init();

        context = new Context(settings);
    }

    void cleanup() {
        delete context;
        glfwTerminate();
    }

    Context::Context(const Settings &settings) {
        if (settings.debugMode)
            spdlog::set_level(spdlog::level::debug);

        vk::ApplicationInfo appInfo{};
        appInfo.setApiVersion(vk::ApiVersion13);
        appInfo.setEngineVersion(neuron::VERSION.toUintVk()).setPEngineName(neuron::NAME.data());
        appInfo.setApplicationVersion(settings.version.toUintVk()).setPApplicationName(settings.name.c_str());

        std::vector<const char *> instanceExtensions;
        std::vector<const char *> instanceLayers;

        vk::DebugUtilsMessengerCreateInfoEXT debuggerCreateInfo{};
        vk::InstanceCreateInfo               instanceCreateInfo{};

        if (settings.debugMode) {
            instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
            instanceLayers.push_back("VK_LAYER_KHRONOS_validation");
            debuggerCreateInfo.messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eError | vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo |
                vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning;
            debuggerCreateInfo.messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eDeviceAddressBinding | vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation;
            debuggerCreateInfo.pUserData       = this;
            debuggerCreateInfo.pfnUserCallback = debugCallback;

            instanceCreateInfo.pNext = &debuggerCreateInfo;
        }

        if (settings.vulkanApiDump) {
            instanceLayers.push_back("VK_LAYER_LUNARG_api_dump");
        }

        if (!settings.offscreenRenderingOnly) {
            uint32_t     count;
            const char **requiredExtensions = glfwGetRequiredInstanceExtensions(&count);
            for (uint32_t i = 0; i < count; i++) {
                instanceExtensions.push_back(requiredExtensions[i]);
            }
        }

        instanceCreateInfo.setPApplicationInfo(&appInfo).setPEnabledExtensionNames(instanceExtensions).setPEnabledLayerNames(instanceLayers);

        m_Instance = vk::createInstance(instanceCreateInfo);
        VULKAN_HPP_DEFAULT_DISPATCHER.init(m_Instance);

        if (settings.debugMode) {
            m_DebugMessenger = m_Instance.createDebugUtilsMessengerEXT(debuggerCreateInfo);
        }
    }

    Context::~Context() {
        if (m_DebugMessenger.has_value()) {
            m_Instance.destroy(m_DebugMessenger.value());
        }

        m_Instance.destroy();
    }

    Context *Context::get() noexcept {
        return context;
    }
} // namespace neuron
