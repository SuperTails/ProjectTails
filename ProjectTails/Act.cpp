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
		solidTiles[floor(pos.x / 256.0)][floor(pos.y / 256.0)] = std::move(ground[i]);
		solidTileRender.push_back(&solidTiles[floor(pos.x / 256.0)][floor(pos.y / 256.0)]);
	}
	
}

void Act::Init() {
	std::cout << "Initializing act " << number << "!\n";
	std::cout << "Loading entities...\n";
	for (std::vector<PhysStruct>::iterator i(phys_paths.begin()); i != phys_paths.end(); i++) {
		if (SDL_HasIntersection(&(i->pos), &(cam->getCollisionRect()))) {
			std::cout << "Loading entity with number " << i->num << " at position " << i->pos.x << " " << i->pos.y << "\n";
			loaded_entities++;
			entities.emplace_back(new PhysicsEntity(*i));
			i->loaded = true;
		}
	}
	PhysicsEntity::setEntityList(&entities);
	std::cout << "Entity loading done!\n";
	std::cout << "Done initializing act!\n";
}

void Act::Unload() {
	solidTiles.clear();
	background.clear();
	phys_paths.clear();
	entities.clear();
}

//Unload offscreen entities and update onscreen ones
void Act::UpdateEntities(Player& player) {
	last_time = time;
	time = SDL_GetTicks();
	Animation::setTimeDifference(time - last_time);
	Player::entityPtrType& playerPlatform = player.platformPtr();
	Player::entityPtrType& playerWall = player.wallPtr();
	SDL_Point platformPos = playerPlatform ? PhysicsEntity::xyPoint((*playerPlatform)->getPosition()) : SDL_Point{ -1, -1 };
	SDL_Rect wallPos = playerWall ? (*playerWall)->getPosition() : SDL_Rect{ -1, -1, -1, -1 };
	entityListIterator j = entities.begin();
 	while(j != entities.end()){
		bool shouldDestroy = (*j)->Update(true, &player, &entities, &j);
		if (shouldDestroy) {
			std::cout << "Destroying entity with number " << (*j)->num << " at position " << (*j)->getPosition().x << " " << (*j)->getPosition().y << "\n";
			if ((*j)->num != -1) {
				phys_paths[(*j)->num].loaded = false;
				phys_paths[(*j)->num].num = -1;
			}
			j = entities.erase(j);
			continue;
		}
		
		(*j)->loaded = true;
		if (!SDL_HasIntersection(&((*j)->getPosition()), &(cam->getCollisionRect())) && (*j)->loaded && (playerPlatform == nullptr || *j != *playerPlatform)) {
			std::cout << "Unloading entity with number " << (*j)->num << " at position " << (*j)->getPosition().x << " " << (*j)->getPosition().y << "\n";
			if ((*j)->num != -1) {
				phys_paths[(*j)->num].pos = (*j)->getPosition();
				phys_paths[(*j)->num].loaded = false;
			}
			loaded_entities--;
			j = entities.erase(j);
		}
		else {
			j++;
		}
	}
	//Load new entities
	for (entityLoadIterator i = phys_paths.begin(); i < phys_paths.end(); i++) {
		if (SDL_HasIntersection(&(i->pos), &(cam->getCollisionRect())) && i->loaded == false && i->num != -1) {
			std::cout << "Loading entity with number " << i->num << " at position " << i->pos.x << " " << i->pos.y << "\n";
			entities.emplace_back(new PhysicsEntity(*i));
			entities.back()->SetTime(SDL_GetTicks());
			entities.back()->loaded = true;
			entities.back()->num = std::distance(phys_paths.begin(), i);
			i->loaded = true;
			loaded_entities++;
		}
	}
	entities.shrink_to_fit();

	if (playerPlatform) {
		j = entities.begin();
		while (j != entities.end()) {
			if (PhysicsEntity::xyPoint((*j)->getPosition()) == platformPos) {
				playerPlatform = &(*j);
				break;
			}
			++j;
		}
		assert(j != entities.end());
	}
	if (playerWall) {
		j = entities.begin();
		while (j != entities.end()) {
			if ((*j)->getPosition() == wallPos) {
				playerWall = &(*j);
				break;
			}
			++j;
		}
	}
}

Act::~Act()
{
}

void Act::SetRenderer(SDL_Renderer* r) {
	renderer = r;
	/*for (int i = 0; i < animation_paths.size(); i++) {
		animations[i].SetRenderer(r);
	}*/
}

void Act::SetCamera(Camera* c) {
	cam = c;
}

void Act::RenderObjects(Player& player) {
	globalObjects::renderBackground(background, cam->getPosition().x - cam->GetOffset().x, ratio);
	entityListIterator i = entities.begin();
	SDL_Rect pos = cam->getPosition();
	while (i != entities.end()) {
		(*i)->Render(pos, ratio);
		i++;
	}
	solidRenderListIterator solidLayer = solidTileRender.begin();
	while (solidLayer != solidTileRender.end()) {
		if(SDL_HasIntersection(&(*solidLayer)->getPosition(), &cam->getCollisionRect()))
			(*solidLayer)->Render(pos, ratio, nullptr, 1);
		solidLayer++;
	}
	player.Render(pos, ratio);
	solidLayer = solidTileRender.begin();
	while (solidLayer != solidTileRender.end()) {
		if (SDL_HasIntersection(&(*solidLayer)->getPosition(), &cam->getCollisionRect()))
			(*solidLayer)->Render(pos, ratio, nullptr, 0);
		solidLayer++;
	}
}

