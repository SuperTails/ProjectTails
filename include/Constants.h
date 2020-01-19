#pragma once
#include <cstddef>
#include <string>
#define WINDOW_HORIZONTAL_SIZE 1024
#define WINDOW_VERTICAL_SIZE 512
#define ASSET "../asset/"
#define BADNIK ASSET"Badnik/"
#define TAILS_PATH ASSET"Tails/"

namespace constants {
	constexpr const std::size_t asset_path_length = sizeof(ASSET) / sizeof(char);
	constexpr const std::size_t TILE_WIDTH = 16;
	constexpr const std::size_t GROUND_WIDTH = 8;
	constexpr const std::size_t GROUND_SIZE = GROUND_WIDTH * GROUND_WIDTH;
	constexpr const std::size_t GROUND_PIXEL_WIDTH = GROUND_WIDTH * TILE_WIDTH;

	//Asset paths
	using namespace std::literals::string_literals;
	const std::string MUSIC_PATH = ASSET"WorldToExplore.wav"s;
	const std::string SKY_PATH = ASSET"Sky.png"s;
	const std::string RING_PATH = ASSET"Ring.png"s;
	const std::string RING_SPARKLE_PATH = ASSET"RingSparkle.png"s;
	const std::string FONT_PATH = ASSET"FontGUI.png"s;
	const std::string EXPLOSION_PATH = ASSET"Explosion.png"s;
	const std::string ROCKET_PATH = ASSET"Rocket.png"s;
	const std::string ENTITY_DATA_PATH = ASSET"EntityData.txt"s;
	const std::string LIVES_PATH = ASSET"Lives.png"s;

	
	const std::string ACT1_DATA_PATH = ASSET"Act1"s;
	const std::string ACT2_DATA_PATH = ASSET"Act2"s;
};
