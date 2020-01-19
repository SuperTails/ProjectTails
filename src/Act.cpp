#include "Player.h"
#include "DataReader.h"
#include "Act.h"
#include "Text.h"
#include "Version.h"
#include "CollisionTile.h"
#include "Camera.h"
#include "Constants.h"
#include "Ground.h"
#include "Hitbox.h"
#include "Drawing.h"
#include <fstream>

enum class Act::ActType : unsigned char { TITLE, TORNADO, NORMAL };

bool operator== (const SDL_Rect& a, const SDL_Rect& b) {
	return (a.x == b.x) && (a.y == b.y) && (a.w == b.w) && (a.h == b.h);
}

bool operator== (const SDL_Point& a, const SDL_Point& b) {
	return (a.x == b.x) && (a.y == b.y);
}

Act::Act(const std::string& path) {
	std::ifstream DataFile;
	DataFile.open(path);

	if (DataFile.fail()) {
		std::cerr << "Data file does not exist!";
		throw std::invalid_argument("Data file does not exist!");
	}

	Version version;
	DataFile >> version;
	const bool hasVersion = DataFile.good();
	DataFile.clear();
	DataFile.seekg(0);
	loadVersion(DataFile, (hasVersion ? version : Version{ 0, 0, 0 }));
}

Act::Act(const int& num, const std::string& name1, const std::vector< PhysicsEntity >& ent, const SDL_Rect& w, const ActType& a, double screenRatio, const SDL_Point& levelSize, const std::vector< std::vector< Animation > >& backgnd, std::vector< Ground >& ground) :
	solidTiles(levelSize.x, std::vector< Ground >(levelSize.y, Ground())),
	number(num),
	name(name1),
	aType(a),
	background(backgnd)
{
	for (const auto& block : ground) {
		const SDL_Point pos = block.getPosition();
		solidTiles[pos.x][pos.y] = block;
	}

	for (const auto& entity : ent) {
		entities.push_back(std::make_unique< PhysicsEntity >(entity));
	}
}

Act::Act(const Act& act) :
	number(act.number),
	name(act.name),
	solidTiles(act.solidTiles),
	background(act.background),
	blockPrefix(act.blockPrefix),
	backgroundFolder(act.backgroundFolder),
	aType(act.aType)
{
	for (const auto& entity : act.entities) {
		entities.push_back(std::make_unique< PhysicsEntity >(*entity));
	}
}

//Unload offscreen entities and update onscreen ones
void Act::updateEntities(Player& player, Camera& cam) {
	// Update entities
	std::vector< bool > toDestroy(entities.size(), false);
	std::vector< std::unique_ptr < PhysicsEntity > > toAdd;
	EntityManager manager(entities, toDestroy, toAdd);
	for (auto& entity : entities) {
		entity->update(&player, &manager);
	}

	updateCollisions(player, toDestroy, toAdd);

	player.update(solidTiles, manager);

	// Unload entities marked to be destroyed
	auto i = entities.begin();
	auto j = toDestroy.begin();
	while (j != toDestroy.end()) {
		if (*j) {
			j = toDestroy.erase(j);
			i = entities.erase(i);
		}
		else {
			++i;
			++j;
		}
	}

	// Add entities created by other entities
	for (auto& entityToAdd : toAdd) {
		entities.push_back(std::move(entityToAdd));
	}
}

void Act::loadVersion(std::ifstream& DataFile, Version version) {
	if (version >= Version{ 0, 0, 1 }) {
		Version temp;
		DataFile >> temp;
		assert(temp == version);
		assert(!DataFile.fail());
	};

	// Number
	DataFile >> number;

	// Name
	DataFile >> std::ws;
	std::getline(DataFile, name);

	// Entity reading
	while (DataFile >> std::ws, DataFile.peek() != 'E'){
		std::cout << "Reading entity, pos: ";

		Point pos{ 0, 0 };
		DataFile >> pos.x >> pos.y; 
		std::cout << "{ " << std::setw(5) << pos.x << ", " << std::setw(5) << pos.y << "}, id: ";

		std::string typeId;
		DataFile >> typeId;
		std::cout << std::setw(15) << typeId << ", flags: ";

		std::vector< char > flags;
		for (char flag = DataFile.get(); flag != '\n' && flag != '\r'; flag = DataFile.get()) {
			if (flag != ' ') {
				flags.push_back(flag);
				std::cout << flag;
			}
		}

		std::cout << "\n";

		using namespace entity_property_data;
		if (flags.size() != requiredFlagCount(getEntityTypeData(typeId).behaviorKey)) {
			throw std::logic_error("Invalid flag count for entity!");
		}

		entities.push_back(std::make_unique< PhysicsEntity >(typeId, flags, pos));
	}
	DataFile.get();

	if (version <= Version{ 0, 0, 1 }) {
		SDL_Rect winArea;
		DataFile >> winArea.x >> winArea.y >> winArea.w >> winArea.h;
		(void)winArea;
	}

	// Act Type
	DataFile >> aType;
	if (aType == ActType::NORMAL) {
		DataFile.ignore(std::numeric_limits< std::streamsize >::max(), '\n');

		// Load the tilemap
		std::string tilemapName;
		std::getline(DataFile, tilemapName);
		Ground::setMap(tilemapName);

		if (version >= Version{ 0, 0, 2 }) {
			std::getline(DataFile, blockPrefix);
			DataReader::LoadLevelBlocks(ASSET + blockPrefix);

			std::getline(DataFile, backgroundFolder);
			background = DataReader::LoadBackground(ASSET + backgroundFolder);
		}

		int numTiles;
		DataFile >> numTiles;

		SDL_Point levelSize;
		DataFile >> levelSize.x >> levelSize.y;

		solidTiles.resize(levelSize.x, std::vector< Ground >(levelSize.y));
		for (int i = 0; i < numTiles; ++i) {
			Ground block;
			DataFile >> block;
			solidTiles[block.getPosition().x / GROUND_PIXEL_WIDTH][block.getPosition().y / GROUND_PIXEL_WIDTH] = block;
		}
	}

	assert(DataFile.good());
}

