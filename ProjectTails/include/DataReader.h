#pragma once
#include "Typedefs.h"
#include "PhysicsEntity.h"
#include <string>
#include <json.hpp>
#include <experimental/filesystem>

using json = nlohmann::json;

enum class ActType : unsigned char;

class Ground;
class CollisionTile;

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

	void LoadActData(const std::string& path, int& n, std::string& name1, std::vector < PhysStruct >& entities, SDL_Rect& winArea, ActType& actType, std::vector < Ground >& ground, SDL_Point& levelSize);
	void LoadEntityData(const std::string& path);
	void LoadTileData(const std::string& path, std::vector < CollisionTile >& tiles);
	void LoadTileData(std::vector < CollisionTile >& tiles, matrix < int >& heights, std::vector < double >& angles);
	void LoadJSONBlock(const std::string& path);
	void LoadCollisionsFromImage(const std::string& path, matrix < int >& heights, std::vector < double >& angles);
	void LoadBackground(const filesystem::path& directory, std::vector < std::vector < Animation > >& background);
	void LoadLevelBlocks(const std::string& path); 
	/**
	* Tile data (height and angle) is loaded from a file into a vector < CollisionTile >
	* Tiles are arranged into GROUND_PIXEL_WIDTH x GROUND_PIXEL_WIDTH pixel 'blocks' (vector < int > referring to the indices)
	* Ground is initialized from those blocks
	* Act has a vector < Ground > 
	**/
};

