#include "BlockEditor.h"
#include "Text.h"
#include "InputComponent.h"
#include "Ground.h"
#include "Camera.h"
#include "json.hpp"
#include "CollisionTile.h"
#include "Functions.h"
#include <fstream>
#include <experimental/filesystem>

bool BlockEditor::editing = false;
	
static const auto lMouseMask = SDL_BUTTON(SDL_BUTTON_LEFT);
static const auto mMouseMask = SDL_BUTTON(SDL_BUTTON_MIDDLE);
static const auto rMouseMask = SDL_BUTTON(SDL_BUTTON_RIGHT);

void BlockEditor::render(const Camera& camera) {
	auto drawRect = [](SDL_Rect dest, bool filled, Uint8 r, Uint8 g, Uint8 b, Uint8 a = SDL_ALPHA_OPAQUE) {
		SDL_SetRenderDrawColor(globalObjects::renderer, r, g, b, a);
		if (filled) {
			SDL_RenderFillRect(globalObjects::renderer, &dest);
		}
		else {
			SDL_RenderDrawRect(globalObjects::renderer, &dest);
		}
	};

	drawRect({ 0, 0, WINDOW_HORIZONTAL_SIZE, WINDOW_VERTICAL_SIZE }, true, 100, 100, 255);

	const int width = GROUND_PIXEL_WIDTH;
	currentBlock().layers[0].Render({ 0    , 0 }, 0, nullptr, camera.scale);
	currentBlock().layers[1].Render({ width, 0 }, 0, nullptr, camera.scale); 

	groundMap.render(SDL_Rect{ width * 2, 0, groundMap.size().x / 2, groundMap.size().y / 2 } * camera.scale);
	drawRect(SDL_Rect{ width * 2, 0, groundMap.size().x / 2, groundMap.size().y / 2 } * camera.scale, false, 0, 0, 0);

	SDL_Point mousePos;
	const auto mouseState = SDL_GetMouseState(&mousePos.x, &mousePos.y);
	mousePos /= camera.scale;

	const int tilesPerRow = groundMap.size().x / TILE_WIDTH;
	const auto tilePos = SDL_Point{ (selectedTile.getIndex() % tilesPerRow), (selectedTile.getIndex() / tilesPerRow) } * TILE_WIDTH + SDL_Point{ 4 * width, 0 };
	drawRect(SDL_Rect{ tilePos.x, tilePos.y, TILE_WIDTH, TILE_WIDTH }, false, 0, 0, 0);

	const SDL_Rect mouseRect{ mousePos.x, mousePos.y, 1, 1 };	
	const SDL_Rect blockArea{ 0, 0, 2 * GROUND_PIXEL_WIDTH, GROUND_PIXEL_WIDTH };
	if (SDL_HasIntersection(&mouseRect, &blockArea) && !flagMode) {
		Surface flipH  = Animation::FlipSurface(groundMap.getSpriteSheet(), SDL_FLIP_HORIZONTAL);
		Surface flipV  = Animation::FlipSurface(groundMap.getSpriteSheet(), SDL_FLIP_VERTICAL);
		Surface flipHV = Animation::FlipSurface(groundMap.getSpriteSheet(), SDL_RendererFlip(SDL_FLIP_HORIZONTAL | SDL_FLIP_VERTICAL));

		const bool layerIndex = mouseRect.x >= GROUND_PIXEL_WIDTH;
		Animation temp = currentBlock().layers[layerIndex];

		{
		Surface::PixelLock lock1{ groundMap.getSpriteSheet() }, lock2{ flipH }, lock3{ flipV }, lock4{ flipHV };

		const auto& [map, src] = Ground::GroundData::getTileFromMap(selectedTile, groundMap.getSpriteSheet(), flipH, flipV, flipHV);

		SDL_BlendMode blendMode;
		SDL_GetSurfaceBlendMode(map.get(), &blendMode);
		SDL_SetSurfaceBlendMode(map.get(), SDL_BLENDMODE_NONE);
		
		const SDL_Point tilePos = (mousePos / TILE_WIDTH) * TILE_WIDTH;
		SDL_Rect dst{ int(tilePos.x % GROUND_PIXEL_WIDTH), tilePos.y, TILE_WIDTH, TILE_WIDTH };

		SDL_BlitSurface(map.get(), &src, temp.getSpriteSheet().get(), &dst);

		SDL_SetSurfaceBlendMode(map.get(), blendMode);
		}

		temp.updateTexture();

		drawRect(SDL_Rect{ width * layerIndex, 0, width, width } * camera.scale, true, 100, 100, 255);
		temp.Render({ width * layerIndex, 0 }, 0, nullptr, camera.scale);

		if (layerIndex == 1) {
			temp.Render                    ({ 0, width }, 0, nullptr, camera.scale);
			currentBlock().layers[0].Render({ 0, width }, 0, nullptr, camera.scale);
		}
		else {
			currentBlock().layers[1].Render({ 0, width }, 0, nullptr, camera.scale);
			temp.Render                    ({ 0, width }, 0, nullptr, camera.scale);
		}

	}
	else {
		currentBlock().layers[1].Render({ 0, width }, 0, nullptr, camera.scale);
		currentBlock().layers[0].Render({ 0, width }, 0, nullptr, camera.scale);
	}

	if (Ground::showCollision) {
		std::array< Animation, 2 > graphics = GroundData::convertTileGraphics(currentBlock().graphics);
		SDL_SetTextureAlphaMod(graphics[0].getTexture().get(), SDL_ALPHA_OPAQUE / 4);
		SDL_SetTextureAlphaMod(graphics[1].getTexture().get(), SDL_ALPHA_OPAQUE / 4);
		graphics[0].Render({ 0    , 0 }, 0, nullptr, camera.scale);
		graphics[1].Render({ width, 0 }, 0, nullptr, camera.scale);
	}

	if (flagMode) {
		renderFlags();
	}

	// Bounding boxes for the blocks
	drawRect(SDL_Rect{ 0    , 0, width, width } * camera.scale, false, 0, 0, 0);
	drawRect(SDL_Rect{ width, 0, width, width } * camera.scale, false, 0, 0, 0);

	text::renderAbsolute(SDL_Point{ width * 2 + 10, groundMap.size().y / 2 + 25 } * camera.scale, "GUI", "Index: " + std::to_string(currentBlockIndex));
	text::renderAbsolute(SDL_Point{ width * 2 + 10, groundMap.size().y / 2 + 5 } * camera.scale, "GUI",
			"Selected Tile: " + std::to_string(selectedTile.getIndex()));

	std::stringstream helpText;
	helpText << "WASD: Set flip  []: Change selected tile  f: Use flag mode\n"
	         << "m: Change graphics or collision  n: add block  r: copy block \nj: save  x: exit";
	text::renderAbsolute(SDL_Point{ 10, width + 10 } * camera.scale, "GUI", helpText.str());
}

