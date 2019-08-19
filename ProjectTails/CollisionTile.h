#pragma once
#include <array>
#include <json.hpp>

class CollisionTile
{
public:
	static const std::size_t heightMapSize = 16;

	CollisionTile(const std::array< int, heightMapSize >& heights, double ang) noexcept;
	CollisionTile() noexcept = default;

	void setHeights(const std::array< int, heightMapSize >& heights) noexcept;
	void setHeight(int index, int height) { heightMap[index] = height; };
	void setAngle(double ang) { angle = ang; };
	void setCollide(bool collide) { canCollide = collide; };

	int getSideHeight(int ind) const { return heightMapSide[ind]; };
	int getHeight(int ind, bool side = false) const { return side ? heightMapSide[ind] : heightMap[ind]; };
	int getAngle() const { return angle; };
	bool getCollide() const { return canCollide; };

	void calculateSideMap();

private:
	std::array < int, heightMapSize > heightMap{};
	std::array < int, heightMapSize > heightMapSide{};
	double angle;
	bool canCollide;
};
