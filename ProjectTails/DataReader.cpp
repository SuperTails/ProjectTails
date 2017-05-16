#include "stdafx.h"
#include "DataReader.h"

//Block and blockFlags refer to the visible tile mappings,
//Collide and collideFlags refer to the collision mappings
void DataReader::LoadJSONBlock(std::string path, Ground::groundArrayData& arrayData) {
	std::ifstream DataFile;
	DataFile.open(path.c_str());
	json j;
	DataFile >> j;

	int numLayers = j["layers"].size();
	bool isDoubleLayer = false;

	const int MAX_GRAPHICS_TILE_INDEX = 339;

	arrayData.graphicsIndices.resize(256, 0);
	arrayData.graphicsFlags.resize(256, 0);

	//Load image data
	for (int i = 0; i < 256; i++) {
		auto& thisTile = j["layers"][0]["tiles"][i];

		arrayData.graphicsIndices[i] = thisTile["tile"].get<int>();

		if (thisTile["rot"].get<int>() == 2) {
			arrayData.graphicsFlags[i] |= SDL_FLIP_VERTICAL;
			arrayData.graphicsFlags[i] |= (!thisTile["flipX"].get<bool>()) * SDL_FLIP_HORIZONTAL;
		}
		else {
			arrayData.graphicsFlags[i] |= (thisTile["flipX"].get<bool>() * SDL_FLIP_HORIZONTAL);
		}
	}
	if (numLayers >= 2) {
		if (j["layers"][1]["tiles"][0]["tile"].get<int>() <= MAX_GRAPHICS_TILE_INDEX) {
			isDoubleLayer = true;
			//Load image data for layer 2
			arrayData.graphicsIndices.resize(512);
			arrayData.graphicsFlags.resize(512);
			for (int i = 0; i < 256; i++) {
				auto& thisTile = j["layers"][1]["tiles"][i];
				arrayData.graphicsIndices[i + 256] = thisTile["tile"].get<int>();
				if (thisTile["rot"].get<int>() == 2) {
					arrayData.graphicsFlags[i + 256] |= SDL_FLIP_VERTICAL;
					arrayData.graphicsFlags[i + 256] |= (!thisTile["flipX"].get<bool>()) * SDL_FLIP_HORIZONTAL;
				}
				else {
					arrayData.graphicsFlags[i + 256] |= (thisTile["flipX"].get<bool>() * SDL_FLIP_HORIZONTAL);
				}
			}
		}
	}
	arrayData.collideIndices.resize(256 * (numLayers - isDoubleLayer - 1), MAX_GRAPHICS_TILE_INDEX + 1);
	arrayData.collideFlags.resize(256 * (numLayers - isDoubleLayer - 1), 0);
	for (int l = 1 + isDoubleLayer; l < numLayers; l++) {
		//Load collision data
		for (int i = 0; i < 256; i++) {
			auto& thisTile = j["layers"][l]["tiles"][i];
			int current = thisTile["tile"].get<int>();
			if (current == 591) {
				current = 560;
			}
			arrayData.collideIndices[i + 256 * (l - isDoubleLayer - 1)] = current - 340;
			if (thisTile["rot"].get<int>() == 2) {
				arrayData.collideFlags[i + 256 * (l - isDoubleLayer - 1)] |= SDL_FLIP_VERTICAL;
				arrayData.collideFlags[i + 256 * (l - isDoubleLayer - 1)] |= (!thisTile["flipX"].get<bool>()) * SDL_FLIP_HORIZONTAL;
			}
			else {
				arrayData.collideFlags[i + 256 * (l - isDoubleLayer - 1)] |= (thisTile["flipX"].get<bool>() * SDL_FLIP_HORIZONTAL);
			}
		}
	}

	DataFile.close();
}

