#include "Tests.h"
#include "CollisionTile.h"
#include "Functions.h"
#include "Player.h"

#include <array>
#include <gtest/gtest.h>

bool tests::doTests{};

TEST(SanityCheck, IsReflexive) {
	ASSERT_EQ(1, 1);
}

TEST(Conversions, HexRadInverse) {
	ASSERT_NEAR(hexToRad(radToHex(0.1)), 0.1, 1e-7);
	ASSERT_DOUBLE_EQ(hexToRad(32.0), M_PI / 4.0);
}

/*TEST(Directions, DirectionCompare) {
	ASSERT_GT(directionCompare(SDL_Point{ 10, 3 }, SDL_Point{ 3, 4 }, Direction::UP), 0);
	ASSERT_GT(directionCompare(SDL_Point{ 10, 5 }, SDL_Point{ 3, 4 }, Direction::RIGHT), 0);
	ASSERT_GT(directionCompare(SDL_Point{ 10, 5 }, SDL_Point{ 3, 4 }, Direction::DOWN), 0);
	ASSERT_GT(directionCompare(SDL_Point{ 10, 5 }, SDL_Point{ 3, 4 }, Direction::LEFT), 0);
}*/

TEST(Directions, Rotations) {
	for (int i = 0; i < 4; ++i) {
		SDL_Point lhs = rotate90(i, SDL_Point{ 0,  2 });
		SDL_Point rhs = rotate90(i, SDL_Point{ 0,  1 });
		Direction dir = static_cast< Direction >(i);
		int result = directionCompare(lhs, rhs, dir);
		EXPECT_EQ(result, -1) << "Expected " << result << " to be less than 0 for direction " << dir;
	}
}

TEST(Directions, Relative) {
	std::array< Direction, 16 > results = {
		Direction::UP, Direction::RIGHT, Direction::DOWN, Direction::LEFT, // GROUND
		Direction::RIGHT, Direction::DOWN, Direction::LEFT, Direction::UP, // LEFT_WALL
		Direction::DOWN, Direction::LEFT, Direction::UP, Direction::RIGHT, // CEILING
		Direction::LEFT, Direction::UP, Direction::RIGHT, Direction::DOWN  // RIGHT_WALL
	};
	for (int i = 0; i < 16; ++i) {
		EXPECT_EQ(results[i], directionFromRelative(static_cast< Direction >(i % 4), static_cast< Mode >(i / 4)));
	}
}

/*TEST(CollisionTiles, SurfacePosition) {
	const std::array< int, CollisionTile::heightMapSize > heights{ 0, 0, 0, 4, 4, 6, 14, 14, 14, 10, 10, 10, 10, 0, 0, 0 };
	CollisionTile::dataList.emplace_back(heights, 0.0);
	CollisionTile tile(0, 0.0);


	const std::array< int, CollisionTile::heightMapSize > resultY{ 0, 0, 0, 12, 12, 10, 2, 2, 2, 6, 6, 6, 6, 0, 0, 0 };
	for (int i = 0; i < CollisionTile::heightMapSize; ++i) {
		SDL_Point actual = surfacePos(tile, i, Direction::DOWN);
		SDL_Point expected = SDL_Point{ i, heights[i] };
		EXPECT_EQ(actual, expected) << "Actual value: { " << actual.x << ", " << actual.y << " } did not match expected { " << expected.x << ", " << expected.y << " }";
	}

	// TODO: Maybe the others?

	CollisionTile::dataList.clear();
}*/

TEST(CollisionTiles, GetHeight) {
	const std::array< int, CollisionTile::heightMapSize > heights{ 0, 0, 0, 4, 4, 6, 14, 14, 14, 10, 10, 10, 10, 0, 0, 0 };
	CollisionTile::dataList.emplace_back(heights, 0.0);
	CollisionTile tile(0, 0.0);

	std::array< int, CollisionTile::heightMapSize > temp{};

	// Top side
	for (int i = 0; i < CollisionTile::heightMapSize; ++i) {
		EXPECT_EQ(getHeight(tile, i, Direction::DOWN), heights[i]);
	}

	// Right side
	const std::array< int, CollisionTile::heightMapSize > rightResult{ 0, 0, 9, 9, 9, 9, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13 };
	for (int i = 0; i < CollisionTile::heightMapSize; ++i) {
		EXPECT_EQ(getHeight(tile, i, Direction::LEFT), rightResult[i]);
	}

	// Bottom side
	const std::array< int, CollisionTile::heightMapSize > bottomResult{ 0, 0, 0, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 0, 0, 0 };
	for (int i = 0; i < CollisionTile::heightMapSize; ++i) {
		EXPECT_EQ(getHeight(tile, i, Direction::UP), bottomResult[i]);
	}

	// Left side
	const std::array< int, CollisionTile::heightMapSize > leftResult{ 0, 0, 10, 10, 10, 10, 10, 10, 10, 10, 11, 11, 13, 13, 13, 13 };
	for (int i = 0; i < CollisionTile::heightMapSize; ++i) {
		EXPECT_EQ(getHeight(tile, i, Direction::RIGHT), leftResult[i]);
	}

	CollisionTile::dataList.clear();
}
