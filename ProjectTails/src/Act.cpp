#include "Player.h"
#include "DataReader.h"
#include "Act.h"
#include "Text.h"
#include "Version.h"
#include "Camera.h"
#include "Constants.h"
#include "Ground.h"
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

		SDL_Rect pos{ 0, 0, 16, 16 };
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

		entities.push_back(std::make_unique< PhysicsEntity >(typeId, flags, pos, false));
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
		const SDL_Rect playerCollide = player.getCollisionRect();
		const SDL_Rect objCollide = entity->getCollisionRect();
		if (SDL_HasIntersection(&playerCollide, &objCollide) && entity->canCollide) {
			player.addCollision(entity);
		}
	}
}

void Act::renderObjects(Player& player, Camera& cam) {
	const SDL_Point pos = cam.getPosition();
	const SDL_Rect cameraView = cam.getCollisionRect();

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
		SDL_SetRenderDrawColor(globalObjects::renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
		static Text tileText(ASSET"FontGUI.png");
		SDL_Point start = ((getXY(cameraView) / GROUND_PIXEL_WIDTH) * GROUND_PIXEL_WIDTH);
		SDL_Point end = ((SDL_Point{ cameraView.x + cameraView.w, cameraView.y + cameraView.h } / GROUND_PIXEL_WIDTH) * GROUND_PIXEL_WIDTH);
		start.x = std::max(start.x, 0);
		start.y = std::max(start.y, 0);
		end.x = std::min(end.x, int(solidTiles.size() * GROUND_PIXEL_WIDTH));
		end.y = std::min(end.y, int(solidTiles[0].size() * GROUND_PIXEL_WIDTH));
		for (int x = start.x; x < solidTiles.size() * GROUND_PIXEL_WIDTH; x += GROUND_PIXEL_WIDTH) {
			for (int y = start.y; y < solidTiles.size() * GROUND_PIXEL_WIDTH; y += GROUND_PIXEL_WIDTH) {
				const SDL_Rect position = SDL_Rect{ x - pos.x, y - pos.y, GROUND_PIXEL_WIDTH, GROUND_PIXEL_WIDTH } * 2;
				SDL_RenderDrawRect(globalObjects::renderer, &position);
				//tileText.setText(std::to_string(int(solidTiles[x / GROUND_PIXEL_WIDTH][y / GROUND_PIXEL_WIDTH].getIndex())));
				//tileText.Render(getXY(position));
			}
		}
	}

	if (!player.getOnGround()) {
		player.render(cam);
	}

	if (globalObjects::debug) {
		const auto camPos = cam.getPosition();
		for (int i = 0; i < 6; ++i) {
			SDL_SetRenderDrawColor(globalObjects::renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
			/*if (auto result = player.checkSensor(Player::Sensor(i), solidTiles)) {
				const auto& pos = (*player.getSensorPoint(result, solidTiles) - camPos) / 3 * 2;
				SDL_RenderSetScale(globalObjects::renderer, 3, 3);
				SDL_RenderDrawPoint(globalObjects::renderer, pos.x, pos.y);
				SDL_RenderSetScale(globalObjects::renderer, 1, 1);
			}*/
			SDL_SetRenderDrawColor(globalObjects::renderer, i % 2 * 255, (i % 4 >> 1) * 255, (i >> 2) * 255, SDL_ALPHA_OPAQUE);

			if (auto result = player.getSensorPoint(static_cast< Player::Sensor >(i), solidTiles)) {
				const auto pos = (*result - camPos) / 3 * 2;
				SDL_RenderSetScale(globalObjects::renderer, 3, 3);
				SDL_RenderDrawPoint(globalObjects::renderer, pos.x, pos.y);
				SDL_RenderSetScale(globalObjects::renderer, 1, 1);
			}
			auto [xRange, yRange, dir] = player.getRange(Player::Sensor(i));
			auto first = (SDL_Point{ xRange.first, yRange.first } - camPos) * 2;
			auto second = (SDL_Point{ xRange.second, yRange.second } - camPos) * 2;

			SDL_RenderDrawLine(globalObjects::renderer, first.x, first.y, second.x, second.y);
		}
	}
}

void Act::renderBlockLayer(const Camera& cam, int layer) const {
	const SDL_Rect cameraView = cam.getCollisionRect();

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
