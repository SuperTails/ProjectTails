// ProjectTails.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "InputComponent.h"
#include "prhsGameLib.cpp"
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

using namespace std;

const double SCREEN_RATIO = 0.5;
const double SCR_RAT_INV = 1.0 / SCREEN_RATIO;

const double stepLength = 1000.0 / 60.0;

const double LOAD_STEPS = 7.0;

void LoadAct(Act* a, Camera* c, SDL_Renderer* r, std::vector < PhysProp > p) {
	a->SetCamera(c);
	a->SetRenderer(r);
	a->Init();
	a->setProps(p);
}

#undef main

int mult(int arg1, double arg2) {
	return (int)((double)arg1 * arg2);
}

int main( int argc, char* argv[] ) { 
	Uint32 frame_start_time;

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

	//Asset paths
	string TORNADO_PATH = ASSET"Tornado.png";
	string MUSIC_PATH = ASSET"WorldToExplore.wav";
	string SKY_PATH = ASSET"Sky.png";
	string TAILS_IDLE_PATH = ASSET"Tails_Idle.png";
	string RING_PATH = ASSET"Ring.png";
	string RING_SPARKLE_PATH = ASSET"RingSparkle.png";
	string FONT_PATH = ASSET"FontGUI.png";
	string EXPLOSION_PATH = ASSET"Explosion.png";
	string ROCKET_PATH = ASSET"Rocket.png";
	string ENTITY_DATA_PATH = ASSET"EntityData.txt";
	

	string ACT1_DATA_PATH = ASSET"Act1";
	string ACT2_DATA_PATH = ASSET"Act2";

	std::vector < std::string > actPaths = { ACT1_DATA_PATH, ACT2_DATA_PATH };
	

	string LIVES_PATH = ASSET"Lives.png";
	string IDLE_PATH = ASSET"Tails_Idle.png";
	string WALK_PATH = ASSET"Tails_Walk.png";
	string RUN_PATH = ASSET"Tails_Run.png";
	string ROLL_BODY_PATH = ASSET"Tails_Roll_Body.png";
	string ROLL_TAILS_PATH = ASSET"Tails_Roll_Tails.png";
	string CROUCH_PATH = ASSET"Tails_Crouch.png";
	string LOOK_UP_PATH = ASSET"Tails_Look_Up.png";
	string SPINDASH_PATH = ASSET"Tails_Spindash.png";
	string FLY_PATH = ASSET"Tails_Fly.png";
	string FLY_TIRED_PATH = ASSET"Tails_Fly_Tired.png";
	string CORKSCREW_PATH = ASSET"Tails_Corkscrew.png";
	string HURT_PATH = ASSET"Tails_Hurt.png";
	string ACT_CLEAR_PATH = ASSET"Tails_Act_Clear.png";

	Player Tails = Player();

	Tails.updatePosition({ 20, 0, 120, 64, 0 }, PRHS_UPDATE_ABSOLUTE);
	Tails.AddAnim({ TORNADO_PATH, 50, 4 });		//0
	Tails.AddAnim({ IDLE_PATH, 133, 4 });
	Tails.AddAnim({ WALK_PATH, 133, 7 });
	Tails.AddAnim({ RUN_PATH,  133, 4 });
	Tails.AddAnim({ ROLL_BODY_PATH, 64, 6 });	//4
	Tails.AddAnim({ ROLL_TAILS_PATH, 128, 3 });
	Tails.AddAnim({ CROUCH_PATH, 133, 5 });
	Tails.AddAnim({ SPINDASH_PATH, 30, 5 });
	Tails.AddAnim({ FLY_PATH, 50, 2 });			//8
	Tails.AddAnim({ FLY_TIRED_PATH, 60, 4 });
	Tails.AddAnim({ LOOK_UP_PATH, 133, 5 });
	Tails.AddAnim({ CORKSCREW_PATH, 50, 11 });
	Tails.AddAnim({ HURT_PATH, 32, 2 });		//12
	Tails.AddAnim({ ACT_CLEAR_PATH, 150, 3 });
	Tails.setCollisionRect({ 0,19,120,30 });
	//REMOVE LATER	

	SDL_Rect CAMERA_SIZE = { (-32), (-8), int(WINDOW_HORIZONTAL_SIZE * SCREEN_RATIO + 64), int(WINDOW_VERTICAL_SIZE * SCREEN_RATIO + 16) };
	Camera cam = Camera(&CAMERA_SIZE);
	cam.SetOffset({ (int)(-WINDOW_HORIZONTAL_SIZE * SCREEN_RATIO / 2), (int)(-WINDOW_VERTICAL_SIZE * SCREEN_RATIO + 128) });

	//Load entities:
	cout << "Loading entity data...\n";
	std::vector < PhysProp > EntityData;
	std::unordered_map < std::string, PhysProp* > entityKeys;
	std::vector < std::string > entityTypes;
	
	DataReader::LoadEntityData(ENTITY_DATA_PATH, EntityData, entityKeys, entityTypes);
	globalObjects::updateLoading(1 / LOAD_STEPS);

	LevelEditor::entityList = &entityKeys;
	LevelEditor::entityTypes = &entityTypes;

	int actNum;
	string actName;
	vector < PhysStructInit > entities;
	SDL_Rect winRect;
	ActType actType;
	std::vector < std::vector < Animation > > background;

	Act::setProps(EntityData);
	PhysicsEntity::physProps = &entityKeys;
	Act::setKeys(entityKeys);

	std::list < Act > acts;

	int currentAct = 0;
	
	Animation::setRenderer(globalObjects::renderer);

	std::vector < CollisionTile > tiles;
	
	std::vector < Ground > ground;
	matrix < int > collides(221);
	std::vector < double > angles(221);
	DataReader::LoadCollisionsFromImage(ASSET"SolidGraph.png", collides, angles);
	globalObjects::updateLoading(1 / LOAD_STEPS);

	collides[220] = std::vector < int >(16, 16);
	angles[220] = 0xFF;

	DataReader::LoadTileData(tiles, collides, angles);

	Ground::setList(tiles);

	std::vector < Ground::groundArrayData > arrayData;
	SDL_Point levelSize;

	std::vector < DataReader::groundData > groundIndices;

	arrayData.resize(constants::NUM_BLOCKS);

	for (int i = 0; i < constants::NUM_BLOCKS; i++) {
		DataReader::LoadJSONBlock(ASSET"EmeraldHillZone\\Block" + std::to_string(i + 1) + ".json", arrayData[i]);
		globalObjects::updateLoading((1 / LOAD_STEPS) / constants::NUM_BLOCKS);
	}
	DataReader::LoadBackground(ASSET"EmeraldHillZone\\Background", background, 2);
	globalObjects::updateLoading(1 / LOAD_STEPS);

	DataReader::LoadActData(ACT1_DATA_PATH, actNum, actName, entities, winRect, actType, &ground, &arrayData, &groundIndices, &levelSize);
	globalObjects::updateLoading(1 / LOAD_STEPS);

	SDL_SetRenderDrawColor(globalObjects::renderer, 0, 0, 0, 0);

	LevelEditor::cam = &cam;
	LevelEditor::levelEntities = entities;

	if (LevelEditor::levelEditing) {
		LevelEditor::init(groundIndices, levelSize, arrayData);
		while (true) {
			globalObjects::last_time = globalObjects::time;
			globalObjects::time = SDL_GetTicks();
			globalObjects::input.UpdateKeys();
			if (LevelEditor::handleInput())
				break;
			LevelEditor::renderTiles();
			LevelEditor::renderEntities();
			LevelEditor::renderText();
			SDL_RenderPresent(globalObjects::renderer);
		}
		std::string output = LevelEditor::convertToString(actNum, actName);
		std::ofstream DataFile(ASSET"Act1Data.txt");
		DataFile << output;
		return 0;
	}

	acts.emplace_back(actNum, actName, entities, winRect, actType, SCREEN_RATIO, levelSize, background, std::move(ground));
	globalObjects::updateLoading(1 / LOAD_STEPS);

	Tails.SetActType(acts.front().getType());

	LoadAct(&(acts.front()), &cam, globalObjects::renderer, EntityData);
	globalObjects::updateLoading(1 / LOAD_STEPS);

	Tails.SetRings(0);

	int frames = 0;
	int last_frames = 0;
	Uint32 start = SDL_GetTicks();

	Text rings_text = Text(FONT_PATH);
	Text rings_count_text = Text(FONT_PATH);
	Text debug_text = Text(FONT_PATH);
	Text debug_text2 = Text(FONT_PATH);

	rings_text.StringToText("RINGS:");
	rings_count_text.StringToText(to_string(Tails.GetRings()));
	debug_text.StringToText("");
	debug_text2.StringToText("");

	//Load Surfaces
	SDL_Surface* Sky = IMG_Load(SKY_PATH.c_str());
	SDL_Surface* Lives = IMG_Load(LIVES_PATH.c_str());
	 
	//Text
	SDL_Texture* rings_text_texture = SDL_CreateTextureFromSurface(window.getRenderer(), rings_text.getText());
	SDL_Texture* rings_count_texture = SDL_CreateTextureFromSurface(window.getRenderer(), rings_count_text.getText());
	SDL_Texture* debug_text_texture = SDL_CreateTextureFromSurface(window.getRenderer(), debug_text.getText());
	SDL_Texture* debug_text_texture2 = SDL_CreateTextureFromSurface(window.getRenderer(), debug_text2.getText());
	//Others
	SDL_Texture* Sky_Texture = SDL_CreateTextureFromSurface(window.getRenderer(), Sky);
	SDL_Texture* Lives_Texture = SDL_CreateTextureFromSurface(window.getRenderer(), Lives);
	

	//cam.updatePosition({Tails.getPosition().x + cam.GetOffset().x, Tails.getPosition().y+ cam.GetOffset().y, cam.getPosition().w, cam.getPosition().h}, PRHS_UPDATE_ABSOLUTE);

	cam.updatePosition(PRHS_Rect{ 0, 0, 0, 0, 0 }, PRHS_UPDATE_ABSOLUTE);

	Uint32 last_time = SDL_GetTicks();
	Uint32 last_time1 = SDL_GetTicks();
	Uint32 time = SDL_GetTicks();

	SoundHandler::init();
	acts.front().resetTime();
	while (!globalObjects::gameState) {
		last_time = time;
		time = SDL_GetTicks();
		Animation::setTimeDifference(time - last_time);
		double thisFrames = ((time - last_time) / (1000.0 / 60.0));
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
			Tails.GetAnim(0)->refreshTime();
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
		globalObjects::last_time = globalObjects::time;
		globalObjects::time = SDL_GetTicks();
		globalObjects::input.UpdateKeys();

		if (!(frames % 500)) {
			cout << "Average FPS: " << 1000.0 * 500.0 / (time - last_time1) << "\n";
			avg_fps = 500.0 * 1000.0 / (time - last_time1);
			last_time1 = time;
			time = SDL_GetTicks();
		}

		if (currentAct >= 0) {

			acts.front().UpdateEntities(Tails);
			acts.front().UpdateCollisions(&Tails);

			rings_count_text.StringToText(to_string(Tails.GetRings()));
			rings_count_texture = SDL_CreateTextureFromSurface(window.getRenderer(), rings_count_text.getText());
			debug_text_texture2 = SDL_CreateTextureFromSurface(window.getRenderer(), debug_text2.getText());


			debug_text2.StringToText(Tails.CollideGround(acts.front().getGround()));
			Tails.UpdateP(cam);
			cam.updatePos(Tails.getPosition(), Tails.lookDirection());

			window.render(Sky_Texture, { 0, 0, WINDOW_HORIZONTAL_SIZE, WINDOW_VERTICAL_SIZE, 0 });

			acts.front().RenderObjects(Tails);

			window.render(rings_text_texture, { 25, 25, rings_text.getText()->w, rings_text.getText()->h, 0 });
			window.render(rings_count_texture, { 25 + rings_text.getText()->w + 3, 25, rings_count_text.getText()->w, rings_count_text.getText()->h, 0 });
			window.render(Lives_Texture, { 2 * 8, WINDOW_VERTICAL_SIZE - 2 * 24, 2 * Lives->w, 2 * Lives->h, 0 });

			window.render(debug_text_texture2, { 25, 75, debug_text2.getText()->w, debug_text2.getText()->h, 0 });

			SDL_DestroyTexture(rings_count_texture);
			SDL_DestroyTexture(debug_text_texture2);
			
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
				Tails.GetAnim(13)->SetFrame(0);
				LoadAct(&acts.front(), &cam, globalObjects::renderer, EntityData);
				SoundHandler::setMusic("..\\..\\asset\\TheAdventureContinues", true);
			}
			
		}

		frames++;

		if (currentAct >= 0 && SDL_HasIntersection(&Tails.getPosition(), &acts.front().GetWinArea()) || (!Mix_PlayingMusic() && SoundHandler::musicState == 3)) {
			std::cout << "Level " << currentAct << " complete!";
			SoundHandler::musicState = 0;
			effectManager::fadeTo(false, 120.0);
			Tails.setActCleared(true);
		}
	};

	//effectManager::unloadEHZRain();

	Uint32 end = SDL_GetTicks();

	//Quit SDL 
	SDL_Quit();
	
	return 0; 
}
