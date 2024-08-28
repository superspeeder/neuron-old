add_executable(neuron_example src/main.cpp
        src/main.cpp)
target_include_directories(neuron_example PRIVATE src/)
target_link_libraries(neuron_example PRIVATE neuron::neuron)

add_executable(neuron::example ALIAS neuron_example)