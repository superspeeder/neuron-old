#include "gtest/gtest.h"

#include "neuron/neuron.hpp"
#include "neuron/utils/utils.hpp"

TEST(Utils, VersionToUintVk) {
    constexpr neuron::utils::Version A{1,0,0};
    constexpr neuron::utils::Version B{1,1,0};
    constexpr neuron::utils::Version C{1,2,0};
    constexpr neuron::utils::Version D{1,3,0};
    EXPECT_EQ(A.toUintVk(), VK_API_VERSION_1_0);
    EXPECT_EQ(B.toUintVk(), VK_API_VERSION_1_1);
    EXPECT_EQ(C.toUintVk(), VK_API_VERSION_1_2);
    EXPECT_EQ(D.toUintVk(), VK_API_VERSION_1_3);
}

struct AAFA_TestType {
    float a;
    int b;
};

TEST(Utils, AllocateAndFillArray) {
    auto* arr1 = neuron::utils::allocateAndFillArray<uint32_t>(5, 0x4d);
    EXPECT_EQ(arr1[0], 0x4d);
    EXPECT_EQ(arr1[1], 0x4d);
    EXPECT_EQ(arr1[2], 0x4d);
    EXPECT_EQ(arr1[3], 0x4d);
    EXPECT_EQ(arr1[4], 0x4d);
    delete[] arr1;

    AAFA_TestType value{3.14f, 0xFB};
    auto* arr2 = neuron::utils::allocateAndFillArray<AAFA_TestType>(3, value);
    EXPECT_EQ(arr2[0].a, 3.14f);
    EXPECT_EQ(arr2[0].b, 0xFB);
    EXPECT_EQ(arr2[1].a, 3.14f);
    EXPECT_EQ(arr2[1].b, 0xFB);
    EXPECT_EQ(arr2[2].a, 3.14f);
    EXPECT_EQ(arr2[2].b, 0xFB);
    delete[] arr2;
}
