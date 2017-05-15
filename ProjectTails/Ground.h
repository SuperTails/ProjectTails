#pragma once
#include "PhysicsEntity.h"
#include "CollisionTile.h"
#include <vector>
#include <json.hpp>

using json = nlohmann::json;

class Ground : public PhysicsEntity
{
public:
	Ground();
	/*
	p - location
	indices - graphics tile indices
	flags - graphics tile flags
	collideIndices - collision tile indices
	collideFlags - collision tile flags
	*/
	Ground(SDL_Point p, const std::vector < int >& indices, const std::vector < int >& flags, const std::vector < int >& collideIndices, const std::vector < int >& collideFlags, bool pFlip = false);
	Ground(Ground&& other);
	Ground(const Ground &);
	
	void Render(const SDL_Rect& camPos, const double& ratio, const SDL_Rect* position = nullptr, const int layer = 0, const bool flip = false) const;
	int* getIndices() { return tileIndices; };
	void setIndices(std::vector < int > ind);
	void setPoint(SDL_Point p) { position = { 256 * p.x, 256 * p.y, 256, 256 }; };
	bool getHeight(int x, int yMax, int yMin, int& height);
	bool getMulti() { return multiPath; };

	CollisionTile& getTile(int ind) { return tileList[tileIndices[ind]]; };
	CollisionTile& getTile(int tileX, int tileY);
	double getTileAngle(int tileX, int tileY);
	int getFlag(int ind);
	bool isEmpty() { return (!tileIndices); };

	void setIndice(int ind, int value) { tileIndices[ind] = value; };
	void setFlag(int ind, int value) { tileFlags[ind] |= value; };
	void unsetFlag(int ind, int value) { tileFlags[ind] &= ~value; };

	static void setWindow(SDL_Window* win) { window = win; };
	static void setMap(std::string mapPath);
	static void setList(std::vector < CollisionTile > list);

	static std::string getMapPath() { return mPath; };
	
	static std::vector < CollisionTile >& getList();

	Ground& operator = (const Ground& arg);

	Ground& operator = (Ground&& arg);

	

	~Ground();
private:
	static std::string mPath;
	static std::vector < CollisionTile > tileList;
	static SDL_Surface* map;
	static SDL_Window* window;
	int* tileIndices;
	int* tileFlags;
	bool multiPath;
	bool flip;
};