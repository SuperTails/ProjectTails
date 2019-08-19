#include "stdafx.h"
#include "Ground.h"
#include "Camera.h"
#include "LevelEditor.h"
#include "Functions.h"
#include "InputComponent.h"
#include "Version.h"
#include <experimental/filesystem>
#include <functional>
#include <limits>
#include <cassert>

bool LevelEditor::levelEditing(false);

/*LevelEditor::LevelEditor(const std::vector< Ground >& levelGround, SDL_Point levelSize, Camera* camera, const std::vector< PhysStruct >& entities) : LevelEditor(organizeBlocks(levelGround, levelSize), camera, entities) {
}*/

LevelEditor::LevelEditor(Act act, Camera* camera) : 
	level((act.unloadAllEntities(), act)),
	currentEntity{ level.getEntities().end() },
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

	SDL_SetRenderDrawColor(globalObjects::renderer, 255, 0, 0, SDL_ALPHA_OPAQUE);
}

void LevelEditor::render() {
	renderTiles();
	renderEntities();
	renderText();
}

void LevelEditor::renderTiles() {
	auto& levelBlocks = level.getGround();

	const SDL_Rect src{ 0, 0, 1, 1 }, dst{ 0, 0, WINDOW_HORIZONTAL_SIZE, WINDOW_VERTICAL_SIZE };
	sky.render({ 0, 0, WINDOW_HORIZONTAL_SIZE, WINDOW_VERTICAL_SIZE });
	for (const auto& column : levelBlocks) {
		for (const auto& block : column) {
			if (block.getIndex() != Ground::NO_TILE) {
				block.Render(*cam);
			}
		}
	}
	const SDL_Point corner = SDL_Point{ 0, 0 } - cam->getPosition();
	const auto boundingRect = SDL_Rect{ corner.x, corner.y, int(levelBlocks.size() * GROUND_PIXEL_WIDTH), int(levelBlocks[0].size() * GROUND_PIXEL_WIDTH)} * cam->scale;
	SDL_SetRenderDrawColor(globalObjects::renderer, 255, 0, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderDrawRect(globalObjects::renderer, &boundingRect);
}

void LevelEditor::renderEntities() const {
	const auto& levelEntities = level.getEntities();

	SDL_SetRenderDrawColor(globalObjects::renderer, 255, 255, 0, SDL_ALPHA_OPAQUE);
	for (const PhysStruct& i : levelEntities) {
		entityView.find(i.typeId)->second.Render(getXY(i.position) - cam->getPosition(), 0, NULL, cam->scale);
		const auto requiredFlags = entity_property_data::requiredFlagCount(entity_property_data::getEntityTypeData(i.typeId).behaviorKey);
		if (i.flags.size() != requiredFlags) {
			const auto dst = (SDL_Rect{ i.position.x, i.position.y, 16, 16 } - cam->getPosition()) * cam->scale;
			SDL_RenderDrawRect(globalObjects::renderer, &dst);
		}
	}
	SDL_SetRenderDrawColor(globalObjects::renderer, 255, 0, 0, (SDL_ALPHA_OPAQUE + SDL_ALPHA_TRANSPARENT) / 2);
	if (currentEntity != levelEntities.end()) {
		const SDL_Rect dst = (currentEntity->position - cam->getPosition()) * cam->scale;
		SDL_RenderDrawRect(globalObjects::renderer, &dst);
	}
}

void LevelEditor::renderText() const {
	const auto& levelEntities = level.getEntities();
	const auto& levelBlocks = level.getGround();

	using std::to_string;

	static Text t(constants::FONT_PATH);
	static Text flagsText(constants::FONT_PATH);
	t.setText("Mode: " + to_string(mode) + " Size: " + to_string(levelBlocks.size()) + " " + to_string(levelBlocks[0].size()));
	t.Render(SDL_Point { 25, 25 });
	if (mode == ENTITY && currentEntity != levelEntities.end()) {
		const auto typeId = currentEntity->typeId;
		const auto requiredFlags = entity_property_data::requiredFlagCount(entity_property_data::getEntityTypeData(typeId).behaviorKey);
		const auto currentString = [&]() -> std::string {
			std::stringstream str;
			std::copy(currentEntity->flags.begin(), currentEntity->flags.end(), std::ostream_iterator< char >(str, " "));
			return str.str();
		}();
		flagsText.StringToText(std::to_string(requiredFlags) + "\nCURRENT: " + currentString);
		flagsText.Render(SDL_Point{ 25, 28 + t.getText().size().y });
	}
	static Text controls(constants::FONT_PATH);
	if (mode == editMode::TILE) {
		controls.setText("M: Change mode   Click and drag: Change tile   LBracket: Zoom out   RBracket: Zoom in");
	}
	else {
		controls.setText("M: Change mode   Click: Select entity");
	}
	controls.Render(SDL_Point{ 25, 50 });
}

bool LevelEditor::handleInput() {
	static double cameraXError = 0;
	static double cameraYError = 0;
	
	double thisDistance = (Timer::getFrameTime().count() / (1000.0 / 60.0)) * 10 / cam->scale;
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

	if (mode == TILE) {
		if (globalObjects::input.GetKeyPress(InputComponent::LBRACKET) && cam->scale > 0.1) {
			cam->scale -= 0.1;
		}
		if (globalObjects::input.GetKeyPress(InputComponent::RBRACKET) && cam->scale < 3.0) {
			cam->scale += 0.1;
		}
	}

	cam->position += SDL_Point{ static_cast< int >(cameraXError), static_cast< int >(cameraYError) };

	cameraXError -= static_cast< int >(cameraXError);
	cameraYError -= static_cast< int >(cameraYError);

	if (globalObjects::input.GetKeyPress(InputComponent::M)) {
		mode = (mode == TILE) ? ENTITY : TILE;
	}

	auto& levelEntities = level.getEntities();

	if (mode == ENTITY && currentEntity != levelEntities.end()) {
		mouseWheelValue += globalObjects::input.GetWheel() * 1.5;
		mouseWheelValue = std::max(1.0, mouseWheelValue);
		mouseWheelValue = std::min(10.0, mouseWheelValue);

		static double entityXError = 0.0;
		static double entityYError = 0.0;

		double thisMove = thisDistance / (mouseWheelValue * cam->scale);
		if (globalObjects::input.GetKeyState(InputComponent::LARROW)) {
			entityXError -= thisMove;
		}
		else if (globalObjects::input.GetKeyState(InputComponent::RARROW)) {
			entityXError += thisMove;
		}
		if (globalObjects::input.GetKeyState(InputComponent::UARROW)) {
			entityYError -= thisMove;
		}
		else if (globalObjects::input.GetKeyState(InputComponent::DARROW)) {
			entityYError += thisMove;
		}

		currentEntity->position.x += static_cast< int >(entityXError);
		currentEntity->position.y += static_cast< int >(entityYError);

		entityXError -= static_cast< int >(entityXError);
		entityYError -= static_cast< int >(entityYError);
	}

	//Mouse coordinates are stored as absolute coordinates
	SDL_Point mouse;
	Uint32 mouseState = SDL_GetMouseState(&mouse.x, &mouse.y);
	mouse = (mouse / cam->scale) + cam->getPosition();

	

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
				return SDL_HasIntersection(&entity.position, &mousePos);
			});
		}
	}

	if (mode == ENTITY) {
		if (globalObjects::input.GetKeyPress(InputComponent::N)) {
			SDL_Rect newPosition{ mouse.x, mouse.y, 16, 16 };
			levelEntities.push_back(PhysStruct{ "RING", {}, newPosition, false });
			currentEntity = std::prev(levelEntities.end());
		}
		else if(globalObjects::input.GetKeyPress(InputComponent::X) && currentEntity != levelEntities.end()) {
			levelEntities.erase(currentEntity);
			currentEntity = levelEntities.end();
		}
		if (currentEntity != levelEntities.end()) {
			const auto& entityTypes = entity_property_data::entityTypes;
			const auto entityType = entityTypes.find(currentEntity->typeId);
			assert(entityType != entityTypes.end());
			if (globalObjects::input.GetKeyPress(InputComponent::LBRACKET)) {
				/*if (entityType == entityTypes.begin()) {
					currentEntity->typeId = std::prev(entityTypes.end())->first;
				}
				else {
					currentEntity->typeId = std::prev(entityType)->first;
				}*/
			}
			else if (globalObjects::input.GetKeyPress(InputComponent::RBRACKET)) {
				if (std::next(entityType) == entityTypes.end()) {
					currentEntity->typeId = entityTypes.begin()->first;
				}
				else {
					currentEntity->typeId = std::next(entityType)->first;
				}
			}
			
		}
		if (globalObjects::input.GetKeyPress(InputComponent::F)) {
			std::cout << "Enter flags: ";
			std::string current;
			std::getline(std::cin, current);
			std::cout << '\n';
			currentEntity->flags = parseFlags(current);
			std::cout << '\n';
		}
	}
	else if (globalObjects::input.GetKeyPress(InputComponent::R)) {
		std::cout << "Enter new size: ";
		SDL_Point newSize;
		std::cin >> newSize.x >> newSize.y;
		newSize.x = (newSize.x == -1) ? levelBlocks.size() : newSize.x;
		newSize.y = (newSize.y == -1) ? levelBlocks[0].size() : newSize.y;

		if (newSize.x > levelBlocks.size()) {
			std::size_t previousSize = levelBlocks.size();
			levelBlocks.resize(newSize.x);
			// Black magic, don't touch
			std::generate(levelBlocks.begin() + previousSize, levelBlocks.end(),
					[&newSize, x = (int)previousSize]() mutable {
						std::vector < Ground > g(newSize.y);
						std::generate(g.begin(), g.end(),
								[&newSize, &x, y = 0]() mutable {
									return Ground{ Ground::NO_TILE, { x, y++ }, false };
								});
						++x;
						return g;
					});
		}
		else {
			levelBlocks.resize(newSize.x);
		}

		if (newSize.y > levelBlocks[0].size()) {
			const std::size_t previousSize = levelBlocks[0].size();
			int x = 0;
			for (auto& g : levelBlocks) {
				g.resize(newSize.y);
				std::generate(g.begin() + previousSize, g.end(),
						[&newSize, &x, y = (int)previousSize]() mutable {
							return Ground{ Ground::NO_TILE, SDL_Point { x, y++ }, false };
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

	return globalObjects::input.GetKeyState(InputComponent::J) || globalObjects::input.GetKeyState(InputComponent::X);
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
	
	for (const PhysStruct& i : levelEntities) {
		current << i.position.x << ' ';
		current << i.position.y << ' ';
		current << i.typeId;
		for (char c : i.flags) {
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
