#include "stdafx.h"
#include "DataReader.h"
#include "Ground.h"
#include "CollisionTile.h"
#include "Constants.h"
#include <fstream>
#include <iomanip>
#include <unordered_map>

enum class ActType : unsigned char { TITLE, TORNADO, NORMAL };

//Block and blockFlags refer to the visible tile mappings,
//Collide and collideFlags refer to the collision mappings
void DataReader::LoadJSONBlock(const std::string& path) {
	Ground::groundArrayData arrayData;

	std::ifstream DataFile(path.c_str());
	json j;
	DataFile >> j;
	DataFile.close();
	
	const auto& layers = j["layers"];

	const int numLayers = layers.size();
	bool isDoubleLayer = false;

	const int MAX_GRAPHICS_TILE_INDEX = 339;

	auto loadGraphicsLayer = [&](int layer) {
		for (int i = 0; i < GROUND_SIZE; ++i) {
			const auto& data = layers[layer]["tiles"][i];
			auto& tile = arrayData.graphics[layer][i];
			// Set tile index for graphics layer 0
			tile = { data["tile"].get<int>(), 0 };
			
			// Sets flags for graphics layer 0
			if (data["rot"].get<int>() == 2) {
				tile.flags |= SDL_FLIP_VERTICAL;
				tile.flags |= (!data["flipX"].get<bool>()) * SDL_FLIP_HORIZONTAL;
			}
			else {
				tile.flags |= data["flipX"].get<bool>() * SDL_FLIP_HORIZONTAL;
			}
		}
	};

	// Load image data
	arrayData.graphics.resize(1);
	loadGraphicsLayer(0);

	if (numLayers >= 2 && layers[1]["tiles"][0]["tile"].get<int>() <= MAX_GRAPHICS_TILE_INDEX) {
		isDoubleLayer = true;

		// Load image data for layer 2
		arrayData.graphics.resize(2);
		loadGraphicsLayer(1);
	}

	const bool hasAnimatedLayer = (layers[isDoubleLayer]["name"] == "Animation");

	// Handle animation
	if (hasAnimatedLayer) {
		
	}

	const int collisionLayersStart = 1 + isDoubleLayer + hasAnimatedLayer;

	const bool hasAdditionalFlags = (layers.back()["name"] == "AdditionalFlags");
	arrayData.collision.resize(numLayers - collisionLayersStart - hasAdditionalFlags, { MAX_GRAPHICS_TILE_INDEX + 1, 0 });

	for (int l = 0; l < numLayers - collisionLayersStart - hasAdditionalFlags; ++l) {
		// Load collision data
		const auto& layerTiles = layers[l + collisionLayersStart]["tiles"];
		std::transform(layerTiles.begin(), layerTiles.end(), arrayData.collision[l].begin(),
			[](const json& tile) {
				int current = tile["tile"].get<int>();
				Ground::Tile result{ ((current == 591) ? 560 : current) - 340, 0 };
				if (tile["rot"].get<int>() == 2) {
					result.flags |= SDL_FLIP_VERTICAL;
					result.flags |= (!tile["flipX"].get<bool>()) * SDL_FLIP_HORIZONTAL;
				}
				else {
					result.flags |= (tile["flipX"].get<bool>()) * SDL_FLIP_HORIZONTAL;
				}
				return result;
			});
	}

	if (hasAdditionalFlags) {
		for (int i = 0; i < GROUND_SIZE; ++i) {
			if (layers.back()["tiles"][i]["tile"].get<int>() == 592) {
				arrayData.collision[0][i].flags |= static_cast < int >(Ground::Flags::TOP_SOLID);
			}
		}
	}

	Ground::addTile(arrayData);

	DataFile.close();
}