CollisionTile& BlockEditor::tileAt(Ground::Layer& layer, SDL_Point pos) const {
	return layer[pos.x + pos.y * GROUND_WIDTH];
}

const CollisionTile& BlockEditor::tileAt(const Ground::Layer& layer, SDL_Point pos) const {
	return layer[pos.x + pos.y * GROUND_WIDTH];
}

void BlockEditor::update(const Camera& camera) {
	bool currentBlockChanged = false;
	SDL_Point mousePos;
	const auto mouseState = SDL_GetMouseState(&mousePos.x, &mousePos.y);
	mousePos /= camera.scale;

	const SDL_Rect mouseRect{ mousePos.x, mousePos.y, 1, 1 };	

	const bool lMouse = mouseState & lMouseMask && !mouseDebounce;
	const bool mMouse = mouseState & mMouseMask && !mouseDebounce;
	const bool rMouse = mouseState & rMouseMask && !mouseDebounce;

	const auto& input = globalObjects::input;
	{
	const int incr = input.getKeyPress(']') - input.getKeyPress('[');
	currentBlockIndex = wrap(currentBlockIndex + incr, Ground::data.size());
	}
	if (input.getKeyPress('w')) {
		selectedTile.flags |= SDL_FLIP_VERTICAL;
	}
	if (input.getKeyPress('s')) {
		selectedTile.flags &= ~SDL_FLIP_VERTICAL;
	}
	if (input.getKeyPress('d')) {
		selectedTile.flags |= SDL_FLIP_HORIZONTAL;
	}
	if (input.getKeyPress('a')) {
		selectedTile.flags &= ~SDL_FLIP_HORIZONTAL;
	}

	flagMode ^= input.getKeyPress('f');

	if (input.getKeyPress('n')) {
		addBlock();
	}
	if (input.getKeyPress('r')) {
		const GroundData& lastBlock = currentBlock();
		addBlock();
		currentBlock() = lastBlock;
		currentBlockChanged = true;
	}

	if (input.getKeyPress('m')) {
		Ground::showCollision = !Ground::showCollision;
		groundMap = Sprite{ Ground::showCollision ? Ground::getCollisionDebugMap() : Ground::map };
		std::for_each(Ground::data.begin(), Ground::data.end(), updateBlock);
	}

	currentBlockChanged |= editBlock(mouseRect, mouseState, mouseDebounce);

	selectTile(mouseRect, mouseState, mouseDebounce);

	if (currentBlockChanged) {
		updateBlock(currentBlock());
		currentBlockChanged = false;
	}

	if (input.getKeyPress('j')) {
		save();
	}

	mouseDebounce = mouseState & (lMouseMask | mMouseMask | rMouseMask);
}

