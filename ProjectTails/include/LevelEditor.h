#pragma once
#include "Miscellaneous.h"
#include "Text.h"
#include "Typedefs.h"
#include "Act.h"
#include "Shapes.h"
#include <fstream>
#include <string>
#include <SDL.h>
#include <unordered_map>
#include <experimental/filesystem>

class Ground;

class LevelEditor {
public:
	enum editMode { TILE, ENTITY };

	//LevelEditor(const std::vector< Ground >& levelGround, SDL_Point levelSize, Camera* camera, const std::vector< PhysStruct >& entities);
	LevelEditor(Act act, Camera* camera);

	static bool levelEditing;

	std::string levelPath;
	
	Act level;

	// Maps entity key to image for rendering
	std::unordered_map< std::string, Animation > entityView;

	std::vector< std::unique_ptr< PhysicsEntity > >::iterator currentEntity;

	editMode mode = editMode::TILE;

	double mouseWheelValue = 0.0;

	void render();

	void renderTiles();

	bool handleInput();

	void save(const std::experimental::filesystem::path& path) const;

	std::string convertToString(int levelNumber, const std::string& levelName, const std::string& blockPrefix, const std::string& backgroundFolder) const;

private:
	Camera* cam = nullptr;

	Sprite sky{};

	std::optional< std::pair< SDL_Point, std::size_t > > mouseDrag{};

	void renderEntities() const;

	void renderText() const;

	std::vector< char > parseFlags(const std::string& s) const;

	std::size_t addGroundIndex(std::size_t current, int addend);
};

std::string to_string(LevelEditor::editMode m);

std::vector< std::vector< Ground > > organizeBlocks(const std::vector< Ground >& blocks, SDL_Point levelSize);
