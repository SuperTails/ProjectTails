#include "stdafx.h"
#include "LevelEditor.h"
#include "Functions.h"
#include "InputComponent.h"
#include <functional>

bool LevelEditor::levelEditing(false);

std::string LevelEditor::levelPath("");

std::vector < std::vector < Ground > > LevelEditor::levelBlocks(0);

std::vector < PhysStruct > LevelEditor::levelEntities;

std::unordered_map < std::string, Animation > LevelEditor::entityView;

std::vector < PhysStruct >::iterator LevelEditor::currentEntity = LevelEditor::levelEntities.end();

bool LevelEditor::mouseDebounce(false);

LevelEditor::editMode LevelEditor::mode(TILE);

Camera* LevelEditor::cam;

SDL_Surface* LevelEditor::Sky;

SDL_Texture* LevelEditor::SkyTexture;

double LevelEditor::mouseWheelValue = 0;

void LevelEditor::init(std::vector < Ground >& levelGround, SDL_Point levelSize) {
	levelBlocks.resize(levelSize.x, std::vector < Ground >(levelSize.y, Ground()));
	Sky = IMG_Load(ASSET"Sky.png");
	SkyTexture = SDL_CreateTextureFromSurface(globalObjects::renderer, Sky);

	for (Ground& g : levelGround) {
		const SDL_Point position = g.getPosition();
		levelBlocks[position.x][position.y] = g;
	}

	for (std::size_t x = 0; x < levelBlocks.size(); ++x) {
		for (std::size_t y = 0; y < levelBlocks[x].size(); ++y) {
			levelBlocks[x][y].setPosition(SDL_Point{ int(x), int(y) });
		}
	}

	auto i = entity_property_data::entityTypes.begin();
	while (i != entity_property_data::entityTypes.end()) {
		if (!i->second.animationTypes.empty()) {
			Animation current(i->second.animationTypes.front());
			entityView.emplace(i->first, current);
		}
		else if(i->first == "PATHSWITCH") {
			using namespace std::chrono_literals;
			AnimStruct temp{ ASSET"Pathswitch.png", 10ms, 1 };
			Animation current(temp);
			entityView.emplace(i->first, current);
		}
		else {
			throw "No image file available for entity";
		}
		++i;
	}

	SDL_SetRenderDrawColor(globalObjects::renderer, 255, 0, 0, SDL_ALPHA_OPAQUE);

	currentEntity = levelEntities.end();
}

void LevelEditor::renderTiles() {
	SDL_Rect src{ 0, 0, 1, 1 }, dst{ 0, 0, WINDOW_HORIZONTAL_SIZE, WINDOW_VERTICAL_SIZE };
	SDL_RenderCopyEx(globalObjects::renderer, SkyTexture, &src, &dst, 0, NULL, SDL_FLIP_NONE);
	for (int column = 0; column < levelBlocks.size(); ++column) {
		for (int row = 0; row < levelBlocks[column].size(); ++row) {
			std::size_t tile = levelBlocks[column][row].getIndex();
			if (tile != static_cast<std::size_t>(-1)) {
				levelBlocks[column][row].Render(cam->getPosition(), 1.0 / globalObjects::ratio);
			}
		}
	}
	SDL_Rect boundingRect = PhysicsEntity::getRelativePos(SDL_Rect{ 0, 0, int(levelBlocks.size() * GROUND_PIXEL_WIDTH), int(levelBlocks[0].size() * GROUND_PIXEL_WIDTH) }, cam->getPosition());
	boundingRect *= globalObjects::ratio;
	SDL_RenderDrawRect(globalObjects::renderer, &boundingRect);
}

void LevelEditor::renderEntities() {
	static Text flagsText(constants::FONT_PATH);
	for (PhysStruct& i : levelEntities) {
		SDL_Rect dst{ i.position.x, i.position.y, 16, 16 };
		dst = PhysicsEntity::getRelativePos(dst, cam->getPosition());
		entityView.find(i.typeId)->second.Render(getXY(dst), 0, NULL, globalObjects::ratio);
		auto requiredFlags = entity_property_data::requiredFlagCount(entity_property_data::getEntityTypeData(i.typeId).behaviorKey);
		if (i.flags.size() != requiredFlags) {
			SDL_RenderFillRect(globalObjects::renderer, &dst);
		}
	}
	if (currentEntity != levelEntities.end()) {
		SDL_Rect dst(PhysicsEntity::getRelativePos(currentEntity->position, cam->getPosition()));
		dst *= globalObjects::ratio;
		SDL_RenderDrawRect(globalObjects::renderer, &dst);
	}
}

void LevelEditor::renderText() {
	static Text t(ASSET"FontGUI.png");
	t.StringToText("Mode: " + mtos(mode) + " Size: " + std::to_string(levelBlocks.size()) + " " + std::to_string(levelBlocks[0].size()));
	t.Render(SDL_Point { 25, 25 });
}