void DataReader::LoadActData(const std::string& path, int& n, std::string& name1, std::vector < PhysStruct >& entities, SDL_Rect& winArea, ActType& actType, std::vector < Ground >& ground, SDL_Point& levelSize) {
	std::ifstream DataFile;
	std::string data;
	DataFile.open(path + "Data.txt");

	if (DataFile.fail()) {
		std::cerr << "Data file does not exist!";
		throw "Data file does not exist!";
	}
	
	//Number
	DataFile >> n;

	//Name
	DataFile.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	std::getline(DataFile, name1);

	//Entity reading
	entities.clear();
	do {
		std::cout << "Destroying entity!\n";
		PhysStruct current;
		DataFile >> data;
		if (data == "E") {
			break;
		}
		current.position.x = atoi(data.c_str());
		DataFile >> data;
		current.position.y = atoi(data.c_str());
		current.position.w = current.position.h = 16;
		DataFile >> data;
		current.typeId = data;
		char flag = DataFile.get();
		while (flag != '\n' && flag != '\r') {
			if (flag == ' ') {
				flag = DataFile.get();
				continue;
			}
			current.flags.push_back(flag);
			flag = DataFile.get();
		}
		using namespace entity_property_data;
		if (current.flags.size() != requiredFlagCount(getEntityTypeData(current.typeId).behaviorKey)) {
			throw std::logic_error("Invalid flag count for entity!");
		}
		entities.push_back(current);
	} while (data != "E");

	//Win Area
	DataFile >> data;
	winArea.x = atoi(data.c_str());
	DataFile >> data;
	winArea.y = atoi(data.c_str());
	DataFile >> data;
	winArea.w = atoi(data.c_str());
	DataFile >> data;
	winArea.h = atoi(data.c_str());

	//Act Type
	DataFile >> data;
	if (data == "TITLE") {
		actType = ActType::TITLE;
	}
	else if (data == "TORNADO") {
		actType = ActType::TORNADO;
	}
	else if (data == "NORMAL") {
		actType = ActType::NORMAL;
		//Start loading ground:

		//Load the tilemap
		DataFile >> data;
		Ground::setMap(std::string{ ASSET } + data);

		int count = 0;
		DataFile >> data;
		int numTiles = atoi(data.c_str());
		DataFile >> data;
		levelSize.x = std::stoi(data);
		DataFile >> data;
		levelSize.y = std::stoi(data);
		ground.resize(numTiles);

		while (count < numTiles) {
			DataFile >> ground[count];
			++count;
		}
	}

	DataFile.close();
}

void DataReader::LoadEntityData(const std::string& path) {
	std::ifstream DataFile(path);

	if (DataFile.fail()) {
		DataFile.close();
		throw "Datafile could not be loaded!";
		return;
	}

	char nextChar = 'a';

	while (!DataFile.eof()) {
		std::string next = "";
		entity_property_data::EntityType current;
		auto getInt = [&]() {
			std::string next;
			DataFile >> next;
			std::cout << next << " ";
			return std::stoi(std::move(next));
		};
		auto getDouble = [&]() {
			std::string next;
			DataFile >> next;
			std::cout << next << " ";
			return std::stod(std::move(next));
		};
		auto& types = entity_property_data::entityTypes;
		DataFile >> next;
		std::cout << next << " ";
		DataFile >> next;
		std::cout << next << " ";
		std::string id = std::move(next);
		current.collisionRect = SDL_Rect{ getInt(), getInt(), getInt(), getInt() };
		current.defaultVelocity = { getDouble(), getDouble() };
		current.defaultGravity = getDouble();
		while (true) {
			DataFile >> next;
			std::cout << next << " ";
			if (next == "EA") {
				break;
			}
			else {
				AnimStruct a;
				a.SpritePath = ASSET + next;
				a.delay = std::chrono::milliseconds(getInt());
				a.frames = getInt();
				current.animationTypes.push_back(a);
			}
		}
		DataFile >> next;
		std::cout << next << " ";
		current.behaviorKey = entity_property_data::stringToKey(next);
		DataFile >> next;
		std::cout << next << "\n";
		types.emplace(std::move(id), std::move(current));
		std::cout << "Done loading entity!\n";
		DataFile.ignore(2);
		nextChar = DataFile.peek();
	}

	std::cout << "All entity loading complete!\n";
	DataFile.close();
}

void DataReader::LoadTileData(const std::string& path, std::vector < CollisionTile >& tiles) {
	std::ifstream DataFile;
	DataFile.open(path + "Tiles.txt");
	std::string data;
	CollisionTile currentTile = CollisionTile();
	std::vector < int > currentHeights;
	int count = 0;

	if (DataFile.fail()) {
		std::cerr << "File does not exist!";
		throw "File does not exist";
	}

	while (!DataFile.eof()) {
		DataFile >> data;
		if (data == "false") {
			currentTile.setHeights(std::vector < int >(TILE_WIDTH,0));
			currentTile.setCollide(false);
		}
		else {
			currentTile.setHeight(atoi(data.c_str()), 0);
			for (int i = 1; i < 16; i++) {
				DataFile >> data;
				currentTile.setHeight(atoi(data.c_str()), i);
			}
			DataFile >> data;
			currentTile.setAngle(atof(data.c_str()));
			currentTile.setCollide(true);
		}
		currentTile.setIndex(count);
		currentTile.calculateSideMap();
		tiles.push_back(currentTile);
		count++;
	}
	tiles.pop_back();

	DataFile.close();
}

void DataReader::LoadTileData(std::vector < CollisionTile >& tiles, matrix < int >& heights, std::vector < double >& angles) {
	CollisionTile currentTile;
	for (int i = 0; i < heights.size(); i++) {
		currentTile.setIndex(i);
		currentTile.setAngle(angles[i]);
		currentTile.setHeights(heights[i]);
		currentTile.calculateSideMap();
		bool collides(false);
		for (int j = 0; j < TILE_WIDTH; j++) {
			if (heights[i][j] != 0) {
				collides = true;
				break;
			}
		}
		currentTile.setCollide(collides);
		tiles.push_back(currentTile);
	}
};