void DataReader::LoadActData(std::string path, int& n, std::string& name1, std::vector < PhysStructInit >& entities, SDL_Rect& winArea, ActType& actType, std::vector < Ground >* ground, std::vector < Ground::groundArrayData >* arrayData, std::vector < groundData >* groundIndices, SDL_Point* levelSize) {
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
	char* temp = new char[30];
	DataFile.getline(temp, 30, '\n');
	name1 = std::string(temp);
	delete[] temp;

	//Entity reading
	entities.clear();
	do {
		PhysStructInit current;
		DataFile >> data;
		current.pos.x = atoi(data.c_str());
		DataFile >> data;
		current.pos.y = atoi(data.c_str());
		current.pos.w = current.pos.h = 16;
		DataFile >> data;
		current.prop = data;
		char flag = DataFile.get();
		while (flag != '\n') {
			if (flag == ' ') {
				flag = DataFile.get();
				continue;
			}
			current.flags.push_back(flag);
			flag = DataFile.get();
		}
		entities.push_back(current);
	} while (data != "E");
	entities.pop_back();

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
		actType = TITLE;
	}
	else if (data == "TORNADO") {
		actType = TORNADO;
	}
	else if (data == "NORMAL") {
		actType = NORMAL;
		//Start loading ground:

		//Load the tilemap
		DataFile >> data;
		Ground::setMap(data);

		int count = 0;
		DataFile >> data;
		int numTiles = atoi(data.c_str());
		DataFile >> data;
		levelSize->x = std::stoi(data);
		DataFile >> data;
		levelSize->y = std::stoi(data);
		ground->resize(numTiles);
		groundIndices->resize(numTiles);
		while (count < numTiles) {
			DataFile >> data;
			SDL_Point a;
			a.x = atoi(data.c_str());
			DataFile >> data;
			a.y = atoi(data.c_str());
			DataFile >> data;
			int ind = std::stoi(data);
			char next = DataFile.get();
			bool flip = false;
			if (next != EOF && next != '\n')
				flip = DataFile.get() - '0';
			if (groundIndices != nullptr) {
				(*groundIndices)[count] = DataReader::groundData{ a.x, a.y, ind, flip };
			}
			(*ground)[count] = Ground(a, (*arrayData)[ind], flip);
			count++;
		}
	}

	DataFile.close();
}

void DataReader::LoadEntityData(std::string path, std::vector < PhysProp >& prop, std::unordered_map < std::string, PhysProp* >& entityKeys, std::vector < std::string >& Types) {
	std::ifstream DataFile(path);

	if (DataFile.fail()) {
		DataFile.close();
		throw "Datafile could not be loaded!";
		return;
	}


	int status = 0;
	while (!DataFile.eof()) {
		status = 0;
		std::string next = "";
		PhysProp current;
		//while (next != "ENDOBJ") {
		switch (status) {
		case 0:
			DataFile >> next;
			std::cout << next << " ";
			status++;
		case 1:
			DataFile >> next;
			std::cout << next << " ";
			Types.push_back(next);
			current.key = Types.back();
			status++;
		case 2:
			DataFile >> next;
			std::cout << next << " ";
			current.collision.x = atoi(next.c_str());
			DataFile >> next;
			std::cout << next << " ";
			current.collision.y = atoi(next.c_str());
			DataFile >> next;
			std::cout << next << " ";
			current.collision.w = atoi(next.c_str());
			DataFile >> next;
			std::cout << next << " ";
			current.collision.h = atoi(next.c_str());
			//case 2.5:
			DataFile >> next;
			std::cout << next << " ";
			current.vel.x = (double)atof(next.c_str());
			DataFile >> next;
			std::cout << next << " ";
			current.vel.y = (double)atof(next.c_str());
			//case 2 3/4:
			DataFile >> next;
			std::cout << next << " ";
			current.gravity = atoi(next.c_str());
			status++;
		case 3:
			while (true) {
				DataFile >> next;
				std::cout << next << " ";
				if (next != "EA") {
					AnimStruct a;
					a.SpritePath = next;
					DataFile >> next;
					std::cout << next << " ";
					a.delay = atoi(next.c_str());
					DataFile >> next;
					std::cout << next << " ";
					a.frames = atoi(next.c_str());
					current.anim.push_back(a);
				}
				else {
					break;
				}
			}
			status++;
		case 4:
			DataFile >> next;
			std::cout << next << " ";
			if (next == "ENEMY") {
				current.eType = ENEMY;
			}
			else if (next == "RING") {
				current.eType = RING;
			}
			else if (next == "WEAPON") {
				current.eType = WEAPON;
			}
			else if (next == "PHYSICS") {
				current.eType = PATHSWITCH;
			}
			else if (next == "SPRING") {
				current.eType = SPRING;
			}
			else if (next == "PLATFORM") {
				current.eType = PLATFORM;
			}
			else if (next == "SPIKES") {
				current.eType = SPIKES;
			}
			else if (next == "MONITOR") {
				current.eType = MONITOR;
			}
			else if (next == "GOALPOST") {
				current.eType = GOALPOST;
			}
			status++;
		case 5:
			DataFile >> next;
			std::cout << next << "\n";
		}
		prop.push_back(current);
		//}
		std::cout << "Done loading entity!\n";
	}

	for (PhysProp& i : prop) {
		entityKeys.emplace(i.key, &i);
	}
	std::cout << "All entity loading complete!\n";
	DataFile.close();
}

