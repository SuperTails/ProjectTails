#include "stdafx.h"
#include "LevelEditor.h"

const bool LevelEditor::levelEditing(false);

std::string LevelEditor::levelPath("");

std::vector < Ground > LevelEditor::groundList(0);

std::vector < std::vector < LevelEditor::groundData > > LevelEditor::levelBlocks(0);

std::vector < PhysStructInit > LevelEditor::levelEntities;

std::unordered_map < std::string, PhysProp* >* LevelEditor::entityList;

std::unordered_map < std::string, Animation > LevelEditor::entityView;

std::vector < std::string >* LevelEditor::entityTypes;

std::vector < PhysStructInit >::iterator LevelEditor::currentEntity = LevelEditor::levelEntities.end();

bool LevelEditor::mouseDebounce(false);

LevelEditor::editMode LevelEditor::mode(TILE);

Camera* LevelEditor::cam;

SDL_Surface* LevelEditor::Sky;

SDL_Texture* LevelEditor::SkyTexture;

double LevelEditor::mouseWheelValue = 0;

void LevelEditor::init(std::vector < DataReader::groundData >& levelGround, SDL_Point levelSize, std::vector < Ground::groundArrayData >& arrayData) {
	levelBlocks.resize(levelSize.x, std::vector < groundData >(levelSize.y, groundData{ -1, false }));
	Sky = IMG_Load("..\\..\\asset\\Sky.png");
	SkyTexture = SDL_CreateTextureFromSurface(globalObjects::renderer, Sky);

	for (DataReader::groundData& g : levelGround) {
		levelBlocks[g.x][g.y].index = g.index;
		levelBlocks[g.x][g.y].flip = g.flip;
	}

	for (int i = 0; i < arrayData.size(); i++) {
		groundList.emplace_back(SDL_Point{ 0, 0 }, arrayData[i]);
	}

	std::unordered_map <  std::string, PhysProp* >::iterator i = entityList->begin();
	while (i != entityList->end()) {
		if (!i->second->anim.empty()) {
			Animation current(i->second->anim.front());
			entityView.emplace(i->first, current);
		}
		else if(i->second->eType == PATHSWITCH) {
			AnimStruct temp{ "..\\..\\asset\\pathswitch.png", 10, 1 };
			Animation current(temp);
			entityView.emplace(i->first, current);
		}
		else {
			throw "No image file available for entity";
		}
		i++;
	}

	SDL_SetRenderDrawColor(globalObjects::renderer, 255, 0, 0, SDL_ALPHA_OPAQUE);

	currentEntity = levelEntities.end();
}

void LevelEditor::renderTiles() {
	SDL_Rect src{ 0, 0, 1, 1 }, dst{ 0, 0, WINDOW_HORIZONTAL_SIZE, WINDOW_VERTICAL_SIZE };
	SDL_RenderCopyEx(globalObjects::renderer, SkyTexture, &src, &dst, 0, NULL, SDL_FLIP_NONE);
	for (int column = 0; column < levelBlocks.size(); column++) {
		for (int row = 0; row < levelBlocks[column].size(); row++) {
			int tile = levelBlocks[column][row].index;
			if (tile != -1) {
				SDL_Rect pos{ column * int(TILE_WIDTH * GROUND_WIDTH), row * int(TILE_WIDTH * GROUND_WIDTH), int(TILE_WIDTH * GROUND_WIDTH), int(TILE_WIDTH * GROUND_WIDTH) };
				SDL_Rect relativePos = PhysicsEntity::getRelativePos(pos, cam->getPosition());
				SDL_RendererFlip currentFlip = static_cast < SDL_RendererFlip >(levelBlocks[column][row].flip);
				groundList[tile].Render(cam->getPosition(), 1.0 / globalObjects::ratio, &relativePos, 0, currentFlip);
				if (groundList[tile].getMulti()) {
					groundList[tile].Render(cam->getPosition(), 1.0 / globalObjects::ratio, &relativePos, 1, currentFlip);
				}
			}
		}
	}
	SDL_Rect boundingRect = PhysicsEntity::getRelativePos(SDL_Rect{ 0, 0, int(levelBlocks.size() * TILE_WIDTH * GROUND_WIDTH), int(levelBlocks[0].size() * TILE_WIDTH * GROUND_WIDTH) }, cam->getPosition());
	boundingRect.x *= globalObjects::ratio;
	boundingRect.y *= globalObjects::ratio;
	boundingRect.w *= globalObjects::ratio;
	boundingRect.h *= globalObjects::ratio;
	SDL_RenderDrawRect(globalObjects::renderer, &boundingRect);
}

