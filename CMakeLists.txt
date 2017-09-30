cmake_minimum_required(VERSION 3.5.1) 
project(ProjectTails)

include_directories(ProjectTails ~/Documents/SDL2_image-2.0.1 ~/Documents/SDL2_mixer-2.0.1)

file(GLOB SOURCES "ProjectTails/*.cpp")

add_executable(ProjectTails ${SOURCES})

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${ProjectTails_SOURCE_DIR}/cmake")

INCLUDE(FindPkgConfig)

#PKG_SEARCH_MODULE(SDL2 REQUIRED sdl2)
#PKG_SEARCH_MODULE(SDL2IMAGE REQUIRED SDL2_image>=2.0.0)
#PKG_SEARCH_MODULE(SDL2MIXER REQUIRED SDL2_mixer>=2.0.0)

find_package (SDL2)
find_package (SDL2_image)
find_package (SDL2_mixer)

include_directories(${SDL2_INCLUDE_DIRS} ${SDL2_IMAGE_INCLUDE_DIRS} ${SDL2_MIXER_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES} ${SDL2_MIXER_LIBRARIES} stdc++fs)

set (CMAKE_BUILD_TYPE Debug)

set (CMAKE_EXPORT_COMPILE_COMMANDS ON)

set (GCC_COVERAGE_COMPILE_FLAGS "-std=c++1z")

add_definitions(${GCC_COVERAGE_COMPILE_FLAGS})