void DataReader::LoadTileData(std::string path, std::vector < CollisionTile >& tiles) {
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
			currentTile.setHeights(std::vector < int >(16,0));
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
		for (int j = 0; j < 16; j++) {
			if (heights[i][j] != 0) {
				collides = true;
				break;
			}
		}
		currentTile.setCollide(collides);
		tiles.push_back(currentTile);
	}
};

void DataReader::LoadTileBlocks(std::string path, matrix < int >& blocks, matrix < int >& blockFlags) {
	std::ifstream DataFile;
	DataFile.open(path + "Blocks.txt");
	std::string data;
	DataFile >> data;
	int numBlocks = atoi(data.c_str());
	int data1;
	char nextFlag = ' ';
	blocks.resize(numBlocks);
	blockFlags.resize(numBlocks);
	for (int j = 0; j < numBlocks; j++) {
		blocks[j].resize(256);
		blockFlags[j].resize(256);
		for (int i = 0; i < 256; i++) {
			DataFile >> data1;
			blocks[j][i] = data1;
			blockFlags[j][i] = 0;
			do {
				DataFile.get(nextFlag);
				if (nextFlag == 'h') {
					blockFlags[j][i] |= SDL_FLIP_HORIZONTAL;
				}
				else if (nextFlag == 'v') {
					blockFlags[j][i] |= SDL_FLIP_VERTICAL;
				}
			} while(nextFlag != ' ' && nextFlag != '\n');
		}

	}

	DataFile.close();
};

void DataReader::LoadCollisionsFromImage(std::string path, matrix < int >& heights, std::vector < double >& angles, SDL_Window* window) {
	SDL_Surface* s = IMG_Load(path.c_str());
	
	SDL_Surface* DataFile = SDL_ConvertSurface(s, SDL_GetWindowSurface(window)->format, NULL);
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

	Uint32 color1 = Animation::getPixel(DataFile, 16, 15);

	heights.resize(224);
	angles.resize(224);
	for (int tileY = 0; tileY < 7 * 16; tileY += 16) {
		std::cout << "Reading collision data on row " << tileY / 16 << "\n";
		for (int tileX = 0; tileX < DataFile->w; tileX += 16) {
			//std::cout << "\tReading collision data on column " << tileX / 16 << '\n';
			heights[tileX / 16 + tileY * 2].resize(16, 0);
			for (int x = 0; x < 16; x++) {
				for (int y = 15; y >= 0; y--) {
					Uint32 color = Animation::getPixel(DataFile, tileX + x, tileY + y);
					if (color & cmask) {
						assert((color & gmask) / dmask >= 0);
						angles[tileX / 16 + tileY * 2] = (color & gmask) / dmask;
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

void DataReader::LoadBackground(std::string path, std::vector < std::vector < Animation > >& background, int numTiles, SDL_Window* window) {
	typedef ::Animation::effectType effectType;
	typedef ::Animation::effectData effectData;
	
	SDL_Surface* current;
	background.clear();
	background.resize(8);
	for (int tile = 0; tile < numTiles; tile++) {
		std::string currentPath = path + std::to_string(tile + 1) + '_';
		for (int layer = 0; layer < 8; layer++) {
			std::string fullCurrentPath = currentPath + std::to_string(layer) + ".png";
			current = IMG_Load(fullCurrentPath.c_str());
			background[layer].push_back(Animation(current, 12.0 * 1000.0 / 60.0, 1, window));
			if (layer == 0) {
				std::vector < std::vector < effectType > > types(4, std::vector<effectType>(1, effectType::PALETTE_SWAP));
				std::vector < std::vector < effectData > > data(4, std::vector<effectData>(1));
				std::vector < Uint32 > oldColors{ 0x006080a0, 0x006080e0, 0x0080a0e0, 0x00a0c0e0 };
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
				for (int i = 0; i < 4; i++) {
					data[i][0].swp.oldColors = oldColors;
					data[i][0].swp.newColors = newColors[i];
				}
				background[0][tile].addStaticEffects(types, data);
			}
			std::cout << "\tLayer " << layer << " done\n";
		}
		std::cout << "Tile " << tile << " done\n";
	}
}