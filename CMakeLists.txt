cmake_minimum_required(VERSION 3.5.1) 
project(ProjectTails)

file(GLOB SOURCES "src/*.cpp")

include_directories(${ProjectTails_SOURCE_DIR}/include /usr/include/SDL2)

set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(GCC_COVERAGE_COMPILE_FLAGS "-std=c++1z -march=native -mtune=native -fconcepts")

add_definitions(${GCC_COVERAGE_COMPILE_FLAGS})

add_executable(ProjectTails ${SOURCES})
target_link_libraries(${PROJECT_NAME} SDL2 SDL2_image SDL2_mixer stdc++fs)
add_test(NAME testall COMMAND ${CMAKE_CURRENT_BINARY_DIR}/ProjectTails -test)

enable_testing()
