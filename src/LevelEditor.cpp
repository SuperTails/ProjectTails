#include "stdafx.h"
#include "Ground.h"
#include "Camera.h"
#include "LevelEditor.h"
#include "Functions.h"
#include "InputComponent.h"
#include "Version.h"
#include "Drawing.h"
#include <experimental/filesystem>
#include <functional>
#include <limits>
#include <cassert>

bool LevelEditor::levelEditing(false);

/*LevelEditor::LevelEditor(const std::vector< Ground >& levelGround, SDL_Point levelSize, Camera* camera, const std::vector< PhysStruct >& entities) : LevelEditor(organizeBlocks(levelGround, levelSize), camera, entities) {
}*/

LevelEditor::LevelEditor(Act act, Camera* camera) : 
	currentEntity(act.getEntities().end()),
	cam{ camera },
	sky{ ASSET"Sky.png" } {

	auto& levelBlocks = act.getGround();

	for (std::size_t x = 0; x < levelBlocks.size(); ++x) {
		for (std::size_t y = 0; y < levelBlocks[x].size(); ++y) {
			levelBlocks[x][y].setPosition(SDL_Point{ static_cast< int >(x), static_cast< int >(y) } * GROUND_PIXEL_WIDTH);
		}
	}

	for (const auto& i : entity_property_data::entityTypes) {
		if (!i.second.animationTypes.empty()) {
			entityView.emplace(i.first, Animation{ i.second.animationTypes.front() });
		}
		else if(i.first == "PATHSWITCH") {
			using namespace std::chrono_literals;
			entityView.emplace(i.first, Animation{ { ASSET"Pathswitch.png", 10ms, 1 } });
		}
		else {
			throw "No image file available for entity";
		}
	}
}

void LevelEditor::render() {
	renderTiles();
	renderEntities();
	renderText();
}

void LevelEditor::renderTiles() {
	auto& levelBlocks = level.getGround();

	sky.render({ 0, 0, WINDOW_HORIZONTAL_SIZE, WINDOW_VERTICAL_SIZE });
	for (const auto& column : levelBlocks) {
		for (const auto& block : column) {
			if (block.getIndex() != Ground::NO_TILE) {
				block.Render(*cam);
			}
		}
	}

	double width = levelBlocks.size() * GROUND_PIXEL_WIDTH;
	double height = (levelBlocks.size() == 0) ? 0 : levelBlocks[0].size() * GROUND_PIXEL_WIDTH;
	Rect dest{ 0, 0, width, height };
	drawing::drawRect(globalObjects::renderer, *cam, dest, drawing::Color{ 255, 0, 0 }, false);
}

void LevelEditor::renderEntities() const {
	const auto& levelEntities = level.getEntities();

	SDL_SetRenderDrawColor(globalObjects::renderer, 255, 255, 0, SDL_ALPHA_OPAQUE);
	for (const std::unique_ptr< PhysicsEntity >& i : levelEntities) {
		Point temp = i->position - cam->getPosition();
		entityView.find(i->getKey())->second.Render(static_cast< SDL_Point >(temp), 0, NULL, cam->scale);
		const auto requiredFlags = entity_property_data::requiredFlagCount(entity_property_data::getEntityTypeData(i->getKey()).behaviorKey);
		if (i->getFlags().size() != requiredFlags) {
			Rect dest{ i->getPosition().x, i->getPosition().y, 16, 16 };

			drawing::drawRect(globalObjects::renderer, *cam, dest, drawing::Color{ 255, 255, 0 }, false);
		}
	}
	if (currentEntity != levelEntities.end()) {
		Point pos = (*currentEntity)->getPosition();
		Rect dest{ pos.x - 1, pos.y - 1, 18, 18 };
		drawing::drawRect(globalObjects::renderer, *cam, dest, drawing::Color{ 255, 0, 0 }, false);
	}
}

