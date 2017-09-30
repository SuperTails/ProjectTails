#include "stdafx.h"
#include "Ground.h"
#include "Functions.h"
#include "Animation.h"
#include "CollisionTile.h"

Surface Ground::map;
std::vector < CollisionTile > Ground::tileList;
std::string Ground::mPath;
std::vector < Ground::GroundData > Ground::data;

void Ground::setMap(const std::string& mapPath) {
	mPath = mapPath;
	Surface newMap(mapPath);
	if (newMap == nullptr) {
		std::cerr << "Could not set tilemap for Ground! Error: " << SDL_GetError() << "\n";
	}
	map = std::move(newMap);
}

void Ground::setCollisionList(std::vector < CollisionTile > list) {
	tileList = list;
}

Ground::Ground() :
	dataIndex(-1),
	flip(false),
	position{ -1, -1 }
{
}

Ground::Ground(std::size_t index, SDL_Point pos, bool pFlip) :
	flip(pFlip),
	dataIndex(index),
	position(pos)
{
}

void Ground::Render(const SDL_Rect& camPos, const double& ratio, const SDL_Rect* position, int layer, bool doFlip) const {
	SDL_Rect rectPos = SDL_Rect { static_cast<int>(this->position.x * GROUND_PIXEL_WIDTH), static_cast<int>(this->position.y * GROUND_PIXEL_WIDTH), GROUND_PIXEL_WIDTH, GROUND_PIXEL_WIDTH };
	SDL_Point pos = getXY((position != nullptr) ? *position : getRelativePosition(rectPos, camPos));
		
	if (layer == 2) {
		data[dataIndex].layers[1].Render(pos, 0, NULL, 1.0 / ratio, SDL_RendererFlip(flip || doFlip));
		data[dataIndex].layers[0].Render(pos, 0, NULL, 1.0 / ratio, SDL_RendererFlip(flip || doFlip));
	}
	else {
		data[dataIndex].layers[layer].Render(pos, 0, NULL, 1.0 / ratio, SDL_RendererFlip(flip || doFlip));
	}
}

const CollisionTile& Ground::getTile(int tileX, int tileY, bool path) const {
	path &= data[dataIndex].getMultiPath();

	if(flip) {
		return tileList[data[dataIndex].getCollision()[path][tileY * GROUND_WIDTH + (GROUND_WIDTH - 1 - tileX)].index];
	}
	return tileList[data[dataIndex].getCollision()[path][tileX + tileY * GROUND_WIDTH].index];
}

int Ground::getFlag(int tileX, int tileY, bool path) const {
	path &= data[dataIndex].getMultiPath();

	if (flip) {
		return data[dataIndex].getCollision()[path][tileY * GROUND_WIDTH + (GROUND_WIDTH - 1 - tileX)].flags ^ SDL_FLIP_HORIZONTAL;
	}
	return data[dataIndex].getCollision()[path][tileX + tileY * GROUND_WIDTH].flags;
}

Ground& Ground::operator= (Ground arg) {
	using std::swap;

	swap(*this, arg);

	return *this;
}

