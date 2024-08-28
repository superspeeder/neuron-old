#pragma once

#include <vulkan/vulkan.hpp>

#include <algorithm>

namespace neuron::math {
    constexpr vk::Extent2D clamp(const vk::Extent2D &value, const vk::Extent2D &minv, const vk::Extent2D &maxv) {
        return {std::clamp(value.width, minv.width, maxv.width), std::clamp(value.height, minv.height, maxv.height)};
    };
} // namespace neuron::math