bool LevelEditor::handleInput() {
	double thisDistance = (Timer::getFrameTime().count() / (1000.0 / 60.0)) * 5;

	static double cameraXError = 0;
	static double cameraYError = 0;
	
	if (globalObjects::input.GetKeyState(InputComponent::A)) {
		cameraXError -= thisDistance;
	}
	else if (globalObjects::input.GetKeyState(InputComponent::D)) {
		cameraXError += thisDistance;
	}
	if (globalObjects::input.GetKeyState(InputComponent::W)) {
		cameraYError -= thisDistance;
	}
	else if (globalObjects::input.GetKeyState(InputComponent::S)) {
		cameraYError += thisDistance;
	}

	cam->updatePosition({ static_cast<int>(cameraXError), static_cast<int>(cameraYError), 0, 0, 0 }, PRHS_UPDATE_RELATIVE);

	cameraXError -= static_cast<int>(cameraXError);
	cameraYError -= static_cast<int>(cameraYError);

	if (globalObjects::input.GetKeyPress(InputComponent::M)) {
		mode = (mode == TILE) ? ENTITY : TILE;
	}

	if (mode == ENTITY && currentEntity != levelEntities.end()) {
		mouseWheelValue += globalObjects::input.GetWheel() * 1.5;
		mouseWheelValue = std::max(1.0, mouseWheelValue);
		mouseWheelValue = std::min(10.0, mouseWheelValue);
		double thisMove = thisDistance / mouseWheelValue;
		if (thisMove < 1)
			thisMove = (SDL_GetTicks() % int(1.0 / thisMove)) == 0;
		if (globalObjects::input.GetKeyState(InputComponent::LARROW)) {
			currentEntity->position.x -= thisMove;
		}
		else if (globalObjects::input.GetKeyState(InputComponent::RARROW)) {
			currentEntity->position.x += thisMove;
		}
		if (globalObjects::input.GetKeyState(InputComponent::UARROW)) {
			currentEntity->position.y -= thisMove;
		}
		else if (globalObjects::input.GetKeyState(InputComponent::DARROW)) {
			currentEntity->position.y += thisMove;
		}
	}

	//Mouse coordinates are stored as absolute coordinates, but every unit is 1 SCREEN pixel
	int mouseX, mouseY;

	Uint32 mouseState = SDL_GetMouseState(&mouseX, &mouseY);

	mouseX += cam->getPosition().x * globalObjects::ratio;
	mouseY += cam->getPosition().y * globalObjects::ratio;

	int mouseTileX = mouseX / (GROUND_PIXEL_WIDTH * globalObjects::ratio);
	int mouseTileY = mouseY / (GROUND_PIXEL_WIDTH * globalObjects::ratio);

	if (mouseState & (SDL_BUTTON(SDL_BUTTON_LEFT) | SDL_BUTTON(SDL_BUTTON_RIGHT))) {
		if (!mouseDebounce) {
			if (mode == TILE) {
				if (mouseTileX < levelBlocks.size() && mouseTileY < levelBlocks[0].size()) {
					Ground& clickedOn = levelBlocks[(mouseX / (GROUND_PIXEL_WIDTH * globalObjects::ratio))][(mouseY / (GROUND_PIXEL_WIDTH * globalObjects::ratio))];
					std::size_t currentIndex = clickedOn.getIndex();
					if (mouseState & SDL_BUTTON(SDL_BUTTON_LEFT)) {
						if (currentIndex == (Ground::tileCount() - 1)) {
							clickedOn.setIndex(static_cast<std::size_t>(-1));
						}
						else {
							clickedOn.setIndex(currentIndex + 1);
						}
					}
					else {
						if (currentIndex == static_cast<std::size_t>(-1)) {
							clickedOn.setIndex(Ground::tileCount() - 1);
						}
						else {
							clickedOn.setIndex(currentIndex - 1);
						}
					}
				}
				mouseDebounce = true;
			}
			else {
				currentEntity = levelEntities.begin();
				SDL_Rect mousePos{ mouseX, mouseY, 16, 16 };
				while (currentEntity != levelEntities.end()) {
					SDL_Rect relativePos(currentEntity->position);

					//Divide positions by 2 because we are at 1/2 scale to get screen coordinates
					relativePos.x *= globalObjects::ratio;
					relativePos.y *= globalObjects::ratio;

					if (SDL_HasIntersection(&relativePos, &mousePos)) {
						break;
					}
					++currentEntity;
				}
				mouseDebounce = true;
			}
		}
	}
	else if (mouseState & SDL_BUTTON(SDL_BUTTON_MIDDLE)) {
		if (!mouseDebounce && mouseTileX < levelBlocks.size() && mouseTileY < levelBlocks[0].size()) {
			Ground& clickedOn = levelBlocks[mouseTileX][mouseTileY];

			bool newFlip = clickedOn.getFlip() ^ 0x1;
			clickedOn.setFlip(newFlip);
		}
		mouseDebounce = true;
	}
	else {
		mouseDebounce = false;
	}

	if (mode == ENTITY) {
		if (globalObjects::input.GetKeyPress(InputComponent::N)) {
			SDL_Rect newPosition{ static_cast<int>(mouseX / globalObjects::ratio), static_cast<int>(mouseY / globalObjects::ratio), 16, 16 };
			PhysStruct newEntity{ "RING", {}, newPosition, false };
			levelEntities.push_back(std::move(newEntity));
			currentEntity = std::prev(levelEntities.end());
		}
		else if(globalObjects::input.GetKeyPress(InputComponent::X) && currentEntity != levelEntities.end()) {
			levelEntities.erase(currentEntity);
			currentEntity = levelEntities.end();
		}
		if (currentEntity != levelEntities.end()) {
			using namespace entity_property_data;
			const auto entityType = std::find_if(entityTypes.begin(), entityTypes.end(), [&](const auto& a){ return currentEntity->typeId == a.first; });
			if (globalObjects::input.GetKeyPress(InputComponent::LBRACKET)) {
				if (entityType == entityTypes.begin()) {
					currentEntity->typeId = std::prev(entityTypes.end())->first;
				}
				else {
					currentEntity->typeId = std::next(entityType)->first;
				}
			}
			else if (globalObjects::input.GetKeyPress(InputComponent::RBRACKET)) {
				if (entityType == entityTypes.end()) {
					currentEntity->typeId = entityTypes.begin()->first;
				}
				else {
					currentEntity->typeId = std::next(entityType)->first;
				}
			}
			
		}
		if (globalObjects::input.GetKeyPress(InputComponent::F)) {
			std::cout << "Enter flags: ";
			std::vector < char >& currentFlags = currentEntity->flags;
			currentFlags.clear();
			std::string current;
			std::getline(std::cin, current);
			int currentNum = -1;
			for (char& c : current) {
				if (c == '\\') {
					currentNum = 0;
					continue;
				}
				if (currentNum != -1) {
					if (c == ' ' || c == '\n') {
						currentFlags.push_back(char(currentNum));
						currentNum = -1;
						continue;
					}
					currentNum *= 10;
					currentNum += int(c - '0');
				}
				else if (c != ' ' && c != '\n')
					currentFlags.push_back(c);
			}
		}
	}
	else {
		if (globalObjects::input.GetKeyPress(InputComponent::R)) {
			SDL_Point newSize;
			std::cout << "Enter new size: ";
			std::cin >> newSize.x >> newSize.y;
			newSize.x = (newSize.x == -1) ? levelBlocks.size() : newSize.x;
			newSize.y = (newSize.y == -1) ? levelBlocks[0].size() : newSize.y;

			if (newSize.x > levelBlocks.size()) {
				std::size_t previousSize = levelBlocks.size();
				levelBlocks.resize(newSize.x);
				std::generate(levelBlocks.begin() + previousSize, levelBlocks.end(),
						[&newSize, x = (int)previousSize]() mutable {
							std::vector < Ground > g(newSize.y);
							std::generate(g.begin(), g.end(),
									[&newSize, &x, y = 0]() mutable {
										return Ground(-1, { x, y++ }, false);
									});
							++x;
							return g;
						});
			}
			else {
				levelBlocks.resize(newSize.x);
			}

			if (newSize.y > levelBlocks[0].size()) {
				std::size_t previousSize = levelBlocks[0].size();
				int x = 0;
				for (auto& g : levelBlocks) {
					g.resize(newSize.y);
					std::generate(g.begin() + previousSize, g.end(),
							[&newSize, &x, y = (int)previousSize]() mutable {
								return Ground { -1u, SDL_Point { x, y++ }, false };
							});
					++x;
				}
			}
			else {
				for (auto& g : levelBlocks) {
					g.resize(newSize.y);
				}
			}
		}
	}

	return globalObjects::input.GetKeyState(InputComponent::J) || globalObjects::input.GetKeyState(InputComponent::X);
}

