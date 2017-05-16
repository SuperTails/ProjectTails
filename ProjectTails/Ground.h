#pragma once
#include "PhysicsEntity.h"
#include "CollisionTile.h"
#include "Typedefs.h"
#include <vector>
#include <json.hpp>

using json = nlohmann::json;

class Ground : public PhysicsEntity
{
public:
	struct groundArrayData {
		std::vector < int > graphicsIndices;
		std::vector < int > graphicsFlags;
		std::vector < int > collideIndices;
		std::vector < int > collideFlags;
		groundArrayData() :
			graphicsIndices(),
			graphicsFlags(),
			collideIndices(),
			collideFlags()
		{};
		groundArrayData(const std::vector<int>& indices, const std::vector<int>& flags, const std::vector<int>& collides, const std::vector<int>& collideFlags) :
			graphicsIndices(indices),
			graphicsFlags(flags),
			collideIndices(collides),
			collideFlags(collideFlags)
		{};
	};

	Ground();
	/*
	p - location
	indices - graphics tile indices
	flags - graphics tile flags
	collideIndices - collision tile indices
	collideFlags - collision tile flags
	*/
	Ground(SDL_Point p, const groundArrayData& arrayData, bool pFlip = false);
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
	int* tileIndices;
	int* tileFlags;
	bool multiPath;
	bool flip;
};