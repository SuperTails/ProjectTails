#include "stdafx.h"
#include "Ground.h"
#include "Functions.h"
#include "Animation.h"
#include "CollisionTile.h"
#include "Camera.h"
#include "json.hpp"

Surface Ground::map;
std::string Ground::mPath;
std::vector< Ground::GroundData > Ground::data;

bool Ground::showCollision = false;

const std::size_t Ground::NO_TILE = static_cast< std::size_t >(-1);

void Ground::setMap(const std::string& mapPath) {
	mPath = mapPath;
	Surface newMap(ASSET + mapPath);
	if (newMap == nullptr) {
		std::cerr << "Could not set tilemap for Ground! Error: " << SDL_GetError() << "\n";
		throw std::invalid_argument("Could not set tilemap for ground!");
	}
	map = std::move(newMap);
}

Ground::Ground(std::size_t index, SDL_Point pos, bool pFlip) :
	flip(pFlip),
	dataIndex(index),
	position(pos)
{
}

void Ground::Render(const Camera& camera, const SDL_Point* position, int layer, bool doFlip) const {
	const SDL_Point rectPos = (this->position) - SDL_Point{ int(camera.getPosition().x), int(camera.getPosition().y) };
	const SDL_Point pos = (position ? *position : rectPos);

	if (layer == 2) {
		data[dataIndex].layers[1].Render(pos, 0, NULL, camera.scale, SDL_RendererFlip(flip || doFlip));
		data[dataIndex].layers[0].Render(pos, 0, NULL, camera.scale, SDL_RendererFlip(flip || doFlip));
	}
	else {
		data[dataIndex].layers[layer].Render(pos, 0, NULL, camera.scale, SDL_RendererFlip(flip || doFlip));
	}

}

const CollisionTile& Ground::getTile(int tileX, int tileY, bool path) const {
	path &= data[dataIndex].getMultiPath();

	if (flip) {
		return data[dataIndex].collision[path].at(tileY * GROUND_WIDTH + (GROUND_WIDTH - 1 - tileX));
	}
	return data[dataIndex].collision[path].at(tileX + tileY * GROUND_WIDTH);
}

int Ground::getFlag(int tileX, int tileY, bool path) const {
	path &= data[dataIndex].getMultiPath();
	const auto& collisionLayer = data[dataIndex].collision[path];

	if (flip) {
		return collisionLayer[tileY * GROUND_WIDTH + (GROUND_WIDTH - 1 - tileX)].flags ^ SDL_FLIP_HORIZONTAL;
	}
	return collisionLayer[tileX + tileY * GROUND_WIDTH].flags;
}

Ground& Ground::operator= (Ground arg) {
	using std::swap;

	swap(*this, arg);

	return *this;
}

double Ground::getTileAngle(int tileX, int tileY, bool path) const {
	const CollisionTile& tile = getTile(tileX, tileY, path);
	int flag = getFlag(tileX, tileY, path) & (SDL_FLIP_NONE | SDL_FLIP_HORIZONTAL | SDL_FLIP_VERTICAL);
	switch (flag) {
	case SDL_FLIP_NONE:
		return tile.getAngle();
	case SDL_FLIP_HORIZONTAL:
		return 0x100 - tile.getAngle();
	case SDL_FLIP_VERTICAL:
		return (0x180 - tile.getAngle()) % 0x100;
	case (SDL_FLIP_HORIZONTAL | SDL_FLIP_VERTICAL):
		return (0x080 + tile.getAngle()) % 0x100;
	default:
		std::cerr << "Invalid flag: " << flag << "\n";
		throw "Invalid flag";
		return -1;
	}
};

void Ground::setPosition(SDL_Point pos) {
	position = pos;
}

SDL_Point Ground::getPosition() const {
	return position;
}


void Ground::setIndex(std::size_t index) {
	dataIndex = index;
}

std::size_t Ground::getIndex() const {
	return dataIndex;
}


void Ground::setFlip(bool newFlip) {
	flip = newFlip;
}

bool Ground::getFlip() const {
	return flip;
}

Surface& Ground::getCollisionDebugMap() {
	static Surface srf = Surface(ASSET"CollisionTiles.png");
	return srf;
}

void swap(Ground& lhs, Ground& rhs) noexcept {
	using std::swap;

	swap(lhs.dataIndex, rhs.dataIndex);
	swap(lhs.position, rhs.position);
	swap(lhs.flip, rhs.flip);
};

Ground::GroundData::GroundData(const Ground::DataType& graphicsLayers, const Ground::DataType& collisionLayers) : 
	collision(collisionLayers),
	graphics(graphicsLayers) {

	updateTileGraphics();
}	

