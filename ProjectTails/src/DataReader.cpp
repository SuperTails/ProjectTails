#include "stdafx.h"
#include "DataReader.h"
#include "Ground.h"
#include "CollisionTile.h"
#include "Constants.h"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <unordered_map>


//Block and blockFlags refer to the visible tile mappings,
//Collide and collideFlags refer to the collision mappings
void DataReader::LoadJSONBlock(const std::string& path) {
	std::ifstream DataFile(path.data());
	json j;
	DataFile >> j;
	DataFile.close();
	
	Ground::GroundData arrayData;
	arrayData = j;

	Ground::addTile(arrayData);
}

void DataReader::LoadEntityData(const std::string& path) {
	std::ifstream DataFile(path.data());

	if (DataFile.fail()) {
		DataFile.close();
		throw "Datafile could not be loaded!";
		return;
	}

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
		current.collisionRect = { getInt(), getInt(), getInt(), getInt() };
		current.defaultVelocity = { getDouble(), getDouble() };
		current.defaultGravity = getDouble();
		while (DataFile >> next, std::cout << next << " ", next != "EA") {
			AnimStruct a;
			a.SpritePath = ASSET + next;
			a.delay = std::chrono::milliseconds(getInt());
			a.frames = getInt();
			current.animationTypes.push_back(a);
		}
		DataFile >> next;
		std::cout << next << " ";
		current.behaviorKey = entity_property_data::behaviorKeyFromId(next);
		if (globalObjects::debug && current.behaviorKey == entity_property_data::Key::PATHSWITCH) {
			using namespace std::chrono_literals;
			current.animationTypes.push_back({ ASSET"Pathswitch.png", 10ms, 1 });
		}
		DataFile >> next;
		std::cout << next << "\n";
		types.emplace(std::move(id), std::move(current));
		std::cout << "Done loading entity!\n";
		DataFile.ignore(2);
		DataFile.peek();
	}

	std::cout << "All entity loading complete!\n";
}


std::vector< std::vector< Animation > > DataReader::LoadBackground(const filesystem::path& directory) {
	std::vector < std::vector < Animation > > background;
	if (!filesystem::exists(directory)) {
		throw std::invalid_argument("Background directory does not exist!");
	}

	background = std::vector< std::vector< Animation > >(8, std::vector< Animation >(2));
	for (const auto& file : filesystem::directory_iterator(directory)) {
		if (file.path().extension() == ".png") {
			const auto tile = file.path().filename().string()[0] - '0';
			const auto layer = file.path().filename().string()[2] - '0';

			using namespace std::chrono_literals;

			background[layer][tile] = Animation{ AnimStruct{ file.path().string(), 250ms, 1 } }; 
			background[layer][tile].setOffset({ { 0, 0 } });

			if (layer == 0) {
				using namespace animation_effects;
				auto color = [](auto r, auto g, auto b) {
					return SDL_MapRGBA(&imageFormat, r, g, b, SDL_ALPHA_OPAQUE);
				};
				const std::array< Uint32, 4 > oldColors{ color(0x60,0x80,0xa0), color(0x60,0x80,0xe0), color(0x80,0xa0,0xe0), color(0xa0,0xc0,0xe0) };
				const std::array< std::array< Uint32, 4 >, 4 > newColors{
					std::array< Uint32, 4 >{
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
				std::vector< AnimationEffectList > effects(4, AnimationEffectList(1, PaletteSwap{}));
				for(std::size_t i = 0; i < effects.size(); ++i) {
					auto& effect = std::get<PaletteSwap>(effects[i][0]);
					effect.oldColors.insert(effect.oldColors.end(), oldColors.begin(), oldColors.end());
					effect.newColors.insert(effect.newColors.end(), newColors[i].begin(), newColors[i].end());
				}
				background[0][tile].addStaticEffects(effects);
			}
		}
	}
	std::cout << "Background loading complete.\n";
	return background;
}

void DataReader::LoadLevelBlocks(const std::string& path) {
	Ground::clearTiles();

	int currentBlockNum = 0;
	while (true) {
		const std::string currentBlockPath = path + std::to_string(currentBlockNum + 1) + ".json";

		if (std::ifstream fileCheck{ currentBlockPath }; !fileCheck.good()) {
			break;
		}

		LoadJSONBlock(currentBlockPath); 
		++currentBlockNum;
	}
}