void Act::save(std::ostream& stream) const {
	// File version
	stream << currentVersion << "\n";

	// Save act number
	stream << number << " ";

	// Save act name
	stream << name << "\n";

	// Save entities
	for (auto& entity : entities) {
		stream << entity->position.x << " " << entity->position.y << " " << entity->getKey();
		for (char c : entity->getFlags()) {
			stream << " " << c;
		}
		stream << "\n";
	}

	// Terminate the list of entities
	stream << "E\n";

	// Print act type (should be ActType::NORMAL)
	stream << aType << "\n";

	// Output ground tilemap without preceding asset path
	stream << Ground::getMapPath() << "\n";

	const std::size_t tileCount = std::accumulate(solidTiles.begin(), solidTiles.end(), 0ull, [](std::size_t value, const std::vector< Ground >& col) {
		return value + std::count_if(col.begin(), col.end(), [](const Ground& ground){ return ground.getIndex() != Ground::NO_TILE; });
	});

	stream << tileCount << "\n";

	for (auto& col : solidTiles) {
		for (auto& block : col) {
			stream << block;
		}
	}

	assert(stream.good());
}

void Act::updateCollisions(Player& player, std::vector< bool >& toDestroy, std::vector< std::unique_ptr< PhysicsEntity > >& toAdd) {
	bool hurt = false;
	for (auto& entity : entities) {
		if (intersects(player, *entity) && entity->canCollide) {
			player.addCollision(entity);
		}
	}
}

