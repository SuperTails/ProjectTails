cmake_minimum_required(VERSION 3.5.1) 
project(ProjectTails)

include_directories(ProjectTails ~/Documents/SDL2_image-2.0.1 ~/Documents/SDL2_mixer-2.0.1)

file(GLOB SOURCES "ProjectTails/src/*.cpp")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${ProjectTails_SOURCE_DIR}/cmake")

INCLUDE(FindPkgConfig)

find_package (SDL2)
find_package (SDL2_image)
find_package (SDL2_mixer)

include_directories(${ProjectTails_SOURCE_DIR}/ProjectTails/include ${SDL2_INCLUDE_DIRS} ${SDL2_IMAGE_INCLUDE_DIRS} ${SDL2_MIXER_INCLUDE_DIRS})

set (CMAKE_BUILD_TYPE Debug)

set (CMAKE_EXPORT_COMPILE_COMMANDS ON)

set (GCC_COVERAGE_COMPILE_FLAGS "-std=c++1z -march=native -mtune=native -fconcepts")

add_definitions(${GCC_COVERAGE_COMPILE_FLAGS})

configure_file(CMakeLists.txt.in googletest-download/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
	RESULT_VARIABLE result
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
if(result)
	message(FATAL_ERROR "CMake step for googletest failed: ${result}")
endif()
execute_process(COMMAND ${CMAKE_COMMAND} --build .
	RESULT_VARIABLE result
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
if(result)
	message(FATAL_ERROR "Build step for googletest failed: ${result}")
endif()

# Prevent overriding the parent project's compiler/linker
# settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Add googletest directly to our build. This defines
# the gtest and gtest_main targets.
add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/googletest-src
                 ${CMAKE_CURRENT_BINARY_DIR}/googletest-build
                 EXCLUDE_FROM_ALL)

# The gtest/gtest_main targets carry header search path
# dependencies automatically when using CMake 2.8.11 or
# later. Otherwise we have to add them here ourselves.
if (CMAKE_VERSION VERSION_LESS 2.8.11)
	include_directories("${gtest_SOURCE_DIR}/include")
endif()

add_executable(ProjectTails ${SOURCES})
target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES} ${SDL2_MIXER_LIBRARIES} stdc++fs gtest)
add_test(NAME testall COMMAND ${CMAKE_CURRENT_BINARY_DIR}/ProjectTails -test)

enable_testing()
