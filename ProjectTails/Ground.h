#pragma once
#include "SDL.h"
#include "Constants.h"
#include "Animation.h"
#include "Typedefs.h"
#include "json.hpp"
#include <array>
#include <vector>
#include <bitset>

using constants::GROUND_SIZE;
using constants::GROUND_WIDTH;
using constants::TILE_WIDTH;
using constants::GROUND_PIXEL_WIDTH;

class Animation;
class CollisionTile;
class BlockEditor;
class Camera;
class Surface;
struct SDL_Point;

class Ground {
	friend class BlockEditor;
public:

	static bool showCollision;

	// SDL_FLIP_HORIZONTAL and SDL_FLIP_VERTICAL are the first two flags
	enum class Flags { TOP_SOLID = 0b0100, };
	
	struct Tile {
		int index;
		int flags;
	};

	friend void to_json(nlohmann::json& j, const Tile& t);
	friend void from_json(const nlohmann::json& j, Tile& t);

	typedef std::array < Tile, GROUND_SIZE > Layer;
	typedef std::vector < Layer > DataType;

	struct groundArrayData {
		DataType graphics;
		DataType collision;
	};

	friend void to_json(nlohmann::json& j, groundArrayData arrayData);
	friend void from_json(const nlohmann::json& j, groundArrayData& arrayData);

	Ground(std::size_t index, SDL_Point pos, bool pFlip = false);
	Ground() = default;
	Ground(const Ground&) = default;
	Ground(Ground&& other) = default;
	~Ground() = default;
	
	void Render(const Camera& camera, const SDL_Point* position = nullptr, int layer = 2, bool doFlip = false) const;
	bool getMulti() const { return data[dataIndex].getMultiPath(); };

	const CollisionTile& getTile(int tileX, int tileY, bool path) const;
	double getTileAngle(int tileX, int tileY, bool path) const;
	int getFlag(int tileX, int tileY, bool path) const;
	bool empty() const { return (dataIndex == -1) || data[dataIndex].collision.empty(); };

	void setPosition(SDL_Point pos);
	SDL_Point getPosition() const;

	void setIndex(std::size_t index);
	std::size_t getIndex() const;

	void setFlip(bool newFlip);
	bool getFlip() const;

	static const std::size_t NO_TILE;

	static void setMap(const std::string& mapPath);
	static void setCollisionList(const std::vector < CollisionTile >& list);

	static void clearTiles() { data.clear(); };
	static void addTile(const groundArrayData& tileData) { data.emplace_back(tileData); };
	static std::size_t tileCount() { return data.size(); };

	static std::string getMapPath() { return mPath; };

	Ground& operator= (Ground arg);

	friend void swap(Ground& lhs, Ground& rhs) noexcept;

private:
	std::size_t dataIndex = NO_TILE;
	bool flip = false;

	SDL_Point position = { -1, -1 };

	static std::string mPath;
	static std::vector < CollisionTile > tileList;
	static Surface map;

	static Surface& getCollisionDebugMap();

	class GroundData;

	static std::vector < GroundData > data;

	class GroundData {
		friend class Ground;
		friend class BlockEditor;
	public:
		GroundData() = default;
		GroundData(const GroundData&) = default;
		GroundData(GroundData&&) = default;

		GroundData(const Ground::groundArrayData& arrayData);

		bool getMultiPath() const { return collision.size() == 2; };

		static std::array< Animation, 2 > convertTileGraphics(const DataType& graphics, Surface& map = Ground::map);

		static std::pair< Surface&, SDL_Rect > getTileFromMap(Tile tile, Surface& flipNone, Surface& flipH, Surface& flipV, Surface& flipHV);

		void updateTileGraphics();

		GroundData& operator= (const GroundData&) = default;

		DataType graphics;
		DataType collision;

		std::array< Animation, 2 > layers;
	};
};

std::istream& operator>> (std::istream& stream, Ground& g);

std::ostream& operator<< (std::ostream& stream, const Ground& g);
