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

	extern bool levelEditing;

	extern std::string levelPath;

	//Contains indices into groundList, organized by levelBlocks[x][y]
	extern std::vector < std::vector < Ground > > levelBlocks;

	//Maps entity key to image for rendering
	extern std::unordered_map < std::string, Animation > entityView;

	//Stores data about one entity object
	extern std::vector < PhysStruct > levelEntities;

	extern std::vector < PhysStruct >::iterator currentEntity;

	extern bool mouseDebounce;

	extern editMode mode;

	extern SDL_Surface* Sky;

	extern SDL_Texture* SkyTexture;

	extern Camera* cam;

	extern double mouseWheelValue;

	void init(std::vector < Ground >& levelGround, SDL_Point levelSize);

	void renderTiles();

	void renderEntities();

	void renderText();

	bool handleInput();

	std::string convertToString(int levelNumber, std::string levelName);

	std::string mtos(editMode m);
}