void LevelEditor::renderEntities() {
	for (PhysStructInit& i : levelEntities) {
		SDL_Rect dst{ i.pos.x, i.pos.y, 16, 16 };
		dst = PhysicsEntity::getRelativePos(dst, cam->getPosition());
		entityView.find(i.prop)->second.Render(&dst, 0, NULL, globalObjects::ratio);
	}
	if (currentEntity != levelEntities.end()) {
		SDL_Rect dst(PhysicsEntity::getRelativePos(currentEntity->pos, cam->getPosition()));
		dst.x *= globalObjects::ratio;
		dst.y *= globalObjects::ratio;
		dst.w *= globalObjects::ratio;
		dst.h *= globalObjects::ratio;
		SDL_RenderDrawRect(globalObjects::renderer, &dst);
	}
	//SDL_RenderPresent(globalObjects::renderer);
}

void LevelEditor::renderText() {
	Text t("..\\..\\asset\\FontGUI.png");
	t.StringToText("Mode: " + mtos(mode) + " Size: " + std::to_string(levelBlocks.size()) + " " + std::to_string(levelBlocks[0].size()));
	SDL_Texture* tex = SDL_CreateTextureFromSurface(globalObjects::renderer, t.getText());
	SDL_Rect dst{ 10, 10, t.getText()->w, t.getText()->h };
	SDL_RenderCopyEx(globalObjects::renderer, tex, NULL, &dst, 0.0, NULL, SDL_FLIP_NONE);
	SDL_DestroyTexture(tex);
	//SDL_RenderPresent(globalObjects::renderer);
}

