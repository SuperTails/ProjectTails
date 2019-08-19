#include "stdafx.h"
#include "CollisionTile.h"

CollisionTile::CollisionTile(const std::array< int, heightMapSize >& heights, double ang) noexcept :
	angle(ang),
	heightMap(heights),
	canCollide(std::any_of(heights.begin(), heights.end(), [](int a) { return a; }))
{
	calculateSideMap();
}


void CollisionTile::setHeights(const std::array< int, heightMapSize >& heights) noexcept {
	std::copy(heights.begin(), heights.end(), heightMap.begin());
	calculateSideMap();
}

void CollisionTile::calculateSideMap() {
	for (int i = 0; i < heightMapSize; ++i) {
		heightMapSide[i] = std::count_if(heightMap.begin(), heightMap.end(), [&](auto height) { return height >= heightMapSize - i; });
	}
}