std::array< Animation, 2 > Ground::GroundData::convertTileGraphics(const DataType& graphics, Surface& map) {
	assert(map != nullptr);

	Surface flipHoriz = Animation::FlipSurface(map, SDL_FLIP_HORIZONTAL);
	Surface flipVertical = Animation::FlipSurface(map, SDL_FLIP_VERTICAL);
	Surface flipBoth = Animation::FlipSurface(map, SDL_RendererFlip(SDL_FLIP_HORIZONTAL | SDL_FLIP_VERTICAL));
	
	std::array< Animation, 2 > layers{ Animation{{ GROUND_PIXEL_WIDTH, GROUND_PIXEL_WIDTH }}, Animation{{ GROUND_PIXEL_WIDTH, GROUND_PIXEL_WIDTH}} };

	for (int layer = 0; layer < graphics.size(); ++layer) {

		const auto& currentLayer = graphics[layer];
		for (int tile = 0; tile < currentLayer.size(); ++tile) {
			// TODO: Figure out what that check was for
			const int index = ((currentLayer[tile].getIndex() == 220) ? 251 : currentLayer[tile].getIndex());
			const int tileFlip = currentLayer[tile].flags & (SDL_FLIP_HORIZONTAL | SDL_FLIP_VERTICAL);
			
			auto [surfaceFlipped, source] = getTileFromMap({ index, tileFlip }, map, flipHoriz, flipVertical, flipBoth); 

			auto dest = SDL_Rect{ int(tile % GROUND_WIDTH), int(tile / GROUND_WIDTH), 1, 1 } * TILE_WIDTH;

			SDL_BlitSurface(surfaceFlipped.get(), &source, layers[layer].getSpriteSheet().get(), &dest);
		}
	}

	for (Animation& i : layers) {
		i.updateTexture();
	}

	return layers;
}

bool Ground::empty() const {
	return dataIndex == NO_TILE || data[dataIndex].collision.size() == 0;
}

std::pair< Surface&, SDL_Rect > Ground::GroundData::getTileFromMap(CollisionTile tile, Surface& flipNone, Surface& flipH, Surface& flipV, Surface& flipHV) {
	const auto [mapWidth, mapHeight] = flipNone.size();
	const int mapTileWidth = mapWidth / TILE_WIDTH;

	tile.flags &= SDL_FLIP_HORIZONTAL | SDL_FLIP_VERTICAL;

	auto source = SDL_Rect{ tile.getIndex() % mapTileWidth, tile.getIndex() / mapTileWidth, 1, 1 } * TILE_WIDTH;
	if (tile.flags & SDL_FLIP_HORIZONTAL) {
		source.x = mapWidth - TILE_WIDTH - source.x;
	}
	if (tile.flags & SDL_FLIP_VERTICAL) {
		source.y = mapHeight - TILE_WIDTH - source.y;
	}

	auto& surfaceFlipped = [&]() -> Surface& {
		switch(static_cast< int >(tile.flags)) {
		case SDL_FLIP_NONE:
			return flipNone;
		case SDL_FLIP_HORIZONTAL:
			return flipH;
		case SDL_FLIP_VERTICAL:
			return flipV;
		case SDL_FLIP_HORIZONTAL | SDL_FLIP_VERTICAL:
			return flipHV;
		default:
			return flipNone;
		}	
	}();

	return { surfaceFlipped, source };
}

void Ground::GroundData::updateTileGraphics() {
	if (showCollision) {
		layers = convertTileGraphics(collision, getCollisionDebugMap());
	}
	else {
		layers = convertTileGraphics(graphics, Ground::map);
	}
}

using json = nlohmann::json;

void to_json(json& j, const CollisionTile& t) {
	const int flip = t.flags & (SDL_FLIP_HORIZONTAL | SDL_FLIP_VERTICAL);
	const bool flipX = (flip == SDL_FLIP_HORIZONTAL) || (flip == SDL_FLIP_VERTICAL);
	const int rot = 2 * bool(flip & SDL_FLIP_VERTICAL);
	
	j = json{ { "rot", rot }, { "flipX", flipX }, { "tile", t.getIndex() } };
}

void from_json(const json& j, CollisionTile& t) {
	t.setIndex(j["tile"].get<int>());
	t.flags = 0;
	if (j["rot"].get<int>() == 2) {
		t.flags |= SDL_FLIP_VERTICAL;
		t.flags |= (!j["flipX"].get<bool>()) * SDL_FLIP_HORIZONTAL;
	}
	else {
		t.flags |= j["flipX"].get<bool>() * SDL_FLIP_HORIZONTAL;
	}
}

