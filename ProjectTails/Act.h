#pragma once
#include <vector>
#include <string>
#include <iostream>
#include <list>
#include "SDL.h"
#include "SDL_mixer.h"
#include "SDL_image.h"
#include "Animation.h"
#include "Player.h"
#include "PhysicsEntity.h"
#include "Camera.h"
#include "Ground.h"
#include <unordered_map>
#include "SoundHandler.h"
#include "DataReader.h"
#include "Typedefs.h"

enum ActType { TITLE, TORNADO, NORMAL };

class Act
{
public:
	Act();

	Act(const int& num, const std::string& name1, const std::vector < PhysStructInit >& ent, const SDL_Rect& w, const ActType& a, const double& screenRatio, const SDL_Point& levelSize, const std::vector < std::vector < Animation > >& backgnd, std::vector < Ground >&& ground = std::vector < Ground > ());
	Act(Act&& other);


	void SetRenderer(SDL_Renderer* r);
	void SetCamera(Camera* c);

	void SetWindow(SDL_Window* w) { window = w; };
	
	SDL_Rect GetWinArea() { return win; };


	void RenderObjects(PRHS_Window* win, Player& player);


	void UpdateCollisions(Player* player);
	void UpdateEntities(Player& player);

	int numEntities() { return entities.size(); };

	std::string GetName() { return name + "Zone"; };
	int GetNumber() { return number; };

	std::vector < std::vector < Ground > >& getGround() { return solidTiles; };


	std::unique_ptr < PhysicsEntity >& GetEntity(int index) { return entities[index]; };
	std::vector < std::unique_ptr < PhysicsEntity > >& GetEntityArray() { return entities; };

	void resetTime() { last_time = time = SDL_GetTicks(); };

	static void setProps(std::vector < PhysProp >& p) { props = p; };
	static void setKeys(std::unordered_map < std::string, PhysProp* >& p) { entityKeys = p; };

	void Init();

	void Unload();

	~Act();

	void incrFrame() { frame++; };
	int getFrame() { return frame; };

	ActType getType() { return aType; };

	typedef matrix < int > matrix;

	static void loadNextAct(std::list<Act>& acts, std::vector<std::string>& actPaths, int& currentAct, matrix& blocks, matrix& blockFlags, matrix& collides, matrix& collideFlags,std::vector<std::vector<Animation>>& background);

private:
	typedef std::vector < std::unique_ptr < PhysicsEntity > > entityList;
	typedef std::vector < PhysStruct > entityLoadList;
	typedef std::list < Ground* > solidRenderList;

	typedef entityList::iterator entityListIterator;
	typedef entityLoadList::iterator entityLoadIterator;
	typedef solidRenderList::iterator solidRenderListIterator;

	Act(const Act& other) {};
	bool debounce;
	Uint32 time;
	Uint32 last_time;
	int number; //Which zone it is, title screen = 0.
	double ratio; //Ratio of playfield coords to screen coords
	std::string name; //The zone's name. Does NOT include "zone," that's added later.
	std::vector < std::string > sound_paths; //Strings for the sound paths.
	entityLoadList phys_paths; //Data for initializing the physics objects
	entityList entities; //Array for the generic physics objects
	std::vector < std::vector < Ground > > solidTiles; //Array for ground
	std::vector < std::vector < Animation > > background;
	solidRenderList solidTileRender;
	SDL_Renderer* renderer;
	Camera* cam;
	int loaded_entities;
	SDL_Rect win;
	ActType aType;
	SDL_Window* window;
	int frame;
	static std::vector < PhysProp > props;
	static std::unordered_map < std::string, PhysProp* > entityKeys;
};