std::string LevelEditor::convertToString(int levelNumber, std::string levelName) {
	std::string current(std::to_string(levelNumber) + levelName + "\n");
	for (PhysStruct& i : levelEntities) {
		current += std::to_string(i.position.x) + " ";
		current += std::to_string(i.position.y) + " ";
		current += i.typeId;
		for (char& c : i.flags) {
			current += " " + std::string(1, c);
		}
		current += '\n';
	}
	current += "E\n";
	current += "1000000 -500 100 3000\n"; //Figure out win area later
	current += "NORMAL\n";
	current += Ground::getMapPath().substr(std::string(ASSET).length()) + '\n';
	std::vector < Ground > groundData;
	for (int x = 0; x < levelBlocks.size(); ++x) {
		for (int y = 0; y < levelBlocks[x].size(); ++y) {
			if (levelBlocks[x][y].getIndex() != std::size_t(-1)) {
				groundData.push_back(levelBlocks[x][y]);
			}
		}
	}
	current += std::to_string(groundData.size()) + '\n';
	current += std::to_string(levelBlocks.size()) + " " + std::to_string(levelBlocks[0].size()) + "\n";
	for (Ground& i : groundData) {
		std::stringstream temp;
		temp << i;
		current += temp.str() + '\n';
	}
	current.pop_back(); //Remove trailing newline
	return current;
}

std::string LevelEditor::mtos(LevelEditor::editMode m) {
	switch (m) {
	case TILE:
		return std::string("TILE");
	case ENTITY:
		return std::string("ENTITY");
	default:
		return std::string("");
	}
}