void to_json(json& j, const Ground::GroundData& arrayDat) {
	Ground::GroundData arrayData = arrayDat;
	// Output graphics tiles
	for (int i = 0; i < arrayData.graphics.size(); ++i) {
		auto& output = j["layers"][i];
		output["name"] = "Graphics " + std::to_string(i);
		output["tiles"] = arrayData.graphics[i];
	}

	// Transform physics tiles to the correct format (numbered starting at 340)
	for (auto& layer : arrayData.collision) {
		if (std::any_of(layer.begin(), layer.end(), [](auto a) { return a.getIndex() >= 340; })) {
			continue;
		}
		std::transform(layer.begin(), layer.end(), layer.begin(), [](auto a) {
				a.setIndex(a.getIndex() + 340);
			return CollisionTile{ (a.getIndex() == 560 ? 591 : a.getIndex()), a.flags };
		});
	}

	for (int i = 0; i < arrayData.collision.size(); ++i) {
		auto& output = j["layers"][i + arrayData.graphics.size()];
		output["name"] = "Collision " + std::to_string(i);
		output["tiles"] = arrayData.collision[i];
	}

	for (int i = 0; i < arrayData.collision.size(); ++i) {
		if (auto& collision = arrayData.collision[i]; std::none_of(collision.begin(), collision.end(), [](auto a){ return a.flags & int(Ground::Flags::TOP_SOLID); })) {
			continue;
		}
		else {
			auto& output = j["layers"][arrayData.graphics.size() + arrayData.collision.size()];
			output["name"] = "AdditionalFlags";
			for (int tile = 0; tile < collision.size(); ++tile) {
				if (output["tiles"][tile].is_null()) {
					bool topOnly = collision[tile].flags & int(Ground::Flags::TOP_SOLID);
					output["tiles"][tile] = CollisionTile{ topOnly ? 592 : 0, 0 };
				}
			}
		}
	}

	j["layers"][0]["name"] = "Graphics 0";
	j["tilewidth"] = TILE_WIDTH;
	j["tileswide"] = GROUND_SIZE;
	j["tileheight"] = TILE_WIDTH;
	j["tileshigh"] = GROUND_SIZE;
}

void from_json(const json& j, Ground::GroundData& block) {
	const auto& layers = j["layers"];

	const int numLayers = layers.size();
	bool isDoubleLayer = false;

	const int MAX_GRAPHICS_TILE_INDEX = 339;

	auto loadGraphicsLayer = [&](const json& dataLayer) {
		Ground::Layer layer{};
		std::copy(dataLayer.begin(), dataLayer.end(), layer.begin());
		block.graphics.push_back(layer);
	};

	// Load image data
	loadGraphicsLayer(layers[0]["tiles"]);

	if (numLayers >= 2 && layers[1]["tiles"][0]["tile"].get<int>() <= MAX_GRAPHICS_TILE_INDEX && layers[1]["name"].front() != 'C') {
		isDoubleLayer = true;

		// Load image data for layer 2
		loadGraphicsLayer(layers[1]["tiles"]);
	}

	const bool hasAnimatedLayer = (layers[isDoubleLayer]["name"] == "Animation");

	// Handle animation
	if (hasAnimatedLayer) {
		
	}

	const bool hasAdditionalFlags = (layers.back()["name"] == "AdditionalFlags");

	const std::size_t collisionLayerStart = 1 + isDoubleLayer + hasAnimatedLayer;
	const std::size_t collisionLayerCount = numLayers - collisionLayerStart - hasAdditionalFlags;

	Ground::Layer blankLayer;
	std::fill(blankLayer.begin(), blankLayer.end(), CollisionTile{ MAX_GRAPHICS_TILE_INDEX + 1, 0 });

	block.collision.resize(numLayers - collisionLayerStart - hasAdditionalFlags, blankLayer);

	for (int l = 0; l < numLayers - collisionLayerStart - hasAdditionalFlags; ++l) {
		// Load collision data
		const auto& layerTiles = layers[l + collisionLayerStart]["tiles"];
		const bool doOffset = layerTiles[0]["tile"].get<int>() >= 340;
		std::transform(layerTiles.begin(), layerTiles.end(), block.collision[l].begin(),
			[doOffset](const json& tile) {
				int current = tile["tile"].get<int>() - 340 * doOffset;
				CollisionTile result{ current, 0 };
				if (tile["rot"].get<int>() == 2) {
					result.flags |= SDL_FLIP_VERTICAL;
					result.flags |= (!tile["flipX"].get<bool>()) * SDL_FLIP_HORIZONTAL;
				}
				else {
					result.flags |= (tile["flipX"].get<bool>()) * SDL_FLIP_HORIZONTAL;
				}
				return result;
			});
	}

	if (hasAdditionalFlags) {
		for (int i = 0; i < GROUND_SIZE; ++i) {
			if (layers.back()["tiles"][i]["tile"].get<int>() == 592) {
				block.collision[0][i].flags |= static_cast < int >(Ground::Flags::TOP_SOLID);
			}
		}
	}

	block.updateTileGraphics();
}

void Ground::clearTiles() {
	data.clear();
}

void Ground::addTile(const GroundData& newTile) {
	data.emplace_back(newTile);
}

bool Ground::GroundData::getMultiPath() const {
	return collision.size() == 2;
}

std::istream& operator>> (std::istream& stream, Ground& g) {
	SDL_Point pos;
	stream >> pos.x >> pos.y;
	assert(pos.x != -1 && pos.y != -1);
	
	std::size_t index;
	stream >> index;
	assert(index != -1);

	bool flip = false;
	if (stream.peek() == ' ') {
		stream.get();
		flip = stream.get() - '0';
	}

	g = Ground{ index, pos, flip };
	return stream;
}

std::ostream& operator<< (std::ostream& stream, const Ground& g) {
	assert(g.getPosition().x != -1 && g.getPosition().y != -1);
	assert(g.getIndex() != Ground::NO_TILE);
	stream << g.getPosition().x << " " << g.getPosition().y << " " << g.getIndex();
	if(g.getFlip()) {
		stream << " 1";
	}

	return stream;
}
