#include "CollisionTile.h"
#include <SDL.h>

std::vector< CollisionTile::CollisionTileData > CollisionTile::dataList{};

void CollisionTile::setCollisionList(const std::vector< CollisionTileData >& list) {
	dataList = list;
}


void CollisionTile::setHeights(const std::array< int, heightMapSize >& heights) noexcept {
	std::copy(heights.begin(), heights.end(), dataList[dataIndex].heightMap.begin());
}

int getHeight2(const CollisionTile &tile, int idx, int dir) {
	switch (dir) {
	case 0:
		for (int j = CollisionTile::heightMapSize - 1; j >= 0; --j) {
			if (tile.getHeight(j) >= CollisionTile::heightMapSize - idx) {
				return 1 + j;
			}
		}
		return 0;
	case 1:
		return CollisionTile::heightMapSize * (tile.getHeight(idx) != 0);
	case 2:
		for (int j = 0; j < CollisionTile::heightMapSize; ++j) {
			if (tile.getHeight(j) >= CollisionTile::heightMapSize - idx) {
				return CollisionTile::heightMapSize - j;
			}
		}
		return 0;
	case 3:
		return tile.getHeight(idx);
	default:
		throw std::invalid_argument("Invalid direction");
	}
}

int getHeight(const CollisionTile &tile, int idx, int dir) {
	int flags = tile.flags;
	if (flags & SDL_FLIP_HORIZONTAL) {
		switch (dir) {
		case 1:
		case 3:
			idx = CollisionTile::heightMapSize - idx - 1;
			break;
		case 0:
			dir = 2;
			break;
		case 2:
			dir = 0;
			break;
		}
	}
	if (flags & SDL_FLIP_VERTICAL) {
		switch (dir) {
		case 0:
		case 2:
			idx = CollisionTile::heightMapSize - idx - 1;
		case 1:
			dir = 3;
			break;
		case 3:
			dir = 1;
			break;
		}
	}

	return getHeight2(tile, idx, dir);
}

SDL_Point surfacePos(const CollisionTile& tile, int idx, int dir) {
	switch (dir) {
	case 0:
		return { int(CollisionTile::heightMapSize - getHeight(tile, idx, dir) - 1), idx };
	case 1:
		return { idx, int(CollisionTile::heightMapSize - getHeight(tile, idx, dir) - 1) };
	case 2:
		return { getHeight(tile, idx, dir), idx };
	case 3:
		return { idx, getHeight(tile, idx, dir) };
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

void CollisionTile::test() {
	dataList.push_back(CollisionTileData{ { 0, 0, 0, 4, 4, 6, 14, 14, 14, 10, 10, 10, 10, 0, 0, 0 }, 0.0 });
	CollisionTile tile{ int(dataList.size() - 1), 0 };

	for (int i = 0; i <= 3; ++i) {
		std::cout << "\nDir: " << i << "\n";
		for (int j = 0; j < 16; ++j) {
			std::cout << "i: " << j << ", " << ::getHeight(tile, j, i) << "\n";
		}
	}
}