void Act::UpdateCollisions(Player* player) {
	bool destroyed = false;
	bool hurt = false;
	entityListIterator i = entities.begin();
	SDL_Point center{ -1, -1 };
	while  ( i != entities.end() ) {
		destroyed = false;
		EntType collideType;
		SDL_Rect playerCollide = player->getCollisionRect();
		SDL_Rect objCollide = (*i)->getCollisionRect();
		if (SDL_HasIntersection(&playerCollide, &objCollide) && (*i)->canCollide) {
			debounce = false;
			collideType = (*i)->GetType();
			switch ((*i)->GetType()) {
			case ENEMY:
				hurt = true;
				std::cout << "Collision type was with enemy\n";
				std::cout << "Entity's number is " << (*i)->num << "\n";
				center.x = (*i)->getPosition().x + (*i)->getCollisionRect().x + (*i)->getCollisionRect().w / 2;
				center.y = (*i)->getPosition().y + (*i)->getCollisionRect().y + (*i)->getCollisionRect().h / 2;
				(*i)->Destroy(ratio);
				i = player->hitEnemy(entities, i, phys_paths, props, globalObjects::window, center);
				destroyed = true;
				break;
			case RING:
				if (player->AddRing()) {
					std::cout << "Collision type was with ring\n";
					std::cout << "Entity's number is " << (*i)->num << "\n";
					(*i)->Destroy(ratio);
					destroyed = true;
				}
				break;
			case PATHSWITCH:
				if (!(*i)->getCustom(1)) {
					std::cout << "Collision type was with pathswitch\n";
					std::cout << "Entity's number is " << (*i)->num << "\n";
					switch ((*i)->getCustom(0)) {
					case 'i':
						player->switchPath();
						break;
					case 's':
						player->setPath(1);
						break;
					case 'u':
						player->setPath(0);
						break;
					default:
						throw "Pathswitch has no set flag!";
					}
					destroyed = false;
					(*i)->setCustom(1, static_cast <double>(true));
					debounce = true;
				}
				break;
			case SPRING:
				std::cout << "Collision type was with spring\n";
				SDL_Point dir;
				switch (static_cast <char>((*i)->getCustom(0))) {
				case 'u':
					player->velocity.y = -10;
					player->setOnGround(false);
					player->setCorkscrew(true);
					break;
				case 'd':
					player->velocity.y = 10;
					player->setOnGround(false);
					break;
				case 'l':
					player->velocity.x = -10;
					player->setGsp(-6.0);
					break;
				case 'r':
					player->velocity.x = 10;
					player->setGsp(6.0);
					break;
				default:
					throw "Spring has no direction flag!";
				}
				player->setJumping(false);
				player->setRolling(false);
				player->setControlLock(48);
				player->setFlying(-1.0);
				(*i)->setCustom(1, 100.0);
				break;
			case PLATFORM:
				player->prevOnPlatform = player->platformPtr() != nullptr;
				dir = player->calcRectDirection(objCollide);
				if (dir.y <= 0 && dir.y >= -20 - objCollide.h / 2 && dir.x >= -objCollide.w / 2 && dir.x <= objCollide.w / 2 && ((playerCollide.y + playerCollide.h) > objCollide.y)) {
					player->hitPlatform(static_cast<std::unique_ptr<PhysicsEntity>*>(&(*i)));
				}
				break;
			case SPIKES:
				dir = player->calcRectDirection((*i)->getPosition());

				if (dir.y < 0 && dir.x >= -(*i)->getCollisionRect().w / 2 && dir.x <= (*i)->getCollisionRect().w / 2)
					i = player->Damage(entities, phys_paths, props, i, (*i)->getPosition().x, globalObjects::window);
				else
					player->hitWall(&(*i));
				break;
			case MONITOR:
				if (player->canDamage()) {
					player->AddRing(10);
					player->setVelocity({ player->getVelocity().x, player->getVelocity().y * -0.9 });
					(*i)->Destroy(ratio);
					destroyed = true;
				}
				break;
			case GOALPOST:
				if (!(*i)->getCustom(1)) {
					(*i)->setCustom(0, 300.0);
					SoundHandler::actFinish();
				}
				break;
			default:
				throw "Invalid collision type.";
			}
			if (collideType == PATHSWITCH) {
				(*i)->setCustom(1, static_cast<double>(true));
			}
		}
		else if ((*i)->GetType() == PATHSWITCH) {
			(*i)->setCustom(1, static_cast <double> (false));
		}
		if (!destroyed) {
			i++;
		}
	}
}

void Act::loadNextAct(std::list<Act>& acts, std::vector<std::string>& actPaths, int& current, std::vector < Ground::groundArrayData >& arrayData, std::vector<std::vector<Animation>>& background) {
	if (!acts.empty()){
		acts.front().Unload();
	}
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