void LevelEditor::renderText() const {
	const auto& levelEntities = level.getEntities();
	const auto& levelBlocks = level.getGround();

	using std::to_string;

	std::string helpText = "Mode: " + to_string(mode) + " Size: " + to_string(levelBlocks.size()) + " " + to_string(levelBlocks[0].size());
	text::renderAbsolute(SDL_Point{ 1, 1 }, "GUI", helpText);

	if (mode == ENTITY && currentEntity != levelEntities.end()) {
		const auto typeId = (*currentEntity)->getKey();
		const auto requiredFlags = entity_property_data::requiredFlagCount(entity_property_data::getEntityTypeData(typeId).behaviorKey);
		const auto currentString = [&]() -> std::string {
			std::stringstream str;
			std::copy((*currentEntity)->getFlags().begin(), (*currentEntity)->getFlags().end(), std::ostream_iterator< char >(str, " "));
			return str.str();
		}();

		text::renderAbsolute(SDL_Point{ 1, 15 }, "GUI", std::to_string(requiredFlags) + "\nCURRENT: " + currentString);
	}
	std::string controlsText;
	if (mode == editMode::TILE) {
		controlsText = "M: Change mode   Click and drag: Change tile   LBracket: Zoom out   RBracket: Zoom in";
	}
	else {
		controlsText = "M: Change mode   Click: Select entity";
	}
	text::renderAbsolute(SDL_Point{ 1, 30 }, "GUI", controlsText);
}

