#include "stdafx.h"
#include "Act.h"

std::vector < PhysProp > Act::props = std::vector < PhysProp >();
std::unordered_map < std::string, PhysProp* > Act::entityKeys = std::unordered_map < std::string, PhysProp* >();

bool operator== (const SDL_Rect& a, const SDL_Rect& b) {
	return (a.x == b.x) && (a.y == b.y) && (a.w == b.w) && (a.h == b.h);
}

bool operator== (const SDL_Point& a, const SDL_Point& b) {
	return (a.x == b.x) && (a.y == b.y);
}

Act::Act()
{
}

Act::Act(Act&& other) :
	number(std::move(other.number)),
	name(std::move(other.name)),
	phys_paths(std::move(other.phys_paths)),
	loaded_entities(std::move(other.loaded_entities)),
	win(std::move(other.win)),
	aType(std::move(other.aType)),
	ratio(std::move(other.ratio)),
	frame(std::move(other.frame)),
	entities(std::move(other.entities)),
	cam(std::move(other.cam)),
	renderer(std::move(other.renderer)),
	solidTiles(std::move(other.solidTiles)),
	time(std::move(other.time)),
	last_time(std::move(other.time)),
	debounce(std::move(other.debounce)),
	background(std::move(other.background))
{
	for (int x = 0; x < solidTiles.size(); x++) {
		for (int y = 0; y < solidTiles[x].size(); y++) {
			if (solidTiles[x][y].getPosition().x != -1)
				solidTileRender.push_back(&solidTiles[x][y]);
		}
	}
	std::cout << "Act move constructor was called.\n";
}

Act::Act(const int& num, const std::string& name1, const std::vector < PhysStructInit >& ent, const SDL_Rect& w, const ActType& a, const double& screenRatio, const SDL_Point& levelSize, const std::vector < std::vector < Animation > >& backgnd, std::vector < Ground >&& ground) :
	solidTiles(levelSize.x, std::vector < Ground >(levelSize.y, Ground())),
	time(SDL_GetTicks()),
	last_time(time),
	number(num),
	name(name1),
	loaded_entities(0),
	win(w),
	aType(a),
	ratio(screenRatio),
	frame(0),
	debounce(false),
	background(backgnd)
{
	for (int i = 0; i != ent.size(); i++) {
		phys_paths.emplace_back(ent[i].pos, *entityKeys.find(ent[i].prop)->second, i, ent[i].flags);
	}
	SDL_Rect pos;
	for (int i = 0; i != ground.size(); i++) {
		pos = ground[i].getPosition();
		solidTiles[pos.x / (TILE_WIDTH * GROUND_WIDTH)][pos.y / (TILE_WIDTH * GROUND_WIDTH)] = std::move(ground[i]);
		solidTileRender.push_back(&solidTiles[pos.x / (TILE_WIDTH * GROUND_WIDTH)][pos.y / (TILE_WIDTH * GROUND_WIDTH)]);
	}
	
}

void Act::initialize() {
	std::cout << "Initializing act " << number << "!\n";
	std::cout << "Loading entities...\n";
	std::vector<PhysStruct>::iterator i = phys_paths.begin(); 
	while (i != phys_paths.end()) {
		if (SDL_HasIntersection(&(i->pos), &(cam->getCollisionRect()))) {
			std::cout << "Loading entity at position " << i->pos.x << " " << i->pos.y << "\n";
			loaded_entities++;
			entities.emplace_back(new PhysicsEntity(*i));
			i = phys_paths.erase(i);
			continue;
		}
		++i;
	}
	std::cout << "Entity loading done!\n";
	std::cout << "Done initializing act!\n";
}

