#pragma once
#include "PhysicsEntity.h"
#include "CollisionTile.h"
#include "Typedefs.h"
#include "CollisionTile.h"
#include <vector>
#include <json.hpp>

using json = nlohmann::json;

using constants::GROUND_SIZE;
using constants::GROUND_WIDTH;
using constants::TILE_WIDTH;
using constants::GROUND_PIXEL_WIDTH;

class Ground : public PhysicsEntity
{
public:
	enum class Flags { TOP_SOLID = 4 };

	struct groundArrayData {
		std::vector < int > graphicsIndices;
		std::vector < int > graphicsFlags;
		std::vector < int > collideIndices;
		std::vector < int > collideFlags;
		groundArrayData() = default;
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
	Ground(const Ground&);
	Ground(Ground&& other) noexcept;
	
	void Render(const SDL_Rect& camPos, const double& ratio, const SDL_Rect* position = nullptr, int layer = 0, bool flip = false) const;
	int* getIndices() { return tileIndices; };
	void setPoint(SDL_Point p) { position = { 256 * p.x, 256 * p.y, 256, 256 }; };
	bool getMulti() const { return multiPath; };

	const CollisionTile& getTile(int tileX, int tileY, bool path) const;
	double getTileAngle(int tileX, int tileY, bool path) const;
	int getFlag(int tileX, int tileY, bool path) const;
	bool empty() const { return (tileIndices == nullptr); };

	void setIndice(int ind, int value) { tileIndices[ind] = value; };
	void setFlag(int ind, int value) { tileFlags[ind] |= value; };
	void unsetFlag(int ind, int value) { tileFlags[ind] &= ~value; };

	static void setMap(std::string mapPath);
	static void setList(std::vector < CollisionTile > list);

	static std::string getMapPath() { return mPath; };

	Ground& operator= (Ground arg);

	friend void swap(Ground& lhs, Ground& rhs) noexcept;

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