#pragma once


#include <vulkan/vulkan.hpp>

#include "neuron/utils/utils.hpp"

#include <optional>

namespace neuron {

    constexpr utils::Version   VERSION = {NEURON_VERSION_MAJOR, NEURON_VERSION_MINOR, NEURON_VERSION_PATCH};
    constexpr std::string_view NAME    = "Neuron";

    struct Settings {
        std::string            name;
        neuron::utils::Version version;

        bool offscreenRenderingOnly = false;
        bool debugMode              = false;
        bool vulkanApiDump          = false;
    };

    void init(const Settings &settings = {});

    void cleanup();

    /**
     *
     * Context is the container for all things that should only exist once
     *
     * For example, this will initialize the vulkan instance & debug messenger.
     * This will also contain references to the main engine loggers.
     *
     * see neuron::graphics::GContext for an actual rendering context.
     *
     */
    class Context {
      public:
        [[nodiscard]] inline vk::Instance getInstance() const noexcept { return m_Instance; }

        [[nodiscard]] inline const std::optional<vk::DebugUtilsMessengerEXT> &getDebugMessenger() const { return m_DebugMessenger; }

        ~Context();

        static Context* get() noexcept;

      private:
        explicit Context(const Settings &settings);

        friend void init(const Settings &settings);

        vk::Instance                              m_Instance;
        std::optional<vk::DebugUtilsMessengerEXT> m_DebugMessenger;
    };
} // namespace neuron
