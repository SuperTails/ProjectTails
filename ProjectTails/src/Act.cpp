#include "Act.h"

//std::vector < PhysProp > Act::props = std::vector < PhysProp >();
//std::unordered_map < std::string, PhysProp* > Act::entityKeys = std::unordered_map < std::string, PhysProp* >();

//enum class ActType : unsigned char { TITLE, NORMAL, TORNADO };

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
	debounce(std::move(other.debounce)),
	background(std::move(other.background))
{
	for (int x = 0; x < solidTiles.size(); x++) {
		for (int y = 0; y < solidTiles[x].size(); y++) {
			if (solidTiles[x][y].getIndex() != -1) {
				solidTileRender.push_back(&solidTiles[x][y]);
			}
		}
	}
	std::cout << "Act move constructor was called.\n";
}

Act::Act(const int& num, const std::string& name1, const std::vector < PhysStruct >& ent, const SDL_Rect& w, const ActType& a, double screenRatio, const SDL_Point& levelSize, const std::vector < std::vector < Animation > >& backgnd, std::vector < Ground >& ground) :
	solidTiles(levelSize.x, std::vector < Ground >(levelSize.y, Ground())),
	number(num),
	name(name1),
	loaded_entities(0),
	win(w),
	aType(a),
	ratio(screenRatio),
	frame(0),
	debounce(false),
	background(backgnd),
	phys_paths(ent.begin(), ent.end())
{
	for (int i = 0; i != ground.size(); i++) {
		SDL_Point pos;
		pos = ground[i].getPosition();
		solidTiles[pos.x][pos.y] = ground[i];
		solidTileRender.push_back(&solidTiles[pos.x][pos.y]);
	}
	
}

void Act::initialize() {
	std::cout << "Initializing act " << number << "!\n";
	std::cout << "Loading entities...\n";
	std::vector<PhysStruct>::iterator i = phys_paths.begin(); 
	SDL_Rect cameraView = (cam->getCollisionRect());
	while (i != phys_paths.end()) {
		if (SDL_HasIntersection(&(i->position), &cameraView)) {
			std::cout << "Loading entity at position " << i->position.x << " " << i->position.y << "\n";
			++loaded_entities;
			entities.emplace_back(std::make_unique<PhysicsEntity>(*i));
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
	// Load new entities
	entityLoadIterator load = phys_paths.begin();
	SDL_Rect cameraView = (cam->getCollisionRect());
	while (load != phys_paths.end()) {
		if (SDL_HasIntersection(&(load->position), &cameraView)) {
			std::cout << "Loading entity at position " << load->position.x << " " << load->position.y << "\n";
			entities.emplace_back(new PhysicsEntity(*load));
			entities.back()->shouldSave = true;
			load = phys_paths.erase(load);
			++loaded_entities;
			continue;
		}
		
		++load;
	}

	entityListIterator i = entities.begin();
	// Unload offscreen entities
	while (i != entities.end()) {
		SDL_Rect position = (*i)->getPosition();
		if (!SDL_HasIntersection(&position, &cameraView)) {
			if ((*i)->shouldSave) {
				phys_paths.push_back((*i)->toPhysStruct());
				i = entities.erase(i);
				continue;
			}
		}
		++i;
	}

	// Update entities
	std::vector < bool > toDestroy(entities.size(), false);
	std::vector < std::unique_ptr < PhysicsEntity > > toAdd;
	EntityManager manager(entities, toDestroy, toAdd);
	for (auto& entity : entities) {
		entity->update(&player, &manager);
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
	for (auto& entityToAdd : toAdd) {
		entities.push_back(std::move(entityToAdd));
	}
}

void Act::updateCollisions(Player& player, std::vector < bool >& toDestroy, std::vector < std::unique_ptr < PhysicsEntity > >& toAdd) {
	for (auto& entity : entities) {
		const SDL_Rect playerCollide = player.getCollisionRect();
		const SDL_Rect objCollide = entity->getCollisionRect();
		if (SDL_HasIntersection(&playerCollide, &objCollide) && entity->canCollide) {
			player.addCollision(entity);
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
	SDL_Rect pos = cam->getPosition();
	SDL_Rect cameraView = cam->getCollisionRect();

	globalObjects::renderBackground(background, pos.x - cam->GetOffset().x, ratio);

	for (const Ground* ground : solidTileRender) {
		SDL_Point gridPos = ground->getPosition();
		SDL_Rect position{ static_cast<int>(gridPos.x * GROUND_PIXEL_WIDTH), static_cast<int>(gridPos.y * GROUND_PIXEL_WIDTH), GROUND_PIXEL_WIDTH, GROUND_PIXEL_WIDTH };
		if (SDL_HasIntersection(&position, &cameraView)) {
			ground->Render(pos, ratio, nullptr, 1);
		}
	}

	if (player.getOnGround()) {
		player.render(pos, ratio);
	}

	for (const auto& entity : entities) {
		entity->Render(pos, ratio);
	}

	for (const Ground* ground : solidTileRender) {
		SDL_Point gridPos = ground->getPosition();
		SDL_Rect position{ static_cast<int>(gridPos.x * GROUND_PIXEL_WIDTH), static_cast<int>(gridPos.y * GROUND_PIXEL_WIDTH), GROUND_PIXEL_WIDTH, GROUND_PIXEL_WIDTH };
		if (SDL_HasIntersection(&position, &cameraView)) {
			ground->Render(pos, ratio, nullptr, 0);
		}
	}

	if (!player.getOnGround()) {
		player.render(pos, ratio);
	}
}

void Act::loadNextAct(std::list<Act>& acts, std::vector<std::string>& actPaths, int& current, std::vector < Ground::groundArrayData >& arrayData, std::vector<std::vector<Animation>>& background) {
	current++;
	int actNum;
	std::string actName;
	std::vector < PhysStruct > entities;
	SDL_Rect winArea;
	ActType actType;
	std::vector < Ground > ground;
	SDL_Point levelSize;
	DataReader::LoadActData(actPaths[current], actNum, actName, entities, winArea, actType, ground, levelSize);
	acts.emplace_back(actNum, actName, entities, winArea, actType, globalObjects::ratio, levelSize, background, ground);
	acts.pop_front();
}
