#pragma once
#include <fstream>
#include <string>
#include <SDL.h>
#include <unordered_map>
#include "Ground.h"
#include "Camera.h"
#include "PhysicsEntity.h"
#include "Act.h"
#include "DataReader.h"
#include "Camera.h"
#include "Miscellaneous.h"
#include "Text.h"
#include "Typedefs.h"

namespace LevelEditor {
	enum editMode { TILE, ENTITY };

	struct groundData {
		int index;
		bool flip;
		groundData() : index(-1), flip(false) {};
		groundData(int ind, bool f) : index(ind), flip(f) {};
	};

	extern const bool levelEditing;

	extern std::string levelPath;

	//Contains all blocks defined for act (one for each json file)
	extern std::vector < Ground > groundList;

	//Contains indices into groundList, organized by levelBlocks[x][y]
	extern std::vector < std::vector < LevelEditor::groundData > > levelBlocks;

	//Maps entity key (e.g. YELLOW_SPRING) to properties
	extern std::unordered_map < std::string, PhysProp* >* entityList;

	//List of all possible keys
	extern std::vector < std::string >* entityTypes;

	//Maps entity key to image for rendering
	extern std::unordered_map < std::string, Animation > entityView;

	//Stores data about one entity object
	extern std::vector < PhysStructInit > levelEntities;

	extern std::vector < PhysStructInit >::iterator currentEntity;

	extern bool mouseDebounce;

	extern editMode mode;

	extern SDL_Surface* Sky;

	extern SDL_Texture* SkyTexture;

	extern Camera* cam;

	extern double mouseWheelValue;

	void init(std::vector < DataReader::groundData >& levelGround, SDL_Point levelSize, matrix < int >& blocks, matrix < int >& blockFlags, matrix < int >& collides, matrix < int >& collideFlags);

	void renderTiles();

	void renderEntities();

	void renderText();

	bool handleInput();

	std::string convertToString(int levelNumber, std::string levelName);

	std::string mtos(editMode m);
}
