#pragma once
#include <vector>
#include <json.hpp>

class CollisionTile
{
public:
	CollisionTile(int ind, std::vector < int > heights, double ang, bool collide);
	CollisionTile();

	void setHeights(std::vector < int >& heights);
	void setHeight(int height, int index) { heightMap[index] = height; };
	void setAngle(double ang) { angle = ang; };
	void setIndex(int ind) { index = ind; };
	void setCollide(bool collide) { canCollide = collide; };

	int getSideHeight(int ind) { return heightMapSide[ind]; };
	int getHeight(int ind, bool side = false) { return side ? heightMapSide[ind] : heightMap[ind]; };
	int getAngle() { return angle; };
	bool getCollide() { return canCollide; };

	void calculateSideMap();

	int getIndex() { return index; };

	~CollisionTile();
private:
	enum { heightMapSize = 16 };
	int heightMap[heightMapSize];
	int heightMapSide[heightMapSize];
	double angle;
	int index;
	bool canCollide;
};