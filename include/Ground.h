#pragma once
#include "SDL2/SDL.h"
#include "Constants.h"
#include "Animation.h"
#include "Typedefs.h"
#include "json.hpp"
#include "DataReader.h"
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
	friend void DataReader::LoadJSONBlock(const std::string& block);

	class GroundData;
public:


	static bool showCollision;

	// SDL_FLIP_HORIZONTAL and SDL_FLIP_VERTICAL are the first two flags
	enum class Flags { TOP_SOLID = (1 << 2) };
	
	friend void to_json(nlohmann::json& j, const CollisionTile& t);
	friend void from_json(const nlohmann::json& j, CollisionTile& t);

	typedef std::array< CollisionTile, GROUND_SIZE > Layer;
	typedef std::vector< Layer > DataType;

	friend void to_json(nlohmann::json& j, const GroundData& arrayData);
	friend void from_json(const nlohmann::json& j, GroundData& arrayData);

	Ground(std::size_t index, SDL_Point pos, bool pFlip = false);
	Ground() = default;
	Ground(const Ground&) = default;
	Ground(Ground&& other) = default;
	~Ground() = default;
	
	void Render(const Camera& camera, const SDL_Point* position = nullptr, int layer = 2, bool doFlip = false) const;
	bool getMulti() const { return data[dataIndex].getMultiPath(); };

	CollisionTile getTile(int tileX, int tileY, bool path) const;
	double getTileAngle(int tileX, int tileY, bool path) const;
	int getFlag(int tileX, int tileY, bool path) const;
	//bool empty() const { return (dataIndex == -1) || data[dataIndex].collision.empty(); };

	void setPosition(SDL_Point pos);
	SDL_Point getPosition() const;

	void setIndex(std::size_t index);
	std::size_t getIndex() const;

	void setFlip(bool newFlip);
	bool getFlip() const;

	bool empty() const;

	static const std::size_t NO_TILE;

	static void setMap(const std::string& mapPath);

	static void clearTiles();
	static void addTile(const GroundData& tileData);
	static std::size_t tileCount() { return data.size(); };

	static std::string getMapPath() { return mPath; };

	Ground& operator= (Ground arg);

	friend void swap(Ground& lhs, Ground& rhs) noexcept;

private:
	std::size_t dataIndex = NO_TILE;
	bool flip = false;

	SDL_Point position = { -1, -1 };

	static std::string mPath;
	static Surface map;

	static Surface& getCollisionDebugMap();

	class GroundData {
		friend class Ground;
		friend class BlockEditor;
	public:
		GroundData() = default;
		GroundData(const GroundData&) = default;
		GroundData(GroundData&&) = default;

		GroundData(const Ground::DataType& graphicsLayers, const Ground::DataType& collisionLayers);

		bool getMultiPath() const;

		static std::array< Animation, 2 > convertTileGraphics(const DataType& graphics, Surface& map = Ground::map);

		static std::pair< Surface&, SDL_Rect > getTileFromMap(CollisionTile tile, Surface& flipNone, Surface& flipH, Surface& flipV, Surface& flipHV);

		void updateTileGraphics();

		GroundData& operator= (const GroundData&) = default;

		DataType graphics;
		DataType collision;

		std::array< Animation, 2 > layers;
	};

	static std::vector < GroundData > data;

};

std::istream& operator>> (std::istream& stream, Ground& g);

std::ostream& operator<< (std::ostream& stream, const Ground& g);
