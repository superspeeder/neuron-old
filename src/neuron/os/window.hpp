#pragma once

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <string>

#include "neuron/graphics/gcontext.hpp"

namespace neuron::os {

    struct WindowSettings {
        std::string title;
        glm::uvec2  size;
        bool        resizable = false;
    };

    class Window : public neuron::graphics::ISurfaceProvider {
      public:
        explicit Window(const WindowSettings &settings);
        virtual ~Window();

        vk::SurfaceKHR getOrCreateSurface() override;
        vk::SurfaceKHR getSurfaceIfAvailable() override;

        [[nodiscard]] bool shouldClose() const;

      private:
        GLFWwindow    *m_Window;
        vk::SurfaceKHR m_Surface;
    };

    void pollEvents();

} // namespace neuron::os
