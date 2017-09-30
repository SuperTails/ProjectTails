// ProjectTails.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "InputComponent.h"
#include "prhsGameLib.h"
#include "png.h"
#include "Animation.h"
#include <algorithm>
#include "Constants.h"
#include "Act.h"
#include "Player.h"
#include "Camera.h"
#include "Text.h"
#include "Ground.h"
#include "DataReader.h"
#include <fstream>
#include <unordered_map>
#include "Miscellaneous.h"
#include "LevelEditor.h"
#include "SoundHandler.h"
#include "effectManager.h"
#include "CollisionTile.h"
#include <cstring>

using namespace std;

const double SCREEN_RATIO = 0.5;
const double SCR_RAT_INV = 1.0 / SCREEN_RATIO;

const double stepLength = 1000.0 / 60.0;

const double LOAD_STEPS = 7.0;

void LoadAct(Act* a, Camera* c, SDL_Renderer* r) {
	a->setCamera(c);
	a->setRenderer(r);
	a->initialize();
}

#undef main

int mult(int arg1, double arg2) {
	return (int)((double)arg1 * arg2);
}

int main( int argc, char* argv[] ) { 
	Uint32 frame_start_time;

	if (argc == 2) {
		LevelEditor::levelEditing = (std::strcmp(argv[1], "le") == 0);
	}

	//Start SDL 
	SDL_Init( SDL_INIT_EVERYTHING ); 
	
	Mix_Init( MIX_INIT_MP3 );

	int img_flags = IMG_INIT_PNG;

	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 4, 1024);

	PRHS_Window window("Project Tails", WINDOW_HORIZONTAL_SIZE, WINDOW_VERTICAL_SIZE, false);

	globalObjects::window = window.getWindow();
	globalObjects::renderer = window.getRenderer();
	globalObjects::ratio = SCREEN_RATIO;

	SDL_SetRenderDrawColor(globalObjects::renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);

	globalObjects::loadProgress.emplace_back("Loading", 0.0);
	globalObjects::updateLoading(0.0);


	std::vector < std::string > actPaths = { constants::ACT1_DATA_PATH, constants::ACT2_DATA_PATH };

	Player Tails;

	Camera cam = Camera({ -32, -8, int(WINDOW_HORIZONTAL_SIZE * SCREEN_RATIO + 64), int(WINDOW_VERTICAL_SIZE * SCREEN_RATIO + 16) });
	cam.SetOffset({ (int)(-WINDOW_HORIZONTAL_SIZE * SCREEN_RATIO / 2), (int)(-WINDOW_VERTICAL_SIZE * SCREEN_RATIO + 128) });

	//Load entities:
	cout << "Loading entity data...\n";
	
	DataReader::LoadEntityData(constants::ENTITY_DATA_PATH);
	globalObjects::updateLoading(1 / LOAD_STEPS);

	int actNum;
	string actName;
	vector < PhysStruct > entities;
	SDL_Rect winRect;
	ActType actType;
	std::vector < std::vector < Animation > > background;

	std::list < Act > acts;

	int currentAct = 0;
	
	std::vector < CollisionTile > tiles;
	
	std::vector < Ground > ground;
	matrix < int > collides(221);
	std::vector < double > angles(221);
	DataReader::LoadCollisionsFromImage(ASSET"SolidGraph.png", collides, angles);
	globalObjects::updateLoading(1 / LOAD_STEPS);

	collides[220] = std::vector < int >(16, 16);
	angles[220] = 0xFF;

	DataReader::LoadTileData(tiles, collides, angles);

	Ground::setCollisionList(tiles);

	std::vector < Ground::groundArrayData > arrayData;
	SDL_Point levelSize;

	DataReader::LoadActData(constants::ACT1_DATA_PATH, actNum, actName, entities, winRect, actType, ground, levelSize);
	globalObjects::updateLoading(1 / LOAD_STEPS);

	DataReader::LoadLevelBlocks(ASSET"EmeraldHillZone/Block");
	DataReader::LoadBackground(ASSET"EmeraldHillZone/Background", background);
	globalObjects::updateLoading(1 / LOAD_STEPS);

	SDL_SetRenderDrawColor(globalObjects::renderer, 0, 0, 0, 0);

	LevelEditor::cam = &cam;
	LevelEditor::levelEntities = entities;

	if (LevelEditor::levelEditing) {
		LevelEditor::init(ground, levelSize);
		while (true) {
			Timer::updateFrameTime();
			globalObjects::input.UpdateKeys();
			if (LevelEditor::handleInput())
				break;
			LevelEditor::renderTiles();
			LevelEditor::renderEntities();
			LevelEditor::renderText();
			SDL_RenderPresent(globalObjects::renderer);
		}
		if (globalObjects::input.GetKeyState(InputComponent::J)) {
			std::string output = LevelEditor::convertToString(actNum, actName);
			std::ofstream DataFile(ASSET"Act1Data.txt");
			DataFile << output;
		}
		return 0;
	}

	acts.emplace_back(actNum, actName, entities, winRect, actType, SCREEN_RATIO, levelSize, background, ground);
	globalObjects::updateLoading(1 / LOAD_STEPS);

	Tails.setActType(static_cast < unsigned char > (acts.front().getType()));

	LoadAct(&(acts.front()), &cam, globalObjects::renderer);
	globalObjects::updateLoading(1 / LOAD_STEPS);

	int frames = 0;
	int last_frames = 0;
	Uint32 start = SDL_GetTicks();

	Text rings_text = Text(constants::FONT_PATH);
	Text rings_count_text = Text(constants::FONT_PATH);
	Text debug_text = Text(constants::FONT_PATH);
	Text debug_text2 = Text(constants::FONT_PATH);

	rings_text.StringToText("RINGS:");
	rings_count_text.StringToText(to_string(Tails.getRings()));
	debug_text.StringToText("");
	debug_text2.StringToText("");

	//Load Surfaces
	SDL_Surface* Sky = IMG_Load(constants::SKY_PATH.c_str());
	SDL_Surface* Lives = IMG_Load(constants::LIVES_PATH.c_str());
	 
	//Others
	SDL_Texture* Sky_Texture = SDL_CreateTextureFromSurface(window.getRenderer(), Sky);
	SDL_Texture* Lives_Texture = SDL_CreateTextureFromSurface(window.getRenderer(), Lives);
	
	cam.updatePosition(PRHS_Rect{ 0, 0, 0, 0, 0 }, PRHS_UPDATE_ABSOLUTE);

	Uint32 last_time = SDL_GetTicks();
	Uint32 last_time1 = SDL_GetTicks();
	Uint32 time = SDL_GetTicks();

	SoundHandler::init();
	while (globalObjects::gameState == 0) {
		Timer::updateFrameTime();
		double thisFrames = Timer::getFrameTime().count() / (1000.0 / 60.0);
		globalObjects::renderTitleScreen(background, cam.getPosition().x);
		globalObjects::renderTitleFlash();
		SDL_RenderPresent(globalObjects::renderer);
		if (globalObjects::titleScreen[2].getFrame() == 10) {
			cam.updatePosition(PRHS_Rect{ int(thisFrames * 20), 0, 0, 0, 0 }, PRHS_UPDATE_RELATIVE);
		}
		globalObjects::input.UpdateKeys();
		if (globalObjects::input.GetKeyState(InputComponent::JUMP)) {
			globalObjects::gameState = 1;
			globalObjects::unloadTitleScreen();
			break;
		}
	}

	double avg_fps = 0;

	debug_text.StringToText("nope");

	SoundHandler::setMusic(ASSET"TheAdventureContinues", true);

	cam.updatePosition(PRHS_Rect{ 0, 0, 0, 0, 0 }, PRHS_UPDATE_ABSOLUTE);

	while (true) {

		SoundHandler::updateMusic();

		frame_start_time = SDL_GetTicks();
		Timer::updateFrameTime();
		globalObjects::input.UpdateKeys();

		if (frames % 1000 == 0) {
			cout << "Average FPS: " << 1000.0 * 1000.0 / (frame_start_time - last_time1) << "\n";
			avg_fps = 1000.0 * 1000.0 / (frame_start_time - last_time1);
			last_time1 = frame_start_time;
		}

		if (currentAct >= 0) {

			acts.front().updateEntities(Tails);

			rings_count_text.StringToText(to_string(Tails.getRings()));

			cam.updatePos(Tails);

			window.render(Sky_Texture, { 0, 0, WINDOW_HORIZONTAL_SIZE, WINDOW_VERTICAL_SIZE, 0 });

			acts.front().renderObjects(Tails);

			rings_text.Render(SDL_Point { 25, 25 });
			rings_count_text.Render(SDL_Point { rings_text.getText()->w + 28, 25 });

			debug_text.StringToText("Position: " + std::to_string(Tails.getPosition().x) + " " + std::to_string(Tails.getPosition().y));
			debug_text.Render(SDL_Point { 25, 40 });

			window.render(Lives_Texture, { 2 * 8, WINDOW_VERTICAL_SIZE - 2 * 24, 2 * Lives->w, 2 * Lives->h, 0 });

			{
				std::pair < int, int > mousePos;
				SDL_GetMouseState(&mousePos.first, &mousePos.second);
				mousePos.first = cam.getPosition().x + mousePos.first * SCREEN_RATIO;
				mousePos.second = cam.getPosition().y + mousePos.second * SCREEN_RATIO;
				debug_text2.StringToText("MOUSE: " + std::to_string(mousePos.first) + " " + std::to_string(mousePos.second));
				debug_text2.Render(SDL_Point { 25, 55 });
			}
			
			effectManager::updateFade();

			window.updateDisplay();
			
			acts.front().incrFrame();

			if (SoundHandler::musicState == 0 && effectManager::fadeComplete()) {
				Act::loadNextAct(acts, actPaths, currentAct, arrayData, background);
				Tails.updatePosition({ 20, 0, 120, 64, 0 }, PRHS_UPDATE_ABSOLUTE);
				SDL_Rect camPos = cam.getPosition();
				camPos.x = 0;
				camPos.y = 0;
				cam.updatePosition(convertRect(camPos), PRHS_UPDATE_ABSOLUTE);
				effectManager::fadeFrom(false, 120.0);
				Tails.setActCleared(false);
				Tails.getAnim(13)->SetFrame(0);
				LoadAct(&acts.front(), &cam, globalObjects::renderer);
				SoundHandler::setMusic(ASSET"TheAdventureContinues", true);
			}
		}

		++frames;

		SDL_Rect playerPosition = Tails.getPosition();
		SDL_Rect winArea = acts.front().getWinArea();
		if ((currentAct >= 0 && SDL_HasIntersection(&playerPosition, &winArea)) || (!Mix_PlayingMusic() && SoundHandler::musicState == 3)) {
			std::cout << "Level " << currentAct << " complete!";
			SoundHandler::musicState = 0;
			effectManager::fadeTo(false, 120.0);
			Tails.setActCleared(true);
		}

		if (globalObjects::input.GetKeyState( static_cast<int>(InputComponent::X) ) ) {
			break;
		}		
	};

	Uint32 end = SDL_GetTicks();

	//Quit SDL 
	SDL_Quit();
	
	return 0; 
}
