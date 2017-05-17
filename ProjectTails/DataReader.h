#pragma once
#include <string>
#include "Act.h"
#include "Ground.h"
#include <fstream>
#include <json.hpp>
#include <iomanip>
#include <unordered_map>
#include "Typedefs.h"

using json = nlohmann::json;

enum ActType;

namespace DataReader
{
	struct groundData {
		int x;
		int y;
		int index;
		bool flip;
	};
	/*
	Path, act number, name,
	entities, winArea,
	actType, *ground,
	*blocks, *blockFlags,
	*collides, *collideFlags
	*groundIndices, *levelSize
	*/
	void LoadActData(std::string path, int& n, std::string& name1, std::vector < PhysStructInit >& entities, SDL_Rect& winArea, ActType& actType, std::vector < Ground >* ground, std::vector < Ground::groundArrayData >* = nullptr, std::vector < groundData >* groundIndices = nullptr, SDL_Point* levelSize = nullptr);
	void LoadEntityData(std::string path, std::vector < PhysProp >& prop, std::unordered_map < std::string, PhysProp* >& entityKeys, std::vector < std::string >& Types);
	void LoadTileData(std::string path, std::vector < CollisionTile >& tiles);
	void LoadTileData(std::vector < CollisionTile >& tiles, matrix < int >& heights, std::vector < double >& angles);
	void LoadJSONBlock(std::string path, Ground::groundArrayData& arrayData);
	void LoadCollisionsFromImage(std::string path, matrix < int >& heights, std::vector < double >& angles);
	void LoadBackground(std::string path, std::vector < std::vector < Animation > >& background, int numTiles);

	/**
	* Tile data (height and angle) is loaded from a file into a vector < CollisionTile >
	* Tiles are arranged into 256x256 pixel 'blocks' (vector < int > referring to the indices)
	* Ground is initialized from those blocks
	* Act has a vector < Ground > 
	**/
};