bool LevelEditor::handleInput() {
	const auto& input = globalObjects::input;
	double thisDistance = (Timer::getFrameTime().count() / (1000.0 / 60.0)) * 10 / cam->scale;
	cam->position.x += thisDistance * (globalObjects::input.getKeyState('a') - globalObjects::input.getKeyState('d'));
	cam->position.y += thisDistance * (globalObjects::input.getKeyState('s') - globalObjects::input.getKeyState('w'));

	if (mode == TILE) {
		double newScale = cam->scale;
		newScale += 0.1 * (input.getKeyPress(']') - input.getKeyPress('['));
		cam->scale = std::clamp(newScale, 0.1, 3.0);
	}

	if (input.getKeyPress('m')) {
		mode = (mode == TILE) ? ENTITY : TILE;
	}

	auto& levelEntities = level.getEntities();

	if (mode == ENTITY && currentEntity != levelEntities.end()) {
		mouseWheelValue += input.getWheel() * 1.5;
		mouseWheelValue = std::max(1.0, mouseWheelValue);
		mouseWheelValue = std::min(10.0, mouseWheelValue);

		double thisMove = thisDistance / (mouseWheelValue * cam->scale);

		(*currentEntity)->position.x += thisMove * (input.getKeyState(SDLK_RIGHT) - input.getKeyState(SDLK_LEFT));
		(*currentEntity)->position.y += thisMove * (input.getKeyState(SDLK_UP) - input.getKeyState(SDLK_DOWN));
	}

	//Mouse coordinates are stored as absolute coordinates
	SDL_Point mouse;
	Uint32 mouseState = SDL_GetMouseState(&mouse.x, &mouse.y);
	mouse = (mouse / cam->scale) + static_cast< SDL_Point >(cam->getPosition());

	const auto mouseTile = mouse / GROUND_PIXEL_WIDTH;
	static bool mouseDebounce = false;

	auto& levelBlocks = level.getGround();

	const auto inBounds = 0 <= mouseTile.x && mouseTile.x < levelBlocks.size() && 0 <= mouseTile.y && mouseTile.y < levelBlocks[0].size();

	const bool lPress = (mouseState & SDL_BUTTON(SDL_BUTTON_LEFT));
	const bool mPress = (mouseState & SDL_BUTTON(SDL_BUTTON_MIDDLE));
	const bool rPress = (mouseState & SDL_BUTTON(SDL_BUTTON_RIGHT));
	const bool lClick = !mouseDebounce && lPress;
	const bool mClick = !mouseDebounce && mPress;
	const bool rClick = !mouseDebounce && rPress;
	mouseDebounce = lPress || mPress || rPress;

	if (inBounds) {
		if (mode == LevelEditor::TILE) {
			if (mClick || rClick) {
				Ground& clickedOn = levelBlocks[mouseTile.x][mouseTile.y];
				clickedOn.setFlip(!clickedOn.getFlip());
			}
			else {
				if (lPress && !mouseDrag) {
					mouseDrag.emplace(mouse, levelBlocks[mouseTile.x][mouseTile.y].getIndex());
				}
				else if (!lPress) {
					mouseDrag.reset();
				}

				SDL_Point mouseStartTile = mouseDrag ? (mouseDrag->first / GROUND_PIXEL_WIDTH) : mouseTile;

				Ground& clickedOn = levelBlocks[mouseStartTile.x][mouseStartTile.y];
				const std::size_t currentIndex = clickedOn.getIndex();

				if (mouseDrag) {
					const int mouseDist = (mouse.y - mouseDrag->first.y) / 20 * cam->scale;
					clickedOn.setIndex(addGroundIndex(mouseDrag->second, mouseDist));
				}


				/*if (lClick) {
					if (currentIndex == (Ground::tileCount() - 1)) {
						clickedOn.setIndex(Ground::NO_TILE);
					}
					else {
						clickedOn.setIndex(currentIndex + 1);
					}
				}
				else {
					if (currentIndex == Ground::NO_TILE) {
						clickedOn.setIndex(Ground::tileCount() - 1);
					}
					else {
						clickedOn.setIndex(currentIndex - 1);
					}
				}*/
			}
		}
		else if (lClick || rClick) {
			const SDL_Rect mousePos{ mouse.x, mouse.y, 16, 16 };
			currentEntity = std::find_if(levelEntities.begin(), levelEntities.end(), [&mousePos](const auto& entity) {
				SDL_Rect temp{ int(entity->position.x), int(entity->position.y), 16, 16 };
				return SDL_HasIntersection(&temp, &mousePos);
			});
		}
	}

	if (mode == ENTITY) {
		if (globalObjects::input.getKeyPress('n')) {
			levelEntities.push_back(std::make_unique< PhysicsEntity >("RING", std::vector< char >{}, Point(mouse)));
			currentEntity = std::prev(levelEntities.end());
		}
		else if(globalObjects::input.getKeyPress('x') && currentEntity != levelEntities.end()) {
			levelEntities.erase(currentEntity);
			currentEntity = levelEntities.end();
		}
		if (currentEntity != levelEntities.end()) {
			const auto& entityTypes = entity_property_data::entityTypes;
			const auto entityType = entityTypes.find((*currentEntity)->getKey());
			assert(entityType != entityTypes.end());
			if (globalObjects::input.getKeyPress('[')) {
				/*if (entityType == entityTypes.begin()) {
					currentEntity->typeId = std::prev(entityTypes.end())->first;
				}
				else {
					currentEntity->typeId = std::prev(entityType)->first;
				}*/
			}
			else if (globalObjects::input.getKeyPress(']')) {
				if (std::next(entityType) == entityTypes.end()) {
					*currentEntity = std::make_unique< PhysicsEntity >(
						entityTypes.begin()->first,
						(*currentEntity)->getFlags(),
						(*currentEntity)->position
					);
				}
				else {
					*currentEntity = std::make_unique< PhysicsEntity >(
						std::next(entityType)->first,
						(*currentEntity)->getFlags(),
						(*currentEntity)->position
					);
				}
			}
			
		}
		if (globalObjects::input.getKeyPress('f')) {
			std::cout << "Enter flags: ";
			std::string current;
			std::getline(std::cin, current);
			std::cout << '\n';
			(*currentEntity)->setFlags(parseFlags(current));
			std::cout << '\n';
		}
	}
	else if (globalObjects::input.getKeyPress('r')) {
		std::cout << "Enter new size: ";
		SDL_Point newSize;
		std::cin >> newSize.x >> newSize.y;
		newSize.x = (newSize.x == -1) ? levelBlocks.size() : newSize.x;
		newSize.y = (newSize.y == -1) ? levelBlocks[0].size() : newSize.y;

		if (newSize.x > levelBlocks.size()) {
			std::size_t previousSize = levelBlocks.size();
			levelBlocks.resize(newSize.x);
			for (int x = previousSize; x < newSize.x; ++x) {
				levelBlocks[x].resize(newSize.y);
				for (int y = 0; y < newSize.y; ++y) {
					levelBlocks[x][y] = Ground{ Ground::NO_TILE, { x, y }, false };
				}
			}
		}
		else {
			levelBlocks.resize(newSize.x);
		}

		if (newSize.y > levelBlocks[0].size()) {
			const std::size_t previousSize = levelBlocks[0].size();
			for (int x = 0; x < levelBlocks.size(); ++x) {
				levelBlocks[x].resize(newSize.y);
				for (int y = previousSize; y < levelBlocks[x].size(); ++y) {
					levelBlocks[x][y] = Ground{ Ground::NO_TILE, SDL_Point{ x, y }, false };
				}
			}
		}
		else {
			for (auto& g : levelBlocks) {
				g.resize(newSize.y);
			}
		}
	}

	return globalObjects::input.getKeyState('j') || globalObjects::input.getKeyState('x');
}

