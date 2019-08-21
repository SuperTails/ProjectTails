#pragma once
#include "Ground.h"
#include "Sprite.h"
#include "Text.h"
#include "CollisionTile.h"
#include <array>

class Layer;
class Camera;
struct SDL_Point;

class BlockEditor {
	using GroundData = Ground::GroundData;
public:
	void render(const Camera& camera);

	CollisionTile& tileAt(Ground::Layer& layer, SDL_Point pos) const;

	const CollisionTile& tileAt(const Ground::Layer& layer, SDL_Point pos) const;

	void update(const Camera& camera);

	void save();

	static bool editing;
private:
	GroundData& getBlock(std::size_t index) const;
	GroundData& currentBlock() const;

	static void updateBlock(GroundData& block);

	void addBlock();

	bool editBlock(SDL_Rect mouseRect, Uint32 mouseState, bool mouseDebounce);

	void selectTile(SDL_Rect mouseRect, Uint32 mouseState, bool mouseDebounce);

	void updateFlags();

	void renderFlags() const;

	Sprite groundMap{ Ground::map };

	std::array< std::array< std::array< Text, GROUND_WIDTH >, GROUND_WIDTH >, 2 > flagsText;

	bool flagMode = false;

	bool mouseDebounce = false;

	std::size_t currentBlockIndex = 0;
	CollisionTile selectedTile = { 0, 0 };

	double xMouseError = 0.0;
	double yMouseError = 0.0;
};