void BlockEditor::updateBlock(Ground::GroundData& block) {
	block.updateTileGraphics();
}

void BlockEditor::addBlock() {
	Ground::GroundData temp;
	Ground::Layer emptyLayer;
	emptyLayer.fill(CollisionTile{ 0, 0 });
	temp.graphics.resize(2, emptyLayer);
	temp.collision.resize(2, emptyLayer);
	Ground::addTile(temp);
	currentBlockIndex = Ground::data.size() - 1;
}

bool BlockEditor::editBlock(SDL_Rect mouseRect, Uint32 mouseState, bool mouseDebounce) {
	const SDL_Rect blockArea{ 0, 0, 2 * GROUND_PIXEL_WIDTH, GROUND_PIXEL_WIDTH };
	const auto rClick = mouseState & rMouseMask && !mouseDebounce;
	auto& type = (Ground::showCollision ? currentBlock().collision : currentBlock().graphics);
	if (SDL_HasIntersection(&blockArea, &mouseRect)) {
		const bool rightBlock = GROUND_PIXEL_WIDTH <= mouseRect.x;
		if (type.size() < 2) {
			Ground::Layer temp;
			temp.fill({ 0, 0 });
			type.resize(2, temp);
		}
		auto& layer = type[rightBlock];
		auto& tile = tileAt(layer, SDL_Point{ int(mouseRect.x % GROUND_PIXEL_WIDTH), mouseRect.y } / TILE_WIDTH);
		if (rClick) {
			selectedTile = tile;
		}
		if (flagMode) {
			if (!mouseDebounce && mouseState && lMouseMask) {
				tile.flags ^= static_cast< int >(Ground::Flags::TOP_SOLID);
				return true;
			}
		}
		else if (mouseState & lMouseMask) {
			tile = selectedTile;
			return true;
		}
	}
	return false;
}

void BlockEditor::selectTile(SDL_Rect mouseRect, Uint32 mouseState, bool mouseDebounce) {
	const auto lMouse = mouseState & lMouseMask && !mouseDebounce; 

	const SDL_Point groundMapSize = groundMap.size() / 2;
	const auto groundMapArea = SDL_Rect{ 2 * GROUND_PIXEL_WIDTH, 0, groundMapSize.x, groundMapSize.y };
	if (SDL_HasIntersection(&groundMapArea, &mouseRect) && lMouse) {
		const int tilesPerRow = groundMap.size().x / TILE_WIDTH;
		selectedTile.setIndex(tilesPerRow * (mouseRect.y * 2 / TILE_WIDTH) + (mouseRect.x - groundMapArea.x) * 2 / TILE_WIDTH);
		std::cout << "Selected tile " << selectedTile.getIndex() << "\n";
	}

}

void BlockEditor::renderFlags() const {
	const auto& blockTiles = (Ground::showCollision ? currentBlock().collision: currentBlock().graphics);
	// TODO: Fix
	for (int blockIndex = 0; blockIndex < blockTiles.size(); ++blockIndex) {
		for (int x = 0; x < GROUND_WIDTH; ++x) {
			for (int y = 0; y < GROUND_WIDTH; ++y) {
				const auto& tile = tileAt(blockTiles[blockIndex], { x, y });
				std::stringstream stream;
				stream << std::hex << tile.flags;
				SDL_Point position{
					int(TILE_WIDTH / 2 + blockIndex * GROUND_PIXEL_WIDTH + x * TILE_WIDTH),
					int(TILE_WIDTH / 2 + y * TILE_WIDTH)
				};
				text::renderAbsolute(position * 2, "GUI", stream.str());
			}
		}
	}
}

void BlockEditor::save() {
	std::cout << "Saving Block" << currentBlockIndex + 1 << ".json\n";

	const std::string path = ASSET"EmeraldHillZone/Block" + std::to_string(currentBlockIndex + 1) + ".json";

	if (std::experimental::filesystem::exists(path)) {
		std::experimental::filesystem::rename(path, path + ".bak");
	}

	auto& graphics = currentBlock().graphics;
	if (graphics.size() > 1 && std::none_of(graphics[1].begin(), graphics[1].end(), [](auto a){ return a.getIndex(); })) {
		graphics.resize(1);
	}

	auto& collision = currentBlock().collision;
	if (collision.size() > 1 && std::none_of(collision[1].begin(), collision[1].end(), [](auto a){ return a.getIndex(); })) {
		collision.resize(1);
	}

	nlohmann::json out = Ground::GroundData{ graphics, collision };
	
	std::ofstream file(path);
	file << out.dump(4);

	std::cout << "Saved.\n";
}

Ground::GroundData& BlockEditor::getBlock(std::size_t index) const {
	return Ground::data[index];
}

Ground::GroundData& BlockEditor::currentBlock() const {
	return getBlock(currentBlockIndex);
}