//Unload offscreen entities and update onscreen ones
void Act::updateEntities(Player& player) {
	last_time = time;
	time = SDL_GetTicks();
	Animation::setTimeDifference(time - last_time);

	// Load new entities
	entityLoadIterator load = phys_paths.begin();
	while (load != phys_paths.end()) {
		if (SDL_HasIntersection(&(load->pos), &(cam->getCollisionRect()))) {
			std::cout << "Loading entity at position " << load->pos.x << " " << load->pos.y << "\n";
			entities.emplace_back(new PhysicsEntity(*load));
			entities.back()->setTime(SDL_GetTicks());
			entities.back()->loaded = true;
			entities.back()->shouldSave = true;
			load = phys_paths.erase(load);
			loaded_entities++;
			continue;
		}
		
		++load;
	}

	entityListIterator i = entities.begin();
	// Unload offscreen entities
	while (i != entities.end()) {
		if (!SDL_HasIntersection(&(*i)->getPosition(), &(cam->getCollisionRect())) && (*i)->loaded) {
			if ((*i)->shouldSave) {
				phys_paths.emplace_back((*i)->getPosition(), (*i)->GetProp(), true, (*i)->getFlags());
				i = entities.erase(i);
				continue;
			}
		}
		++i;
	}

	i = entities.begin();

	// Update entities
	std::vector < bool > toDestroy(entities.size(), false);
	std::vector < std::unique_ptr < PhysicsEntity > > toAdd;
	EntityManager manager(entities, toDestroy, toAdd);
	while (i != entities.end()) {

		(*i)->loaded = true;
		(*i)->update(true, &player, &manager);

		++i;
	}

	updateCollisions(player, toDestroy, toAdd);

	player.update(solidTiles, manager);

	// Unload entities marked to be destroyed
	i = entities.begin();
	std::vector<bool>::iterator j = toDestroy.begin();
	while (j != toDestroy.end()) {
		if (*j) {
			j = toDestroy.erase(j);
			i = entities.erase(i);
			continue;
		}
		++i;
		++j;
	}

	// Add entities created by other entities
	i = toAdd.begin();
	while (i != toAdd.end()) {
		entities.push_back(std::move(*i));
	}
}

void Act::updateCollisions(Player& player, std::vector < bool >& toDestroy, std::vector < std::unique_ptr < PhysicsEntity > >& toAdd) {
	bool destroyed = false;
	bool hurt = false;
	entityListIterator i = entities.begin();
	SDL_Point center{ -1, -1 };
	while (i != entities.end()) {
		destroyed = false;
		EntType collideType;
		SDL_Rect playerCollide = player.getCollisionRect();
		SDL_Rect objCollide = (*i)->getCollisionRect();
		if (SDL_HasIntersection(&playerCollide, &objCollide) && (*i)->canCollide) {
			debounce = false;
			collideType = (*i)->getType();

			player.addCollision(&(*i));
		}
		else if ((*i)->getType() == PATHSWITCH) {
			(*i)->setCustom(1, static_cast <double> (false));
		}
		if (!destroyed) {
			i++;
		}
	}
}

Act::~Act()
{
}

void Act::setRenderer(SDL_Renderer* r) {
	renderer = r;
}

void Act::setCamera(Camera* c) {
	cam = c;
}

void Act::renderObjects(Player& player) {
	globalObjects::renderBackground(background, cam->getPosition().x - cam->GetOffset().x, ratio);
	entityListIterator i = entities.begin();
	SDL_Rect pos = cam->getPosition();
	while (i != entities.end()) {
		(*i)->Render(pos, ratio);
		i++;
	}
	solidRenderListIterator solidLayer = solidTileRender.begin();
	while (solidLayer != solidTileRender.end()) {
		if (SDL_HasIntersection(&(*solidLayer)->getPosition(), &cam->getCollisionRect())) {
			(*solidLayer)->Render(pos, ratio, nullptr, 1);
		}
		solidLayer++;
	}
	if (player.getOnGround()) {
		player.render(pos, ratio);
	}
	solidLayer = solidTileRender.begin();
	while (solidLayer != solidTileRender.end()) {
		if (SDL_HasIntersection(&(*solidLayer)->getPosition(), &cam->getCollisionRect())) {
			(*solidLayer)->Render(pos, ratio, nullptr, 0);
		}
		solidLayer++;
	}
	if (!player.getOnGround()) {
		player.render(pos, ratio);
	}
}

void Act::loadNextAct(std::list<Act>& acts, std::vector<std::string>& actPaths, int& current, std::vector < Ground::groundArrayData >& arrayData, std::vector<std::vector<Animation>>& background) {
	current++;
	int actNum;
	std::string actName;
	std::vector < PhysStructInit > entities;
	SDL_Rect winArea;
	ActType actType;
	std::vector < Ground > ground;
	std::vector < DataReader::groundData > groundIndices;
	SDL_Point levelSize;
	DataReader::LoadActData(actPaths[current], actNum, actName, entities, winArea, actType, &ground, &arrayData, &groundIndices, &levelSize);
	acts.emplace_back(actNum, actName, entities, winArea, actType, globalObjects::ratio, levelSize, background, std::move(ground));
	acts.pop_front();
}