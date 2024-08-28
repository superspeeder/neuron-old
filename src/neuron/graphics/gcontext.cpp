#include "gcontext.hpp"

#include "neuron/math/utils.hpp"

#include <GLFW/glfw3.h>
#include <limits>

#include <unordered_set>

#include <spdlog/spdlog.h>

namespace neuron::graphics {


    GContext::GContext(const GCSettings &settings) {

        // TODO: non-naive gpu selection, allow user to manually select gpu
        m_Gpu = Context::get()->getInstance().enumeratePhysicalDevices().front();


        std::unordered_map<uint32_t, uint32_t> queueCounts;

        auto queueFamilyProperties = m_Gpu.getQueueFamilyProperties();

        uint32_t index = 0;
        for (const auto &properties : queueFamilyProperties) {
            if (!m_TransferQueueFamily.has_value() && (properties.queueFlags & vk::QueueFlagBits::eTransfer) && !(properties.queueFlags & vk::QueueFlagBits::eGraphics) &&
                !(properties.queueFlags & vk::QueueFlagBits::eCompute)) {
                m_TransferQueueFamily = index;
            }

            if (!m_ComputeQueueFamily.has_value() && (properties.queueFlags & vk::QueueFlagBits::eCompute) && !(properties.queueFlags & vk::QueueFlagBits::eGraphics)) {
                m_ComputeQueueFamily = index;
            }

            if (properties.queueFlags & vk::QueueFlagBits::eVideoDecodeKHR) {
                m_VideoDecodeQueueFamily = index;
            }

            index++;
        }

        // TODO: support nonprimary presentation families, primary queue families other than 0

        for (const auto &request : settings.queueRequests) {
            uint32_t qf = UINT32_MAX;
            switch (request.type) {
            case QueueType::Primary:
                qf = m_PrimaryQueueFamily;
                break;
            case QueueType::Transfer:
                if (!m_TransferQueueFamily.has_value()) {
                    spdlog::warn("No exclusive transfer queue. Defaulting to primary.");
                }
                qf = m_TransferQueueFamily.value_or(m_PrimaryQueueFamily);
                break;
            case QueueType::Compute:
                if (!m_ComputeQueueFamily.has_value()) {
                    spdlog::warn("No separate compute queue. Defaulting to primary.");
                }
                qf = m_ComputeQueueFamily.value_or(m_PrimaryQueueFamily);
                break;
            case QueueType::VideoEncode:
                if (!m_VideoEncodeQueueFamily.has_value()) {
                    throw std::runtime_error("No video encoding queue family");
                }
                qf = m_VideoEncodeQueueFamily.value();
                break;
            case QueueType::VideoDecode:
                if (!m_VideoDecodeQueueFamily.has_value()) {
                    throw std::runtime_error("No video decoding queue family");
                }
                qf = m_VideoDecodeQueueFamily.value();
                break;
            }

            if (queueCounts.contains(qf)) {
                queueCounts[qf] += request.count;
            } else {
                queueCounts[qf] = request.count;
            }
        }

        auto pcount = queueCounts.find(m_PrimaryQueueFamily);
        if (pcount == queueCounts.end())
            queueCounts[m_PrimaryQueueFamily] = 1;
        else
            queueCounts[m_PrimaryQueueFamily] += 1;

        std::vector<float *>                   queuePriorities;
        std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;

        for (const auto &entry : queueCounts) {
            if (queueFamilyProperties[entry.first].queueCount < entry.second) {
                throw std::runtime_error("Not enough queues available for requests.");
            }

            auto *priorities = utils::allocateAndFillArray<float>(entry.second, 1.0f);

            queueCreateInfos.emplace_back(vk::DeviceQueueCreateFlags{}, entry.first, entry.second, priorities);
            queuePriorities.push_back(priorities);
        }

        std::vector<const char *>       deviceExtensions;
        std::unordered_set<std::string> extraExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

        for (const char *extensionName : settings.requestedExtensions) {
            extraExtensions.erase(std::string(extensionName));
            deviceExtensions.push_back(extensionName);
        }

        for (const auto &string : extraExtensions) {
            deviceExtensions.push_back(string.c_str());
        }

        // TODO: user requested features
        vk::PhysicalDeviceFeatures2 f2{};
        f2.features.tessellationShader = true;
        f2.features.geometryShader     = true;
        f2.features.wideLines          = true;
        f2.features.largePoints        = true;
        f2.features.fillModeNonSolid   = true;

        m_Device = m_Gpu.createDevice(vk::DeviceCreateInfo({}, queueCreateInfos, {}, deviceExtensions, nullptr, &f2));

        for (float *p : queuePriorities) {
            delete[] p;
        }

        m_PrimaryQueue = m_Device.getQueue(m_PrimaryQueueFamily, queueCounts[m_PrimaryQueueFamily] - 1);
    }