bool LevelEditor::handleInput() {
	int thisDistance = ((globalObjects::time - globalObjects::last_time) / (1000.0 / 60.0)) * 10;
	if (globalObjects::input.GetKeyState(InputComponent::A)) {
		cam->updatePosition({ -1 * thisDistance, 0, 0, 0, 0 }, PRHS_UPDATE_RELATIVE);
	}
	else if (globalObjects::input.GetKeyState(InputComponent::D)) {
		cam->updatePosition({ thisDistance, 0, 0, 0, 0 }, PRHS_UPDATE_RELATIVE);
	}
	if (globalObjects::input.GetKeyState(InputComponent::W)) {
		cam->updatePosition({ 0, -1 * thisDistance, 0, 0, 0 }, PRHS_UPDATE_RELATIVE);
	}
	else if (globalObjects::input.GetKeyState(InputComponent::S)) {
		cam->updatePosition({ 0, thisDistance, 0, 0, 0 }, PRHS_UPDATE_RELATIVE);
	}
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
			currentEntity->pos.x -= thisMove;
		}
		else if (globalObjects::input.GetKeyState(InputComponent::RARROW)) {
			currentEntity->pos.x += thisMove;
		}
		if (globalObjects::input.GetKeyState(InputComponent::UARROW)) {
			currentEntity->pos.y -= thisMove;
		}
		else if (globalObjects::input.GetKeyState(InputComponent::DARROW)) {
			currentEntity->pos.y += thisMove;
		}
	}

	//Mouse coordinates are stored as absolute coordinates, but every unit is 1 SCREEN pixel
	int mouseX, mouseY;

	Uint32 mouseState = SDL_GetMouseState(&mouseX, &mouseY);

	mouseX += cam->getPosition().x * globalObjects::ratio;
	mouseY += cam->getPosition().y * globalObjects::ratio;

	if (mouseState & (SDL_BUTTON(SDL_BUTTON_LEFT) | SDL_BUTTON(SDL_BUTTON_RIGHT))) {
		if (!mouseDebounce) {
			if (mode == TILE) {
				if ((mouseX / (TILE_WIDTH * GROUND_WIDTH)) / globalObjects::ratio < levelBlocks.size() && (mouseY / (TILE_WIDTH * GROUND_WIDTH)) / globalObjects::ratio < levelBlocks[0].size()) {
					int temp = (mouseState & SDL_BUTTON(SDL_BUTTON_LEFT)) ? 2 : groundList.size() + 1;
					int& index = levelBlocks[(mouseX / (TILE_WIDTH * GROUND_WIDTH)) / globalObjects::ratio][(mouseY / (TILE_WIDTH * GROUND_WIDTH)) / globalObjects::ratio].index;
					index = ((index + temp) % (groundList.size() + 1)) - 1;
				}
				mouseDebounce = true;
			}
			else {
				currentEntity = levelEntities.begin();
				SDL_Rect mousePos{ mouseX, mouseY, 16, 16 };
				while (currentEntity != levelEntities.end()) {
					SDL_Rect relativePos(currentEntity->pos);

					//Divide positions by 2 because we are at 1/2 scale to get screen coordinates
					relativePos.x *= globalObjects::ratio;
					relativePos.y *= globalObjects::ratio;

					if (SDL_HasIntersection(&relativePos, &mousePos)) {
						break;
					}
					currentEntity++;
				}
				mouseDebounce = true;
			}
		}
	}
	else if (mouseState & SDL_BUTTON(SDL_BUTTON_MIDDLE)) {
		if (!mouseDebounce && (mouseX / (TILE_WIDTH * GROUND_WIDTH)) / globalObjects::ratio < levelBlocks.size() && (mouseY / (TILE_WIDTH * GROUND_WIDTH)) / globalObjects::ratio < levelBlocks[0].size()) {
			levelBlocks[(mouseX / (TILE_WIDTH * GROUND_WIDTH)) / globalObjects::ratio][(mouseY / (TILE_WIDTH * GROUND_WIDTH)) / globalObjects::ratio].flip ^= 0x1;
		}
		mouseDebounce = true;
	}
	else {
		mouseDebounce = false;
	}

	if (mode == ENTITY) {
		if (globalObjects::input.GetKeyPress(InputComponent::N)) {
			PhysStructInit newEntity{ SDL_Rect{ static_cast < int >(mouseX / globalObjects::ratio), static_cast < int >(mouseY / globalObjects::ratio), 16, 16 }, std::vector < char >(), std::string("RING") };
			levelEntities.push_back(newEntity);
			currentEntity = levelEntities.end() - 1;
		}
		else if(globalObjects::input.GetKeyPress(InputComponent::X) && currentEntity != levelEntities.end()) {
			levelEntities.erase(currentEntity);
			currentEntity = levelEntities.end();
		}
		if ((globalObjects::input.GetKeyPress(InputComponent::LBRACKET) || globalObjects::input.GetKeyPress(InputComponent::RBRACKET)) && currentEntity != levelEntities.end()) {
			std::vector < std::string >::iterator current = entityTypes->begin();
			while (current != entityTypes->end()) {
				if (*current == currentEntity->prop) {
					break;
				}
				++current;
			}
			if (globalObjects::input.GetKeyPress(InputComponent::LBRACKET)) {
				if (current == entityTypes->begin()) {
					current = entityTypes->end() - 1;
				}
				else {
					--current;
				}
				
			}
			else {
				if (current == entityTypes->end() - 1) {
					current = entityTypes->begin();
				}
				else {
					++current;
				}
			}
			currentEntity->prop = *current;
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
			for (std::vector < groundData >& vec : levelBlocks) {
				vec.resize(newSize.y, groundData{ -1, false });
			}
			levelBlocks.resize(newSize.x, std::vector < groundData >(newSize.y, groundData{ -1, false }));
		}
	}

	return globalObjects::input.GetKeyState(InputComponent::J);
}

std::string LevelEditor::convertToString(int levelNumber, std::string levelName) {
	std::string current(std::to_string(levelNumber) + levelName + "\n");
	std::cout << current.max_size() << "\n";
	for (PhysStructInit& i : levelEntities) {
		current += std::to_string(i.pos.x) + " ";
		current += std::to_string(i.pos.y) + " ";
		current += i.prop;
		for (char& c : i.flags) {
			current += " " + std::string(1, c);
		}
		current += '\n';
	}
	current += "E E E\n";
	current += "1000000 -500 100 3000\n"; //Figure out win area later
	current += "NORMAL\n";
	current += Ground::getMapPath() + '\n';
	std::vector < DataReader::groundData > groundData;
	for (int x = 0; x < levelBlocks.size(); x++) {
		for (int y = 0; y < levelBlocks[x].size(); y++) {
			if (levelBlocks[x][y].index != -1) {
				groundData.push_back(DataReader::groundData{ x, y, levelBlocks[x][y].index, levelBlocks[x][y].flip });
			}
		}
	}
	current += std::to_string(groundData.size()) + '\n';
	current += std::to_string(levelBlocks.size()) + " " + std::to_string(levelBlocks[0].size()) + "\n";
	for (DataReader::groundData& i : groundData) {
		current += std::to_string(i.x) + " " + std::to_string(i.y) + " " + std::to_string(i.index);
		if (i.flip)
			current += " 1";
		current += "\n";
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