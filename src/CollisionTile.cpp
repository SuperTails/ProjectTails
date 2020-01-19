#include "CollisionTile.h"
#include <SDL.h>

std::vector< CollisionTile::CollisionTileData > CollisionTile::dataList{};

void CollisionTile::setCollisionList(const std::vector< CollisionTileData >& list) {
	dataList = list;
}


void CollisionTile::setHeights(const std::array< int, heightMapSize >& heights) noexcept {
	std::copy(heights.begin(), heights.end(), dataList[dataIndex].heightMap.begin());
}

int CollisionTile::getAngle() const {
	switch (flags & (SDL_FLIP_HORIZONTAL | SDL_FLIP_VERTICAL)) {
	case SDL_FLIP_NONE:
		return dataList[dataIndex].angle;
	case SDL_FLIP_HORIZONTAL:
		return 0x100 - dataList[dataIndex].angle;
	case SDL_FLIP_VERTICAL:
		return (0x180 - int(dataList[dataIndex].angle)) % 0x100;
	case (SDL_FLIP_HORIZONTAL | SDL_FLIP_VERTICAL):
		return (0x080 + int(dataList[dataIndex].angle)) % 0x100;
	default:
		std::cerr << "Invalid flag: " << flags << "\n";
		throw "Invalid flag";
		return -1;
	}
}

// TODO: Confirm angles are correct
int CollisionTile::getAngle(Direction dir) const {
	if (!(flags & SDL_FLIP_HORIZONTAL) && dir == Direction::LEFT) {
		return 64;
	}
	else if ((flags & SDL_FLIP_HORIZONTAL) && dir == Direction::RIGHT) {
		return 192;
	}
	else if ((flags & SDL_FLIP_VERTICAL) && dir == Direction::DOWN) {
		return 128;
	}
	else if (!(flags & SDL_FLIP_VERTICAL) && dir == Direction::UP) {
		return 0;
	}
	else {
		return getAngle();
	}
}

int getHeight2(const CollisionTile &tile, int idx, Direction dir) {
	switch (dir) {
	case Direction::LEFT:
		for (int j = CollisionTile::heightMapSize - 1; j >= 0; --j) {
			if (tile.getHeight(j) >= CollisionTile::heightMapSize - idx) {
				return 1 + j;
			}
		}
		return 0;
	case Direction::UP:
		return CollisionTile::heightMapSize * (tile.getHeight(idx) != 0);
	case Direction::RIGHT:
		for (int j = 0; j < CollisionTile::heightMapSize; ++j) {
			if (tile.getHeight(j) >= CollisionTile::heightMapSize - idx) {
				return CollisionTile::heightMapSize - j;
			}
		}
		return 0;
	case Direction::DOWN:
		return tile.getHeight(idx);
	default:
		throw std::invalid_argument("Invalid direction");
	}
}

int getHeight(const CollisionTile &tile, int idx, Direction dir) {
	int flags = tile.flags;
	if (flags & SDL_FLIP_HORIZONTAL) {
		switch (dir) {
		case Direction::UP:
		case Direction::DOWN:
			idx = CollisionTile::heightMapSize - idx - 1;
			break;
		case Direction::LEFT:
			dir = Direction::RIGHT;
			break;
		case Direction::RIGHT:
			dir = Direction::LEFT;
			break;
		}
	}
	if (flags & SDL_FLIP_VERTICAL) {
		switch (dir) {
		case Direction::LEFT:
		case Direction::RIGHT:
			idx = CollisionTile::heightMapSize - idx - 1;
		case Direction::UP:
			dir = Direction::DOWN;
			break;
		case Direction::DOWN:
			dir = Direction::UP;
			break;
		}
	}

	return getHeight2(tile, idx, dir);
}

std::optional< SDL_Point > surfacePos(const CollisionTile& tile, int idx, Direction dir) {
	if (getHeight(tile, idx, dir) == 0) {
		return {};
	}
	switch (dir) {
	case Direction::LEFT:
		return { { getHeight(tile, idx, dir) - 1, idx } };
	case Direction::UP:
		return { { idx, getHeight(tile, idx, dir) - 1 } };
	case Direction::RIGHT:
		return { { int(CollisionTile::heightMapSize - getHeight(tile, idx, dir)), idx } };
	case Direction::DOWN:
		return { { idx, int(CollisionTile::heightMapSize - getHeight(tile, idx, dir)) } };
	default:
		throw "Invalid direction\n";
	}
}

void CollisionTile::loadFromImage(const std::string& path) {
	Surface DataFile(path);
	if (DataFile == nullptr) {
		throw "Could not open image file!\n";
	}

	Surface::PixelLock pixelLock(DataFile);

	std::vector< CollisionTileData > result;
	result.reserve(DataFile.size().x / constants::TILE_WIDTH * 8);

	for (int tileY = 0; tileY < 8; ++tileY) {
		std::cout << "Reading collision data on row " << tileY << "\n";
		for (int tileX = 0; tileX < DataFile.size().x / constants::TILE_WIDTH; ++tileX) {
			result.push_back(loadCollisionTile(DataFile, { int(tileX * constants::TILE_WIDTH), int(tileY * constants::TILE_WIDTH) }));
		}
	}

	CollisionTile::setCollisionList(result);
};

CollisionTile::CollisionTileData CollisionTile::loadCollisionTile(const Surface& surface, SDL_Point topLeft) {
	const Uint32 colorMask  = imageFormat.Rmask | imageFormat.Gmask | imageFormat.Bmask;
	const Uint32 angleMask  = imageFormat.Gmask;
	const Uint32 angleShift = imageFormat.Gshift;

	CollisionTile::CollisionTileData result;
	for (int x = topLeft.x; x < topLeft.x + CollisionTile::heightMapSize; ++x) {
		for (int y = topLeft.y; y < topLeft.y + CollisionTile::heightMapSize; ++y) {
			const Uint32 color = getPixel(surface.get(), x, y);
			if (color & colorMask) {
				// Move to the next column
				const double tempAngle = (color & angleMask) >> angleShift;
				result.angle = ((tempAngle == 255.0) ? 0.0 : tempAngle);
				result.heightMap[x - topLeft.x] = 16 - (y - topLeft.y);
				break;
			}
		}
	}

	return result;
}

CollisionTile::CollisionTileData::CollisionTileData(std::array< int, CollisionTile::heightMapSize > hMap, double ang) :
	heightMap(hMap),
	angle(ang)
{

}

void CollisionTile::test() {
	dataList.push_back(CollisionTileData{ { 0, 0, 0, 4, 4, 6, 14, 14, 14, 10, 10, 10, 10, 0, 0, 0 }, 0.0 });
	CollisionTile tile{ int(dataList.size() - 1), 0 };

	for (int i = 0; i <= 3; ++i) {
		std::cout << "\nDir: " << i << "\n";
		for (int j = 0; j < 16; ++j) {
			std::cout << "i: " << j << ", " << ::getHeight(tile, j, static_cast< Direction >(i)) << "\n";
		}
	}
}
