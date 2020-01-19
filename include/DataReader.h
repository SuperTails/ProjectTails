#pragma once
#include "Typedefs.h"
#include "Constants.h"
#include "PhysicsEntity.h"
#include <string_view>
#include <json.hpp>
#include <experimental/filesystem>
#include <iosfwd>

using json = nlohmann::json;

class Ground;

namespace DataReader
{
	using namespace std::experimental;
	/*
	Path, act number, name,
	entities, winArea,
	actType, *ground,
	*blocks, *blockFlags,
	*collides, *collideFlags
	*groundIndices, *levelSize
	*/


	void LoadEntityData(const std::string& path);
	void LoadJSONBlock(const std::string& path);
	std::vector< std::vector< Animation > > LoadBackground(const filesystem::path& directory);
	void LoadLevelBlocks(const std::string& path); 
	/**
	* Tile data (height and angle) is loaded from a file into a vector < CollisionTile >
	* Tiles are arranged into GROUND_PIXEL_WIDTH x GROUND_PIXEL_WIDTH pixel 'blocks' (vector < int > referring to the indices)
	* Ground is initialized from those blocks
	* Act has a vector < Ground > 
	**/
};