void LevelEditor::save(const std::experimental::filesystem::path& path) const {
	const std::string savedLevel = convertToString(level.getNumber(), level.getName(), level.getBlockPrefix(), level.getBackgroundFolder());

	if (std::experimental::filesystem::exists(path)) {
		std::experimental::filesystem::rename(path, path.native() + ".bak");
	}

	std::ofstream file(path);
	file << savedLevel;
}

std::string LevelEditor::convertToString(int levelNumber, const std::string& levelName, const std::string& blockPrefix, const std::string& backgroundFolder) const {
	std::stringstream current;

	current << currentVersion << '\n';

	current << levelNumber << ' ' << levelName << '\n';

	auto& levelEntities = level.getEntities();
	
	for (const std::unique_ptr< PhysicsEntity >& i : levelEntities) {
		current << i->position.x << ' ';
		current << i->position.y << ' ';
		current << i->getKey();
		for (char c : i->getFlags()) {
			current << ' ' << c;
		}
		current << '\n';
	}
	current << "E\n";
	current << "NORMAL\n";

	current << Ground::getMapPath() << '\n';
	current << blockPrefix << '\n';
	current << backgroundFolder << '\n';

	auto& levelBlocks = level.getGround();

	std::vector< Ground > groundData;
	for (const auto& column : levelBlocks) {
		for (const auto& block : column) {
			if (block.getIndex() != Ground::NO_TILE) {
				groundData.push_back(block);
			}
		}
	}

	current << groundData.size() << '\n';
	current << levelBlocks.size() << ' ' << levelBlocks[0].size() << '\n';
	for (const Ground& i : groundData) {
		current << i << '\n';
	}
	return current.str();
}

std::size_t LevelEditor::addGroundIndex(std::size_t current, int addend) {
	/*addend %= static_cast< int >(Ground::tileCount() + 1);
	if (addend > 0) {
		if (current + addend == Ground::tileCount()) {
			return Ground::NO_TILE;
		}
		else {
			return current + addend;
		}
	}
	else if (addend < 0) {
		if (-addend <= current + 1) {
			return current + addend;
		}
		else {
			return Ground::tileCount() + (addend + current + 1);
		}
	}
	else {
		return current;
	}*/
	int signedIndex = (current == Ground::NO_TILE ? -1 : static_cast< int >(current));
	++signedIndex;
	signedIndex += addend;
	signedIndex %= static_cast< int >(Ground::tileCount() + 1);
	while (signedIndex < 0) {
		signedIndex += (Ground::tileCount() + 1);
	}
	return static_cast< std::size_t >(signedIndex - 1);
}


std::string to_string(LevelEditor::editMode m) {
	using namespace std::string_literals;
	static const std::array< std::string, 2 > names = { "TILE"s, "ENTITY"s };
	
	assert(static_cast< int >(m) < names.size());

	return names[static_cast< int >(m)];
}

std::vector< std::vector< Ground > > organizeBlocks(const std::vector< Ground >& blocks, SDL_Point levelSize) {
	std::vector< std::vector< Ground > > levelBlocks;
	for (const Ground& g : blocks) {
		const SDL_Point position = g.getPosition();
		levelBlocks[position.x / GROUND_PIXEL_WIDTH][position.y / GROUND_PIXEL_WIDTH] = g;
	}

	for (std::size_t x = 0; x < levelBlocks.size(); ++x) {
		for (std::size_t y = 0; y < levelBlocks[x].size(); ++y) {
			levelBlocks[x][y].setPosition(SDL_Point{ int(x), int(y) } * GROUND_PIXEL_WIDTH);
		}
	}
	return levelBlocks;
}

std::vector< char > LevelEditor::parseFlags(const std::string& current) const {
	std::vector< char > newFlags;
	std::optional< int > currentNum;
	for (char c : current) {
		if (c == '\\') {
			currentNum = 0;
			continue;
		}
		else if (currentNum) {
			if (isspace(c)) {
				std::cout << *currentNum << ", ";
				newFlags.push_back(char(*currentNum));
				currentNum.reset();
				continue;
			}
			else {
				*currentNum *= 10;
				*currentNum += int{ c - '0' };
			}
		}
		else if (c != ' ' && c != '\n') {
			std::cout << c << ", ";
			newFlags.push_back(c);
		}
	}
	return newFlags;
}
