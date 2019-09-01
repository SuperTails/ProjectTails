#pragma once
#include "DataReader.h"
#include "Player.h"
#include <array>
#include <json.hpp>

class CollisionTile {
public:
	static const std::size_t heightMapSize = 16;

	CollisionTile() = default;
	CollisionTile(int idx, int fl) : dataIndex(idx), flags(fl) {};

	void setHeights(const std::array< int, heightMapSize >& heights) noexcept;
	void setHeight(int index, int height) { dataList[dataIndex].heightMap[index] = height; };
	void setAngle(double ang) { dataList[dataIndex].angle = ang; };

	int getHeight(int ind) const { return dataList[dataIndex].heightMap[ind]; };
	int getAngle() const { return dataList[dataIndex].angle; };

	int getAngle(Direction dir) const;

	void setIndex(int idx) { dataIndex = idx; };

	int getIndex() const { return dataIndex; };

	int flags = 0;

	static void loadFromImage(const std::string& image);

private:
	int getRawAngle() const;

	int dataIndex = 0;

	struct CollisionTileData {
		CollisionTileData() = default;
		CollisionTileData(const CollisionTileData&) = default;
		CollisionTileData(CollisionTileData&&) = default;

		CollisionTileData(std::array< int, heightMapSize > hMap, double ang);

		constexpr CollisionTileData& operator=(CollisionTileData&) = default;
		constexpr CollisionTileData& operator=(const CollisionTileData&) = default;

		std::array < int, heightMapSize > heightMap{};
		double angle{};
	};

	static CollisionTileData loadCollisionTile(const Surface& surface, SDL_Point topLeft);

	static void setCollisionList(const std::vector< CollisionTileData >& list);

public:
	static std::vector< CollisionTileData > dataList;

	static void test();
};

// dir:
// 0 = from the right
// 1 = from the bottom
// 2 = from the left
// 3 = from the top
int getHeight(const CollisionTile &tile, int idx, Direction dir);

std::optional< SDL_Point > surfacePos(const CollisionTile &tile, int idx, Direction dir);
