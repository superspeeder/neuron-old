#include "window.hpp"

namespace neuron::os {
    Window::Window(const WindowSettings &settings) {
        glfwDefaultWindowHints();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, settings.resizable);

        m_Window = glfwCreateWindow((int)settings.size.x, (int)settings.size.y, settings.title.c_str(), nullptr, nullptr);
    }

    vk::SurfaceKHR Window::getOrCreateSurface() {
        if (!m_Surface) {
            VkSurfaceKHR s;
            glfwCreateWindowSurface(Context::get()->getInstance(), m_Window, nullptr, &s);
            m_Surface = s;
        }

        return m_Surface;
    }

    vk::SurfaceKHR Window::getSurfaceIfAvailable() {
        return m_Surface;
    }

    bool Window::shouldClose() const {
        return glfwWindowShouldClose(m_Window);
    }

    Window::~Window() {
        if (m_Surface)
            Context::get()->getInstance().destroy(m_Surface);
        glfwDestroyWindow(m_Window);
    }

    void pollEvents() {
        glfwPollEvents();
    }
} // namespace neuron::os
