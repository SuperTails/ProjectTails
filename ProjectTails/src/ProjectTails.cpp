// ProjectTails.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "InputComponent.h"
#include "prhsGameLib.h"
#include "png.h"
#include "Animation.h"
#include "Constants.h"
#include "Act.h"
#include "Player.h"
#include "Camera.h"
#include "Text.h"
#include "Ground.h"
#include "DataReader.h"
#include "Miscellaneous.h"
#include "LevelEditor.h"
#include "SoundHandler.h"
#include "effectManager.h"
#include "CollisionTile.h"
#include "BlockEditor.h"
#include <unordered_map>
#include <fstream>
#include <algorithm>
#include <cstring>
#include <vector>

using namespace std;

const double SCREEN_RATIO = 0.5;
const double SCR_RAT_INV = 1.0 / SCREEN_RATIO;

const double stepLength = 1000.0 / 60.0;

const double LOAD_STEPS = 6.0;

#undef main

int mult(int arg1, double arg2) {
	return (int)((double)arg1 * arg2);
}

std::vector< std::string_view >::const_iterator findArg(const std::vector< std::string_view >& args, const std::string_view& arg) {
	return std::find(args.begin(), args.end(), arg);
}

using namespace std::string_view_literals;

const std::array< std::pair< std::string_view, bool* >, 3 > options{
	make_pair("-level"sv, &LevelEditor::levelEditing),
	//make_pair("-test"sv , &tests::doTests),
	make_pair("-debug"sv, &globalObjects::debug),
	make_pair("-block"sv, &BlockEditor::editing),
};

