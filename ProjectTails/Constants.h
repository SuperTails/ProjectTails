#pragma once
#include <cstddef>
#define WINDOW_HORIZONTAL_SIZE 1024
#define WINDOW_VERTICAL_SIZE 512
#define ASSET "..\\..\\asset\\"
#define BADNIK ASSET"Badnik\\"

namespace constants {
	const std::size_t TILE_WIDTH = 16;
	const std::size_t GROUND_WIDTH = 8;
	const std::size_t GROUND_SIZE = GROUND_WIDTH * GROUND_WIDTH;
	const std::size_t GROUND_PIXEL_WIDTH = GROUND_WIDTH * TILE_WIDTH;
	const std::size_t NUM_BLOCKS = 14;
};