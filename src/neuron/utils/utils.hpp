#pragma once

#include <cinttypes>

#include <vulkan/vulkan.hpp>

namespace neuron::utils {
    struct Version {
        uint32_t major, minor, patch;

        [[nodiscard]] constexpr uint32_t toUintVk() const {
            return vk::makeApiVersion(0U, major, minor, patch);
        };
    };

    template<typename T> [[nodiscard]] T* allocateAndFillArray(size_t size, T value) {
        T* arr = new T[size];
        std::fill(arr, arr+size, value);
        return arr;
    };

}