    GContext::~GContext() {
        m_Device.destroy();
    }

    std::optional<vk::Queue> GContext::getQueue(QueueType type, uint32_t index) const {
        auto family = getQueueFamily(type);
        if (family.has_value()) {
            return m_Device.getQueue(family.value(), index);
        }

        return std::nullopt;
    }

    std::optional<uint32_t> GContext::getQueueFamily(QueueType type) const {
        switch (type) {
        case QueueType::Primary:
            return m_PrimaryQueueFamily;
        case QueueType::Transfer:
            return m_TransferQueueFamily;
        case QueueType::Compute:
            return m_ComputeQueueFamily;
        case QueueType::VideoEncode:
            return m_VideoEncodeQueueFamily;
        case QueueType::VideoDecode:
            return m_VideoDecodeQueueFamily;
        }
    }

    vk::Queue GContext::getPrimaryQueue() const {
        return m_PrimaryQueue;
    }

    std::tuple<vk::Format, vk::ColorSpaceKHR> selectPreferredSurfaceFormat(const std::shared_ptr<GContext> &gc, vk::SurfaceKHR surface) {
        auto surfaceFormats  = gc->getGpu().getSurfaceFormatsKHR(surface);
        bool foundBGRA_SRGB  = false;
        bool foundRGBA_SRGB  = false;
        bool foundBGRA_UNORM = false;
        bool foundRGBA_UNORM = false;

        for (const auto &sf : surfaceFormats) {
            if (sf.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
                switch (sf.format) {
                case vk::Format::eR8G8B8A8Srgb:
                    foundRGBA_SRGB = true;
                    break;
                case vk::Format::eB8G8R8A8Srgb:
                    foundBGRA_SRGB = true;
                    break;
                case vk::Format::eR8G8B8A8Unorm:
                    foundRGBA_UNORM = true;
                    break;
                case vk::Format::eB8G8R8A8Unorm:
                    foundBGRA_UNORM = true;
                    break;
                default:
                    break;
                }
            }
        }

        if (foundBGRA_SRGB) {
            return std::make_tuple(vk::Format::eB8G8R8A8Srgb, vk::ColorSpaceKHR::eSrgbNonlinear);
        } else if (foundRGBA_SRGB) {
            return std::make_tuple(vk::Format::eR8G8B8A8Srgb, vk::ColorSpaceKHR::eSrgbNonlinear);
        } else if (foundBGRA_UNORM) {
            return std::make_tuple(vk::Format::eB8G8R8A8Unorm, vk::ColorSpaceKHR::eSrgbNonlinear);
        } else if (foundRGBA_UNORM) {
            return std::make_tuple(vk::Format::eR8G8B8A8Unorm, vk::ColorSpaceKHR::eSrgbNonlinear);
        }

        return std::make_tuple(surfaceFormats[0].format, surfaceFormats[0].colorSpace);
    }