void Act::renderObjects(Player& player, Camera& cam) {
	const Point pos = cam.getPosition();
	const Rect cameraView = cam.getViewArea();

	globalObjects::renderBackground(background, cam);

	renderBlockLayer(cam, 1);

	if (player.getOnGround()) {
		player.render(cam);
	}

	for (const auto& entity : entities) {
		entity->Render(cam);
	}

	renderBlockLayer(cam, 0);

	if (globalObjects::debug) {
		SDL_Point start = ((SDL_Point{ int(cameraView.x), int(cameraView.y) } / GROUND_PIXEL_WIDTH) * GROUND_PIXEL_WIDTH);
		SDL_Point end = ((SDL_Point{ int(cameraView.x + cameraView.w), int(cameraView.y + cameraView.h) } / GROUND_PIXEL_WIDTH) * GROUND_PIXEL_WIDTH) + SDL_Point{ 1, 1 };
		start.x = std::max(start.x, 0);
		start.y = std::max(start.y, 0);
		end.x = std::min(end.x, int(solidTiles.size() * GROUND_PIXEL_WIDTH));
		end.y = std::min(end.y, int(solidTiles[0].size() * GROUND_PIXEL_WIDTH));
		for (int x = start.x; x < end.x; x += GROUND_PIXEL_WIDTH) {
			for (int y = start.y; y < end.y; y += GROUND_PIXEL_WIDTH) {
				drawing::drawRect(globalObjects::renderer, cam,
					Rect{ double(x), double(y), GROUND_PIXEL_WIDTH, GROUND_PIXEL_WIDTH },
					drawing::Color{ 255, 255, 255 },
					false
				);
			}
		}
		for (int x = start.x; x < end.x; x += TILE_WIDTH) {
			for (int y = start.y; y < end.y; y += TILE_WIDTH) {
				drawing::drawPoint(globalObjects::renderer, cam,
					Point{ double(x), double(y) },
					drawing::Color{ 200, 200, 200 },
					1
				);
			}
		}
	}

	if (!player.getOnGround()) {
		player.render(cam);
	}

	if (globalObjects::debug) {
		for (auto& entity : entities) {
			if (intersects(player, *entity)) {
				SDL_SetRenderDrawColor(globalObjects::renderer, 255, 0, 255, SDL_ALPHA_OPAQUE);
				Rect box = *entity->getAbsHitbox().hitbox.getAABoundingBox();

				drawing::drawRect(globalObjects::renderer, cam, box, drawing::Color{ 255, 0, 255 }, true);
			}
		}

		auto result = findGroundHeight(solidTiles, player.getPosition(), player.getMode(), player.getHitbox(), false, player.getPath(), Direction::DOWN);
		if (result) {
			drawing::drawPoint(globalObjects::renderer, cam, static_cast< Point >(result->first), drawing::Color{ 0, 0, 255 }, 4);
		}

		auto lResult = findGroundHeight(solidTiles, player.getPosition(), player.getMode(), player.getWallHitbox(), false, player.getPath(), Direction::LEFT);
		auto rResult = findGroundHeight(solidTiles, player.getPosition(), player.getMode(), player.getWallHitbox(), false, player.getPath(), Direction::RIGHT);

		if (lResult) {
			drawing::drawPoint(globalObjects::renderer, cam, static_cast< Point >(lResult->first), drawing::Color{ 0, 255, 255 }, 4);
		}
		if (rResult) {
			drawing::drawPoint(globalObjects::renderer, cam, static_cast< Point >(rResult->first), drawing::Color{ 255, 255, 0 }, 4);
		}

		SDL_Point mousePos;
		SDL_GetMouseState(&mousePos.x, &mousePos.y);
		mousePos /= cam.scale;
		mousePos += static_cast< SDL_Point >(cam.getPosition());

		CollisionTile tile0 = getTile(mousePos, solidTiles, false);
		CollisionTile tile1 = getTile(mousePos, solidTiles, true);
		
		if (tile0.getIndex() || tile1.getIndex()) {
			Point topLeft = static_cast< Point >((mousePos / TILE_WIDTH) * TILE_WIDTH);
			drawing::drawRect(globalObjects::renderer, cam,
				Rect{ topLeft.x, topLeft.y, TILE_WIDTH, TILE_WIDTH },
				drawing::Color{ 255, 255, 255 },
				false
			);
		}

		std::stringstream tileDebugInfo;
		
		auto printThings = [](std::stringstream& stream, const CollisionTile& tile) {
			stream << "idx: " << tile.getIndex() << ", raw angle: " << tile.getAngle() << ", flags: ";
			if (tile.flags & SDL_FLIP_HORIZONTAL) stream << "H";
			if (tile.flags & SDL_FLIP_VERTICAL)   stream << "V";
			if (tile.flags & (int)Ground::Flags::TOP_SOLID) stream << "T";
		};

		if (tile0.getIndex()) {
			printThings(tileDebugInfo, tile0);
			tileDebugInfo << "\n";
		}
		if (tile1.getIndex() && tile1.getIndex() != tile0.getIndex()) {
			printThings(tileDebugInfo, tile1);
			tileDebugInfo << "\n";
		}

		text::renderAbsolute({ 250, 1 }, "GUI", tileDebugInfo.str());
	}

}

void Act::renderBlockLayer(const Camera& cam, int layer) const {
	const Rect cameraView = cam.getViewArea();

	const std::size_t leftTile = std::clamp< int >((cameraView.x - GROUND_PIXEL_WIDTH) / GROUND_PIXEL_WIDTH, 0, solidTiles.size() - 1);
	const std::size_t rightTile = std::clamp< int >((cameraView.x + cameraView.w + GROUND_PIXEL_WIDTH) / GROUND_PIXEL_WIDTH, 0, solidTiles.size() - 1);
	const std::size_t topTile = std::clamp< int >((cameraView.y - GROUND_PIXEL_WIDTH) / GROUND_PIXEL_WIDTH, 0, solidTiles[0].size() - 1);
	const std::size_t bottomTile = std::clamp< int >((cameraView.y + cameraView.h + GROUND_PIXEL_WIDTH) / GROUND_PIXEL_WIDTH, 0, solidTiles[0].size() - 1);
	for (std::size_t tileX = leftTile; tileX <= rightTile; ++tileX) {
		for (std::size_t tileY = topTile; tileY <= bottomTile; ++tileY) {
			const Ground& block = solidTiles[tileX][tileY];
			if (block.getIndex() != Ground::NO_TILE) {
				block.Render(cam, nullptr, layer);
			}
		}
	}
}

using namespace std::literals::string_view_literals;
static constexpr const std::array< std::string_view, 3 > actTypeStrings{ "TITLE"sv, "TORNADO"sv, "NORMAL"sv };

std::istream& operator>> (std::istream& stream, Act::ActType& type) {
	std::string next;
	stream >> next;
	if (const auto& result = std::find(actTypeStrings.begin(), actTypeStrings.end(), next); result != actTypeStrings.end()) {
		type = static_cast< Act::ActType >(std::distance(actTypeStrings.begin(), result));
		return stream;
	}
	else {
		stream.setstate(std::istream::failbit);
		return stream;
	}
}

std::ostream& operator<< (std::ostream& stream, Act::ActType  type) {
	stream << actTypeStrings[static_cast< std::size_t >(type)];
	return stream;
}
