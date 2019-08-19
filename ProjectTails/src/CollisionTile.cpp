#include "CollisionTile.h"

CollisionTile::CollisionTile()
{
}


CollisionTile::CollisionTile(int ind, std::vector < int > heights, double ang, bool collide) :
	index(ind),
	angle(ang),
	canCollide(collide)
{
	//Note that index 0 on the sideways map is at the top
	std::memcpy(heightMap, &(heights[0]), 16 * sizeof(int));
	std::memset(heightMapSide, 0, 16 * sizeof(int));
	for (int i = 0; i < 16; i++) {
		for (int j = 0; j < 16; j++) {
			if (heightMap[j] >= i) {
				++heightMapSide[i];
			}
		}
	}
}


CollisionTile::~CollisionTile()
{
}

void CollisionTile::setHeights(const std::vector < int >& heights) {
	std::memcpy(heightMap, &(heights[0]), 16 * sizeof(int));
}

void CollisionTile::calculateSideMap() {
	for (int i = 0; i < 16; i++) {
		heightMapSide[i] = 0;
		for (int j = 0; j < 16; j++) {
			if (heightMap[j] >= 16 - i) {
				++heightMapSide[i];
			}
		}
	}
}
