#pragma once
#include "DataReader.h"
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

	void setIndex(int idx) { dataIndex = idx; };

	int getIndex() const { return dataIndex; };

	int flags = 0;

	static void loadFromImage(const std::string& image);

private:
	int dataIndex = 0;

	struct CollisionTileData {
		std::array < int, heightMapSize > heightMap{};
		double angle;
	};

	static CollisionTileData loadCollisionTile(const Surface& surface, SDL_Point topLeft);

	static void setCollisionList(const std::vector< CollisionTileData >& list);

	static std::vector< CollisionTileData > dataList;
public:
	static void test();
};

// dir:
// 0 = from the right
// 1 = from the bottom
// 2 = from the left
// 3 = from the top
int getHeight(const CollisionTile &tile, int idx, int dir);