double Ground::getTileAngle(int tileX, int tileY, bool path) const {
	if (empty()) {
		return 0.0;
	}
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

void swap(Ground& lhs, Ground& rhs) noexcept {
	using std::swap;

	swap(lhs.dataIndex, rhs.dataIndex);
	swap(lhs.position, rhs.position);
	swap(lhs.flip, rhs.flip);
};

Ground::GroundData::GroundData(const Ground::groundArrayData& arrayData) : 
	collision(arrayData.collision),
	layers { Animation(SDL_Point{GROUND_PIXEL_WIDTH, GROUND_PIXEL_WIDTH}), Animation(SDL_Point{GROUND_PIXEL_WIDTH, GROUND_PIXEL_WIDTH}) } {
	
	SDL_Rect current = { 0, 0, TILE_WIDTH, TILE_WIDTH };
	SDL_Rect dest = { 0, 0, TILE_WIDTH, TILE_WIDTH };

	Surface flipHoriz = Animation::FlipSurface(map, SDL_FLIP_HORIZONTAL);
	Surface flipVertical = Animation::FlipSurface(map, SDL_FLIP_VERTICAL);
	Surface flipBoth = Animation::FlipSurface(flipHoriz, SDL_FLIP_VERTICAL);

	const int mapWidth = map.size().x;
	const int mapTileWidth = mapWidth / TILE_WIDTH;
	const int mapHeight = map.size().y;
	
	for (int layer = 0; layer < arrayData.graphics.size(); ++layer) {
		const auto& currentLayer = arrayData.graphics[layer];
		for (int tile = 0; tile < arrayData.graphics[layer].size(); ++tile) {
			const int index = currentLayer[tile].index;
			const int tileFlip = currentLayer[tile].flags & (SDL_FLIP_HORIZONTAL | SDL_FLIP_VERTICAL);
			auto current = SDL_Rect{ index % mapTileWidth, index / mapTileWidth, 1, 1 } * TILE_WIDTH;
			if (tileFlip & SDL_FLIP_HORIZONTAL) {
				current.x = mapWidth - TILE_WIDTH - current.x;
			}
			if (tileFlip & SDL_FLIP_VERTICAL) {
				current.y = mapHeight - TILE_WIDTH - current.y;
			}

			const auto dest = SDL_Rect{ int(tile % GROUND_WIDTH), int(tile / GROUND_WIDTH), 1, 1 } * TILE_WIDTH;

			SDL_Surface* surfaceFlipped = [&]() {
				switch(static_cast<int>(tileFlip)) {
				case SDL_FLIP_NONE:
					return map.get();
				case SDL_FLIP_HORIZONTAL:
					return flipHoriz.get();
				case SDL_FLIP_VERTICAL:
					return flipVertical.get();
				case SDL_FLIP_HORIZONTAL | SDL_FLIP_VERTICAL:
					return flipBoth.get();
				}	
			}();
			//0 is rendered in front of the player
			//1 is rendered behind the player
			SDL_Rect tempDest = dest;
			if (getMultiPath() && arrayData.graphics.size() == 1) {
				const int tilePathFront = arrayData.collision[0][tile].index;
				const int tilePathBack = arrayData.collision[1][tile].index;
				SDL_Rect blank{ 0, 0, TILE_WIDTH, TILE_WIDTH };
				if (tilePathBack && !tilePathFront) {
					SDL_BlitSurface(surfaceFlipped, &current, layers[1].getSpriteSheet(), &tempDest);
					SDL_BlitSurface(map.get(), &blank, layers[0].getSpriteSheet(), &tempDest);
				}
				else {
					SDL_BlitSurface(surfaceFlipped, &current, layers[0].getSpriteSheet(), &tempDest); 
					SDL_BlitSurface(map.get(), &current, layers[1].getSpriteSheet(), &tempDest);
				}
			}
			else if (arrayData.graphics.size() == 2) {
				SDL_BlitSurface(surfaceFlipped, &current, layers[layer].getSpriteSheet(), &tempDest);
			}
			else {
				SDL_BlitSurface(surfaceFlipped, &current, layers[layer].getSpriteSheet(), &tempDest); //src, srcrect, dst, dstrect
			}	
		}
	}

	for (Animation& i : layers) {
		i.updateTexture();
	}
};

std::istream& operator>> (std::istream& stream, Ground& g) {
	SDL_Point pos;
	stream >> pos.x >> pos.y;
	assert(pos.x != -1 && pos.y != -1);
	
	std::size_t index;
	stream >> index;
	assert(index != -1);

	bool flip = false;
	if (stream.get() == ' ') {
		flip = stream.get() - '0';
	}

	g = Ground{ index, pos, flip };
	return stream;
}

std::ostream& operator<< (std::ostream& stream, const Ground& g) {
	assert(g.getPosition().x != -1 && g.getPosition().y != -1);
	assert(g.getIndex() != -1);
	stream << g.getPosition().x << " " << g.getPosition().y
		<< " " << g.getIndex();
	if(g.getFlip()) {
		stream << " 1";
	}

	return stream;
}
