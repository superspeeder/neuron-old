
include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/b514bdc898e2951020cbdca1304b75f5950d1f59.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(neuron_unit_tests neuron/tests/unit/basic_unit.cpp)
target_include_directories(neuron_unit_tests PRIVATE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(neuron_unit_tests PUBLIC neuron::neuron GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(neuron_unit_tests)


add_executable(neuron::unit_tests ALIAS neuron_unit_tests)