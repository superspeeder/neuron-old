#pragma once

#include "neuron/neuron.hpp"
#include "neuron/utils/utils.hpp"

#include <functional>
#include <memory>
#include <optional>

namespace neuron::graphics {

    constexpr vk::ComponentMapping      STANDARD_COMPONENT_MAPPING = {vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG, vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eA};
    constexpr vk::ImageSubresourceRange BASIC_ISR                  = {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1};

    enum class QueueType {
        Primary,
        Transfer,
        Compute,
        VideoEncode,
        VideoDecode,
    };

    struct QueueRequest {
        QueueType type;
        uint32_t  count;
    };

    struct GCSettings {
        std::vector<QueueRequest> queueRequests;
        std::vector<const char *> requestedExtensions;
    };

    /**
     *
     * GContext is the actual connection to the GPU. This is required for all rendering operations you want to do.
     *
     */
    class GContext final {
      public:
        explicit GContext(const GCSettings &settings = {});
        ~GContext();

        [[nodiscard]] inline const vk::PhysicalDevice &getGpu() const { return m_Gpu; }

        [[nodiscard]] inline const vk::Device &getDevice() const { return m_Device; }

        [[nodiscard]] std::optional<vk::Queue> getQueue(QueueType type, uint32_t index = 0) const;
        [[nodiscard]] std::optional<uint32_t>  getQueueFamily(QueueType type) const;

        [[nodiscard]] vk::Queue getPrimaryQueue() const;


      private:
        vk::PhysicalDevice m_Gpu;
        vk::Device         m_Device;

        uint32_t                m_PrimaryQueueFamily = 0;
        std::optional<uint32_t> m_TransferQueueFamily;
        std::optional<uint32_t> m_ComputeQueueFamily;
        std::optional<uint32_t> m_VideoEncodeQueueFamily;
        std::optional<uint32_t> m_VideoDecodeQueueFamily;

        vk::Queue m_PrimaryQueue;
    };

    struct RenderTargetConfiguration {
        vk::Extent2D extent;
        vk::Format   format;
    };

    /**
     *
     * Interface class for render targets. Makes using the render systems much easier (no custom bs specifically for rendering to an image instead of the screen)
     *
     */
    class IRenderTarget {
      public:
        /**
         * This is called whenever the rendering target must be resized (for example, when the surface of a SurfaceRenderingTarget is resized)
         *
         * @param newSize The new size.
         */
        virtual void resizeTarget(const vk::Extent2D &newSize) = 0;

        [[nodiscard]] virtual vk::Image     getImageTarget(uint32_t index) const     = 0;
        [[nodiscard]] virtual vk::ImageView getImageViewTarget(uint32_t index) const = 0;

        [[nodiscard]] inline vk::Image getImageTarget() const { return getImageTarget(0); };

        [[nodiscard]] inline vk::ImageView getImageViewTarget() const { return getImageViewTarget(0); };

        /**
         * This tells you if there is multiple images underlying in the target. This is commonly the case for render targets that make up stages of the frame rendering, or the
         * surface target itself. Generally, single buffered render targets are useful if you are going to render an image once or periodically (not each frame).
         *
         * @return
         */
        [[nodiscard]] virtual bool isMultiBuffered() const noexcept = 0;

        [[nodiscard]] inline const RenderTargetConfiguration &getCurrentConfiguration() const noexcept { return m_Configuration; };

      protected:
        RenderTargetConfiguration m_Configuration;
    };

    class ISurfaceProvider {
      public:
        virtual vk::SurfaceKHR getOrCreateSurface()    = 0;
        virtual vk::SurfaceKHR getSurfaceIfAvailable() = 0;
    };

    struct SurfaceRenderTargetConfiguration {
        vk::ImageUsageFlags desiredImageUsage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst;
    };

    class SurfaceRenderTarget final : public IRenderTarget {
      public:
        SurfaceRenderTarget(const std::shared_ptr<GContext> &gc, vk::SurfaceKHR surface, const SurfaceRenderTargetConfiguration &configuration = {});
        SurfaceRenderTarget(const std::shared_ptr<GContext> &gc, const std::shared_ptr<ISurfaceProvider> &surfaceProvider,
                            const SurfaceRenderTargetConfiguration &configuration = {});

        virtual ~SurfaceRenderTarget();

        [[nodiscard]] inline vk::SwapchainKHR getSurfaceSwapchain() const noexcept { return m_Swapchain; };

        [[nodiscard]] inline vk::PresentModeKHR getSurfacePresentMode() const noexcept { return m_PresentMode; };

        [[nodiscard]] inline vk::ColorSpaceKHR getSurfaceColorSpace() const noexcept { return m_ColorSpace; };

        void                        resizeTarget(const vk::Extent2D &newSize) override;
        [[nodiscard]] vk::Image     getImageTarget(uint32_t index) const override;
        [[nodiscard]] vk::ImageView getImageViewTarget(uint32_t index) const override;

        [[nodiscard]] inline bool isMultiBuffered() const noexcept override { return m_Images.size() > 1; };

        static constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

      private:
        std::shared_ptr<GContext> m_GC;

        vk::SurfaceKHR m_Surface;

        vk::ColorSpaceKHR  m_ColorSpace;
        vk::PresentModeKHR m_PresentMode;

        vk::SwapchainKHR           m_Swapchain;
        std::vector<vk::Image>     m_Images;
        std::vector<vk::ImageView> m_ImageViews;

        uint32_t m_CurrentFrame = 0;

        std::function<vk::Extent2D()> m_SizeProvider;

        SurfaceRenderTargetConfiguration m_TargetConfiguration;

        void initialConfigure();
        void createSwapchain();
    };

} // namespace neuron::graphics
