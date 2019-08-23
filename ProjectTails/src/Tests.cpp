#include "Tests.h"
#include "CollisionTile.h"
#include "Functions.h"

#include <array>
#include <gtest/gtest.h>

bool tests::doTests{};

TEST(SanityCheck, IsReflexive) {
	ASSERT_EQ(1, 1);
}

TEST(CollisionTiles, SurfacePosition) {
	const std::array< int, CollisionTile::heightMapSize > heights{ 0, 0, 0, 4, 4, 6, 14, 14, 14, 10, 10, 10, 10, 0, 0, 0 };
	CollisionTile::dataList.emplace_back(heights, 0.0);
	CollisionTile tile(0, 0.0);

	for (int i = 0; i < CollisionTile::heightMapSize; ++i) {
		EXPECT_EQ(surfacePos(tile, i, 3), (SDL_Point{ i, heights[i] }));
	}

	// TODO: Maybe the others?

	CollisionTile::dataList.clear();
}

TEST(CollisionTiles, GetHeight) {
	const std::array< int, CollisionTile::heightMapSize > heights{ 0, 0, 0, 4, 4, 6, 14, 14, 14, 10, 10, 10, 10, 0, 0, 0 };
	CollisionTile::dataList.emplace_back(heights, 0.0);
	CollisionTile tile(0, 0.0);

	std::array< int, CollisionTile::heightMapSize > temp{};

	// Top side
	for (int i = 0; i < CollisionTile::heightMapSize; ++i) {
		EXPECT_EQ(getHeight(tile, i, 3), heights[i]);
	}

	// Right side
	const std::array< int, CollisionTile::heightMapSize > rightResult{ 0, 0, 9, 9, 9, 9, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13 };
	for (int i = 0; i < CollisionTile::heightMapSize; ++i) {
		EXPECT_EQ(getHeight(tile, i, 0), rightResult[i]);
	}

	// Bottom side
	const std::array< int, CollisionTile::heightMapSize > bottomResult{ 0, 0, 0, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 0, 0, 0 };
	for (int i = 0; i < CollisionTile::heightMapSize; ++i) {
		EXPECT_EQ(getHeight(tile, i, 1), bottomResult[i]);
	}

	// Left side
	const std::array< int, CollisionTile::heightMapSize > leftResult{ 0, 0, 10, 10, 10, 10, 10, 10, 10, 10, 11, 11, 13, 13, 13, 13 };
	for (int i = 0; i < CollisionTile::heightMapSize; ++i) {
		EXPECT_EQ(getHeight(tile, i, 2), leftResult[i]);
	}

	CollisionTile::dataList.clear();
}
