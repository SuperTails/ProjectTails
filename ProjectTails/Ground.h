#pragma once
#include "Constants.h"
#include "Typedefs.h"
#include <array>
#include <vector>

using constants::GROUND_SIZE;
using constants::GROUND_WIDTH;
using constants::TILE_WIDTH;
using constants::GROUND_PIXEL_WIDTH;

class Animation;

class CollisionTile;

class Ground {
public:
	enum class Flags { TOP_SOLID = 4 };

	struct Tile {
		int index;
		int flags;
	};

	typedef std::array < Tile, GROUND_SIZE > Layer;
	typedef std::vector < Layer > DataType;

	struct groundArrayData {
		DataType graphics;
		DataType collision;
	};

	Ground();
	Ground(std::size_t index, SDL_Point pos, bool pFlip = false);
	Ground(const Ground&) = default;
	Ground(Ground&& other) = default;
	~Ground() = default;
	
	void Render(const SDL_Rect& camPos, const double& ratio, const SDL_Rect* position = nullptr, int layer = 2, bool doFlip = false) const;
	void setPoint(SDL_Point p) { position = { 256 * p.x, 256 * p.y }; };
	bool getMulti() const { return data[dataIndex].getMultiPath(); };

	const CollisionTile& getTile(int tileX, int tileY, bool path) const;
	double getTileAngle(int tileX, int tileY, bool path) const;
	int getFlag(int tileX, int tileY, bool path) const;
	bool empty() const { return (dataIndex == -1) || data[dataIndex].getCollision().empty(); };

	void setPosition(SDL_Point pos);
	SDL_Point getPosition() const;

	void setIndex(std::size_t index);
	std::size_t getIndex() const;

	void setFlip(bool newFlip);
	bool getFlip() const;

	static void setMap(const std::string& mapPath);
	static void setCollisionList(std::vector < CollisionTile > list);

	static void clearTiles() { data.clear(); };
	static void addTile(const groundArrayData& tileData) { data.emplace_back(tileData); };
	static std::size_t tileCount() { return data.size(); };

	static std::string getMapPath() { return mPath; };

	Ground& operator= (Ground arg);

	friend void swap(Ground& lhs, Ground& rhs) noexcept;

private:
	std::size_t dataIndex;
	bool flip;

	SDL_Point position;

	static std::string mPath;
	static std::vector < CollisionTile > tileList;
	static Surface map;

	class GroundData;

	static std::vector < GroundData > data;

	class GroundData {
		friend class Ground;
	public:
		GroundData() = default;
		GroundData(const GroundData&) = default;
		GroundData(GroundData&&) = default;

		GroundData(const Ground::groundArrayData& arrayData);

		const DataType& getCollision() { return collision; }; 

		bool getMultiPath() { return collision.size() == 2; };

	private:
		DataType graphics;
		DataType collision;

		std::array < Animation, 2 > layers;
	};
};

std::istream& operator>> (std::istream& stream, Ground& g);

std::ostream& operator<< (std::ostream& stream, const Ground& g);