    vk::PresentModeKHR selectPreferredPresentMode(const std::shared_ptr<GContext> &gc, vk::SurfaceKHR surface) {
        auto presentModes = gc->getGpu().getSurfacePresentModesKHR(surface);
        for (const auto &pm : presentModes) {
            if (pm == vk::PresentModeKHR::eMailbox) {
                return vk::PresentModeKHR::eMailbox;
            }
        }

        return vk::PresentModeKHR::eFifo;
    }

    SurfaceRenderTarget::SurfaceRenderTarget(const std::shared_ptr<GContext> &gc, vk::SurfaceKHR surface, const SurfaceRenderTargetConfiguration &configuration)
        : m_GC(gc), m_TargetConfiguration(configuration) {
        m_Surface = surface;
        initialConfigure();
        createSwapchain();
    }

    SurfaceRenderTarget::SurfaceRenderTarget(const std::shared_ptr<GContext> &gc, const std::shared_ptr<ISurfaceProvider> &surfaceProvider,
                                             const SurfaceRenderTargetConfiguration &configuration)
        : m_GC(gc), m_TargetConfiguration(configuration) {
        m_Surface = surfaceProvider->getOrCreateSurface();
        initialConfigure();
        createSwapchain();
    }

    void SurfaceRenderTarget::resizeTarget(const vk::Extent2D &newSize) {}

    vk::Image SurfaceRenderTarget::getImageTarget(uint32_t index) const {
        return m_Images[index];
    }

    vk::ImageView SurfaceRenderTarget::getImageViewTarget(uint32_t index) const {
        return m_ImageViews[index];
    }

    void SurfaceRenderTarget::initialConfigure() {
        std::tie(m_Configuration.format, m_ColorSpace) = selectPreferredSurfaceFormat(m_GC, m_Surface);
        m_PresentMode                                  = selectPreferredPresentMode(m_GC, m_Surface /* TODO: non-vsync */);
    }

    void SurfaceRenderTarget::createSwapchain() {
        const vk::SwapchainKHR old = m_Swapchain;

        auto capabilities = m_GC->getGpu().getSurfaceCapabilitiesKHR(m_Surface);

        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            m_Configuration.extent = capabilities.currentExtent;
        } else {
            m_Configuration.extent = neuron::math::clamp(m_SizeProvider(), capabilities.minImageExtent, capabilities.maxImageExtent);
        }

        uint32_t minImageCount = capabilities.minImageCount + 1;
        if (capabilities.maxImageCount > 0 && minImageCount > capabilities.maxImageCount) {
            minImageCount = capabilities.maxImageCount;
        }

        m_Swapchain = m_GC->getDevice().createSwapchainKHR(
            vk::SwapchainCreateInfoKHR({}, m_Surface, minImageCount, m_Configuration.format, m_ColorSpace, m_Configuration.extent, 1, m_TargetConfiguration.desiredImageUsage,
                                       vk::SharingMode::eExclusive, {}, capabilities.currentTransform, vk::CompositeAlphaFlagBitsKHR::eOpaque, m_PresentMode, true, old));

        m_GC->getDevice().waitIdle();

        for (const auto &iv : m_ImageViews)
            m_GC->getDevice().destroy(iv);
        m_GC->getDevice().destroy(old);

        m_Images = m_GC->getDevice().getSwapchainImagesKHR(m_Swapchain);

        m_ImageViews.resize(m_Images.size());
        for (size_t i = 0; i < m_Images.size(); i++) {
            m_ImageViews[i] =
                m_GC->getDevice().createImageView(vk::ImageViewCreateInfo({}, m_Images[i], vk::ImageViewType::e2D, m_Configuration.format, STANDARD_COMPONENT_MAPPING, BASIC_ISR));
        }
    }

    SurfaceRenderTarget::~SurfaceRenderTarget() {
        for (const auto &iv : m_ImageViews)
            m_GC->getDevice().destroy(iv);
        if (m_Swapchain)
            m_GC->getDevice().destroy(m_Swapchain);
    }


} // namespace neuron::graphics