void DataReader::LoadCollisionsFromImage(const std::string& path, matrix < int >& heights, std::vector < double >& angles) {
	SDL_Surface* s = IMG_Load(path.c_str());
	
	SDL_Surface* DataFile = SDL_ConvertSurface(s, SDL_GetWindowSurface(globalObjects::window)->format, 0u);
	SDL_FreeSurface(s);
	if (DataFile == NULL) {
		throw "Could not open image file!\n";
	}

	if (SDL_MUSTLOCK(DataFile)) {
		SDL_LockSurface(DataFile);
	}

	Uint32 cmask, gmask, dmask;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	cmask = 0xFFFFFF00;
	gmask = 0x00FF0000;
	dmask = 0x00010000;
#else
	cmask = 0x00FFFFFF;
	gmask = 0x0000FF00;
	dmask = 0x00000100;
#endif

	Uint32 color1 = ::getPixel(DataFile, 16, 15);

	heights.resize(224);
	angles.resize(224);
	for (int tileY = 0; tileY < 7 * 16; tileY += 16) {
		std::cout << "Reading collision data on row " << tileY / 16 << "\n";
		for (int tileX = 0; tileX < DataFile->w; tileX += 16) {
			//std::cout << "\tReading collision data on column " << tileX / 16 << '\n';
			heights[tileX / 16 + tileY * 2].resize(16, 0);
			for (int x = 0; x < 16; x++) {
				for (int y = 15; y >= 0; y--) {
					Uint32 color = ::getPixel(DataFile, tileX + x, tileY + y);
					if (color & cmask) {
						assert((color & gmask) / dmask >= 0);
						int tempAngle = (color & gmask) / dmask;
						angles[tileX / 16 + tileY * 2] = ((tempAngle == 255) ? 0.0 : tempAngle);
						heights[tileX / 16 + tileY * 2][x] = 16 - y;
					}
					else {
						break;
					}
				}
			}
		}
	}

	SDL_FreeSurface(DataFile);
};

void DataReader::LoadBackground(const filesystem::path& directory, std::vector < std::vector < Animation > >& background) {
	SDL_Surface* current;
	background = std::vector < std::vector < Animation > >(8, std::vector<Animation>(2, Animation{}));
	for (const auto& file : filesystem::directory_iterator(directory)) {
		if (file.path().extension() == ".png") {
			auto tile = file.path().filename().string()[0] - '0';
			auto layer = file.path().filename().string()[2] - '0';

			using namespace std::chrono_literals;

			background[layer][tile] = Animation{ AnimStruct{ file.path().string(), 200ms, 1 } }; 

			if (layer == 0) {
				using namespace animation_effects;
				std::vector < AnimationEffectList > effects(4, AnimationEffectList(1, PaletteSwap{}));
				auto color = [](auto r, auto g, auto b) {
					return SDL_MapRGBA(&imageFormat, r, g, b, SDL_ALPHA_OPAQUE);
				};
				std::vector < Uint32 > oldColors{ color(0x60,0x80,0xa0), color(0x60,0x80,0xe0), color(0x80,0xa0,0xe0), color(0xa0,0xc0,0xe0) };
				std::vector < std::vector < Uint32 > > newColors{
					{
						oldColors[0], oldColors[1], oldColors[2], oldColors[3]
					},
					{
						oldColors[2], oldColors[0], oldColors[1], oldColors[3]
					}, 
					{
						oldColors[3], oldColors[2], oldColors[0], oldColors[1]
					},
					{
						oldColors[1], oldColors[3], oldColors[2], oldColors[0]
					}
				};
				for(std::size_t i = 0; i < effects.size(); ++i) {
					auto& effect = std::get<PaletteSwap>(effects[i][0]);
					effect.oldColors = oldColors;
					effect.newColors = newColors[i];
				}
				background[0][tile].addStaticEffects(effects);
			}
		}
	}
	std::cout << "Background loading complete.\n";
}

void DataReader::LoadLevelBlocks(const std::string& path) {
	int currentBlockNum = 0;
	std::string currentBlock = path + std::to_string(currentBlockNum + 1) + ".json";
	Ground::clearTiles();
	std::ifstream fileCheck;
	while (true) {
		fileCheck.open(currentBlock.c_str());
		if (!fileCheck.good()) {
			break;
		}
		fileCheck.close();
		LoadJSONBlock(currentBlock); 
		++currentBlockNum;
		currentBlock = path + std::to_string(currentBlockNum + 1) + ".json";
	}
}
