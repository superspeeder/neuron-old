#include <neuron/neuron.hpp>
#include <neuron/os/window.hpp>
#include <neuron/graphics/gcontext.hpp>

int main() {
    neuron::init(neuron::Settings{"Neuron Example Application", neuron::utils::Version{0, 1, 0}, false, true, false});
    {
        auto gc = std::make_shared<neuron::graphics::GContext>(neuron::graphics::GCSettings{
            {
                neuron::graphics::QueueRequest{neuron::graphics::QueueType::Transfer, 1},
            },
            {},
        });

        auto queue = gc->getQueue(neuron::graphics::QueueType::Transfer, 0);

        auto window = std::make_shared<neuron::os::Window>(neuron::os::WindowSettings{"Window", {800, 600}, false});

        auto surfaceTarget = std::make_shared<neuron::graphics::SurfaceRenderTarget>(gc, window, neuron::graphics::SurfaceRenderTargetConfiguration{});

        while (!window->shouldClose()) {
            neuron::os::pollEvents();
        }
    }

    neuron::cleanup();
}