int main(int argc, char **argv ) { 
	if (!sdlStatus.success) {
		std::cerr << "Initialization failed.\n";
		return 1;
	}

	const std::vector< std::string_view > args(argv + 1, argv + argc);

	if (findArg(args, "-help") != args.end()) {
		std::cout <<
		"Usage: ProjectTails [option]\n"
		"\tOptions:\n"
		"\t\t-level   Run level editor\n"
		"\t\t-debug   Run in debug mode\n"
		"\t\t-block   Run block editor\n";
		return 1;
	}

	for (auto& option : options) {
		*option.second = findArg(args, option.first) != args.end();
	}

	if (globalObjects::debug) {
		if (BlockEditor::editing) {
			std::cerr << "Options '-debug' and '-block' are incompatible!\n";
			return 1;
		}

		Ground::showCollision = true;
	}

	PRHS_Window window("Project Tails", WINDOW_HORIZONTAL_SIZE, WINDOW_VERTICAL_SIZE, false);

	globalObjects::window = window.getWindow();
	globalObjects::renderer = window.getRenderer();

	SDL_SetRenderDrawColor(globalObjects::renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);

	globalObjects::loadProgress.emplace_back("Loading", 0.0);
	globalObjects::updateLoading(0.0);


	const std::array< std::string_view, 2 > actPaths = { constants::ACT1_DATA_PATH, constants::ACT2_DATA_PATH };

	Player Tails;

	Camera cam = Camera({ -32, -8, double(WINDOW_HORIZONTAL_SIZE * SCREEN_RATIO + 64), double(WINDOW_VERTICAL_SIZE * SCREEN_RATIO + 16) });
	cam.setOffset({ (double)(-WINDOW_HORIZONTAL_SIZE * SCREEN_RATIO / 2), (double)(-WINDOW_VERTICAL_SIZE * SCREEN_RATIO + 128) });

	text::addFont("GUI", Surface{ ASSET"FontGUI.png" }, "0123456789:ABCDEFGHIJKLMNOPQRSTUVWXYZ.-,[]", 10);

	//Load entities:
	cout << "Loading entity data...\n";
	
	DataReader::LoadEntityData(constants::ENTITY_DATA_PATH);
	globalObjects::updateLoading(1 / LOAD_STEPS);

	std::vector< std::vector < Animation > > background;

	CollisionTile::loadFromImage(ASSET"SolidGraph.png");
	
	globalObjects::updateLoading(1 / LOAD_STEPS);

	SDL_SetRenderDrawColor(globalObjects::renderer, 0, 0, 0, 0);

	Act currentAct{ ASSET"Act1Data.txt" };

	if (LevelEditor editor(currentAct, &cam); LevelEditor::levelEditing) {
		while (true) {
			Timer::updateFrameTime();
			globalObjects::input.updateKeys();
			if (editor.handleInput())
				break;
			editor.render();
			SDL_RenderPresent(globalObjects::renderer);
		}
		if (globalObjects::input.getKeyState('j')) {
			editor.save(ASSET"Act1Data.txt");
		}
		return 0;
	}

	if (BlockEditor editor; BlockEditor::editing) {
		while (!globalObjects::input.getKeyPress('x')) {
			Timer::updateFrameTime();
			globalObjects::input.updateKeys();

			editor.update(cam);
			editor.render(cam);
			SDL_RenderPresent(globalObjects::renderer);
			SDL_Delay(5);
		}
		return 0;
	}

	globalObjects::updateLoading(1 / LOAD_STEPS);

	Tails.setActType(static_cast< unsigned char > (currentAct.getType()));

	globalObjects::updateLoading(1 / LOAD_STEPS);

	int frames = 0;
	int last_frames = 0;
	Uint32 start = SDL_GetTicks();

	/*Text rings_text = Text(constants::FONT_PATH);
	Text rings_count_text = Text(constants::FONT_PATH);
	Text debug_text = Text(constants::FONT_PATH);
	Text debug_text2 = Text(constants::FONT_PATH);

	rings_text.StringToText("RINGS:");
	rings_count_text.StringToText(to_string(Tails.getRings()));
	debug_text.StringToText("");
	debug_text2.StringToText("");*/

	// Sprites
	Sprite sky{ constants::SKY_PATH };
	Sprite lives{ constants::LIVES_PATH };
	 
	cam.position = { 0, 0 };

	background = DataReader::LoadBackground(ASSET"EmeraldHillZone/Background/");
	
	SoundHandler::init();
	while (globalObjects::gameState == 0) {
		Timer::updateFrameTime();
		const double thisFrames = Timer::getFrameTime().count() / (1000.0 / 60.0);
		globalObjects::renderTitleScreen(background, cam);
		globalObjects::renderTitleFlash();
		SDL_RenderPresent(globalObjects::renderer);
		if (globalObjects::titleScreen[2].getFrame() == 10) {
			cam.position.x += thisFrames * 20.0;
		}
		globalObjects::input.updateKeys();
		if (globalObjects::input.getKeyState(InputComponent::KeyMap::JUMP)) {
			globalObjects::gameState = 1;
			globalObjects::unloadTitleScreen();
			break;
		}
	}

	double avg_fps = 0.0;

	//debug_text.StringToText("nope");

	SoundHandler::setMusic(ASSET"TheAdventureContinues", true);

	cam.position = { 0, 0 };

	Timer::updateFrameTime();

	auto fpsCounterStart = Timer::getTime();

	CollisionTile::test();

	while (true) {
		SoundHandler::updateMusic();

		Timer::updateFrameTime();
		globalObjects::input.updateKeys();

		if (globalObjects::debug) {
			Timer::slowMotion ^= globalObjects::input.getKeyPress('m');
			Timer::paused ^= globalObjects::input.getKeyPress('f');
			if (globalObjects::input.getKeyPress('r')) {
				Timer::frameAdvance();
			}
			else if (Timer::paused) {
				continue;
			}
		}

		if (frames % 1000 == 0) {
			avg_fps = 1000.0 * 1000.0 / (Timer::getTime() - fpsCounterStart).count();
			cout << "Average FPS: " << avg_fps << "\n";
			fpsCounterStart = Timer::getTime();
		}

		currentAct.updateEntities(Tails, cam);

		cam.updatePos(Tails);

		sky.render({ 0, 0, WINDOW_HORIZONTAL_SIZE, WINDOW_VERTICAL_SIZE });

		std::stringstream debugText;
		debugText << "Position: " << Tails.getPosition().x << " " << Tails.getPosition().y << "\n";
		debugText << "Velocity: " << setprecision(3) << setw(5) << Tails.getVelocity().x << " " << setw(5) << Tails.getVelocity().y << "\n";
		debugText << "Angle: " << Tails.getAngle() << " Deg: "  << hexToDeg(Tails.getAngle()) << "\n";
		debugText << "Spindash: " << Tails.getSpindash() << "\n";
		{
			SDL_Point mousePos;
			SDL_GetMouseState(&mousePos.x, &mousePos.y);
			mousePos = SDL_Point{ int(cam.getPosition().x), int(cam.getPosition().y) } + (mousePos * SCREEN_RATIO);
			debugText << "PATH: " << std::boolalpha << Tails.getPath() << "\n";
			debugText << "MOUSE: " << mousePos.x << " " << mousePos.y << "\n";
		}

		currentAct.renderObjects(Tails, cam);

		lives.render({ 2 * 8, WINDOW_VERTICAL_SIZE - 2 * 24, 2 * lives.size().x, 2 * lives.size().y });

		effectManager::updateFade();

		text::renderAbsolute({ 1, 1 }, "GUI", "RINGS: " + std::to_string(Tails.getRings()));

		text::renderAbsolute({ 1, 15 }, "GUI", debugText.str());

		window.updateDisplay();
		
		/*if (SoundHandler::musicState == 0 && effectManager::fadeComplete()) {
			Act::loadNextAct(acts, actPaths, currentAct, arrayData, background);
			Tails.position = { 20, 0 };
			cam.position = { 0, 0 };
			effectManager::fadeFrom(false, 120.0);
			Tails.setActCleared(false);
			Tails.getAnim(13)->SetFrame(0);
			LoadAct(&acts.front(), &cam, globalObjects::renderer);
			SoundHandler::setMusic(ASSET"TheAdventureContinues", true);
		}

		SDL_Rect playerPosition = Tails.getPosition();
		SDL_Rect winArea = currentAct.getWinArea();
		if (SDL_HasIntersection(&playerPosition, &winArea) || (!Mix_PlayingMusic() && SoundHandler::musicState == 3)) {
			std::cout << "Level " << currentAct.getNumber() << " complete!";
			SoundHandler::musicState = 0;
			effectManager::fadeTo(false, 120.0);
			Tails.setActCleared(true);
		}*/

		++frames;

		if (globalObjects::input.getKeyState(static_cast< int >('x'))) {
			break;
		}
	};

	Uint32 end = SDL_GetTicks();

	return 0; 
}
