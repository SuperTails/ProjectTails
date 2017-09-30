#pragma once
#include <cstddef>
#include <string>
#include <array>
#include "Animation.h"
#define WINDOW_HORIZONTAL_SIZE 1024
#define WINDOW_VERTICAL_SIZE 512
#define ASSET "/home/super-tails/Documents/ProjectTails/asset/"
#define BADNIK ASSET"Badnik/"
#define TAILS_PATH ASSET"Tails/"

namespace constants {
	const std::size_t TILE_WIDTH = 16;
	const std::size_t GROUND_WIDTH = 8;
	const std::size_t GROUND_SIZE = GROUND_WIDTH * GROUND_WIDTH;
	const std::size_t GROUND_PIXEL_WIDTH = GROUND_WIDTH * TILE_WIDTH;

	//Asset paths
	const std::string MUSIC_PATH = ASSET"WorldToExplore.wav";
	const std::string SKY_PATH = ASSET"Sky.png";
	const std::string RING_PATH = ASSET"Ring.png";
	const std::string RING_SPARKLE_PATH = ASSET"RingSparkle.png";
	const std::string FONT_PATH = ASSET"FontGUI.png";
	const std::string EXPLOSION_PATH = ASSET"Explosion.png";
	const std::string ROCKET_PATH = ASSET"Rocket.png";
	const std::string ENTITY_DATA_PATH = ASSET"EntityData.txt";
	const std::string LIVES_PATH = ASSET"Lives.png";

	
	const std::string ACT1_DATA_PATH = ASSET"Act1";
	const std::string ACT2_DATA_PATH = ASSET"Act2";
};
