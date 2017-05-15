#include "stdafx.h"
#include "PhysicsEntity.h"

std::vector < std::unique_ptr < PhysicsEntity > >* PhysicsEntity::actEntityList = nullptr;

std::unordered_map < std::string, PhysProp* >* PhysicsEntity::physProps = nullptr;

PhysicsEntity::PhysicsEntity()
{
	position = { 0,0,0,0 };
	velocity = { 0,0 };
	self = nullptr;
}

PhysicsEntity::PhysicsEntity(PhysicsEntity&& other) :
	invis(std::move(other.invis)),
	anim_data(std::move(other.anim_data)),
	posError(std::move(other.posError)),
	prop(std::move(other.prop)),
	velocity(std::move(other.velocity)),
	loaded(std::move(other.loaded)),
	num(std::move(other.num)),
	currentAnim(std::move(other.currentAnim)),
	eType(std::move(other.eType)),
	collisionRect(std::move(other.collisionRect)),
	gravity(std::move(other.gravity)),
	customVars(std::move(other.customVars)),
	self(nullptr),
	animations(std::move(other.animations)),
	PRHS_Entity(std::move(other))
{
	std::cout << "PhysicsEntity move constructor was called.\n";
}

PhysicsEntity::PhysicsEntity(const PhysicsEntity& arg) {
	invis = false;
	posError = arg.posError;
	window = arg.window;
	prop = arg.prop;
	for (int i = 0; i < arg.animations.size(); i++) {
		animations.emplace_back(new Animation(*arg.animations[i]));
	}
	position = arg.position;
	previousPosition = arg.previousPosition;
	velocity = arg.prop.vel;
	loaded = arg.loaded;
	num = arg.num;
	time = SDL_GetTicks();
	last_time = time;
	currentAnim = arg.currentAnim;
	eType = arg.prop.eType;
	collisionRect = arg.prop.collision;
	anim_data = arg.anim_data;
	canCollide = true;
	destroyAfterLoop = arg.destroyAfterLoop;
	gravity = arg.gravity;
	customVars = arg.customVars;
	self = nullptr;
};

PhysicsEntity::PhysicsEntity(PhysStruct p, SDL_Window* w) :
	invis(false),
	anim_data(p.prop.anim),
	window(w),
	prop(p.prop),
	velocity(p.prop.vel),
	loaded(false),
	num(p.num),
	time(SDL_GetTicks()),
	last_time(time),
	currentAnim(0),
	eType(p.prop.eType),
	collisionRect(p.prop.collision),
	canCollide(true),
	destroyAfterLoop(false),
	gravity(p.prop.gravity),
	self(nullptr)
{
	posError = { 0.0, 0.0 };
	for (int i = 0; i < anim_data.size(); i++) {
		animations.emplace_back(new Animation(anim_data[i], window));
	}
	position.w = collisionRect.w;
	position.h = collisionRect.h;

	previousPosition = convertRect(p.pos);
	position = convertRect(p.pos);
	for (int i = 0; i < p.flags.size(); i++) {
		customVars.push_back(p.flags[i]);
	}
	this->customInit();
}

PhysicsEntity::PhysicsEntity(SDL_Rect pos, SDL_Window* win, bool multi, SDL_Point tileSize) {
	self = nullptr;
	position = convertRect(pos);
	animations.emplace_back(new Animation({ 16, 16 }, win));
	if (multi)
		animations.emplace_back(new Animation({ 16, 16 }, win));
	currentAnim = 0;
	posError = { 0.0, 0.0 };
	velocity = { 0.0, 0.0 };
};

PhysicsEntity::~PhysicsEntity() {
	std::vector < std::unique_ptr < Animation > >::iterator i = animations.begin();
	while (i != animations.end()) {
		i = animations.erase(i);
	}
};

bool PhysicsEntity::Update(bool updateTime, PhysicsEntity* player, entityListPtr entityList, entityListIter* iter) {
	if (updateTime) {
		last_time = time;
		time = SDL_GetTicks();
	}

	previousPosition = position;


	posError.x += (time - last_time) * velocity.x / (1000.0 / 60.0);
	posError.y += (time - last_time) * velocity.y / (1000.0 / 60.0);

	position.x += floor(posError.x);
	position.y += floor(posError.y);

	posError.x -= floor(posError.x);
	posError.y -= floor(posError.y);

	velocity.y += gravity * (time - last_time) / (1000.0 / 60.0);
	
	this->custom(player, std::vector < double >(), entityList, iter);

	if (!animations.empty()) {
		int temp = currentAnim;
		do {
			animations[temp % animations.size()]->Update();
			temp /= animations.size();
		} while (temp != 0);
		if (animations[currentAnim % animations.size()]->GetLooped() && destroyAfterLoop) {
			return true;
		}
	}
	return false;
}

PhysicsEntity::entityPtrType& PhysicsEntity::platformPtr() {
	return self;
}

void PhysicsEntity::AddAnim(AnimStruct n, SDL_Window* w) {
	animations.emplace_back(new Animation(n, w));
}

SDL_Rect PhysicsEntity::GetRelativePos(const SDL_Rect& p) const {
	return { (position.x - p.x), (position.y - p.y), (position.w), (position.h) };
}

void PhysicsEntity::Destroy(double ratio, SDL_Window* window) {
	velocity.x = 0;
	velocity.y = 0;
	canCollide = false;
	currentAnim = animations.size() - 1;
	animations[currentAnim]->refreshTime();
	animations[currentAnim]->SetFrame(0);
	destroyAfterLoop = true;
}

SDL_Rect PhysicsEntity::getPosition() {
	return SDL_Rect{ position.x, position.y,
		std::max((!animations.empty()) ? animations[currentAnim % animations.size()]->GetSize().x : 0, collisionRect.w),
		std::max((!animations.empty()) ? animations[currentAnim % animations.size()]->GetSize().y : 0, collisionRect.h) };
}

SDL_Rect PhysicsEntity::getCollisionRect() {
	return SDL_Rect{ position.x + collisionRect.x, position.y + collisionRect.y, collisionRect.w, collisionRect.h };
}

std::unique_ptr < Animation >& PhysicsEntity::GetAnim(int index) {
	return animations[index];
}

std::unique_ptr < Animation >& PhysicsEntity::GetAnim() {
	return animations[currentAnim];
}

void PhysicsEntity::custom(PhysicsEntity* player, std::vector < double > args, entityListPtr entityList, entityListIter* iter) {
	const std::string& key = prop.key;
	double frameCount = (time - last_time) / (1000.0 / 60.0);
	int distance = -1;
	if(entityList)
		distance = std::distance(entityList->begin(), *iter);
	switch(eType){
	case RING:
		if (velocity.x != 0.0 || velocity.y != 0.0) {
			if (customVars[0] != -1) {
				customVars[0] -= std::max((double)(time - last_time), 0.0);
				animations[0]->SetDelay((int)(233.0 * (customVars[0] / 4267.0) + 33.0));
				if (customVars[0] == 0) {
					destroyAfterLoop = true;
				}
			}
		}
		break;
	case SPRING:
		if (customVars[1] >= (time - last_time)) {
			customVars[1] -= (time - last_time);
			currentAnim = 1;
		}
		else {
			customVars[1] = 0;
			currentAnim = 0;
		}
		break;
	case ENEMY:
		if (key == "BEEBADNIK") {
			/* vars:
			0 = time until moving again
			1 = already fired
			*/
			if (currentAnim == 0 || customVars[0] == 0.0) {
				velocity.x = -2.0;
currentAnim = 0;
if (player && player->getPosition().x != position.x && abs(double(position.y - player->getPosition().y) / (player->getPosition().x - position.x) - 1) <= 0.1) {
	currentAnim = 1;
	customVars[0] = 120.0;
	velocity.x = 0.0;
	customVars[1] = 0.0;
}

			}
			else {
				customVars[0] = std::max(0.0, customVars[0] - (time - last_time) / (1000.0 / 60.0));
				if (customVars[0] < 60.0 && !customVars[1]) {
					customVars[1] = 1.0;
					PhysStruct temp(SDL_Rect{ position.x + 30, position.y + 25, position.w, position.h }, *physProps->find(std::string("BEEPROJECTILE"))->second, -1, std::vector<char>());
					entityList->emplace(*iter, new PhysicsEntity(temp, globalObjects::window));
					*iter = entityList->begin() + distance;
				}
				velocity.x = 0;
			}
		}
		else if (key == "CRABBADNIK") {
			/* vars:
			0 = frames until walk, default 240.0
			1 = frames until stop, default 30.0
			2 = sign of direction
			*/
			if (customVars[0] > 0.0) {
				currentAnim = 0;
				customVars[0] -= frameCount;
				if (customVars[0] <= 0.0) {
					position.y += 7;
					customVars[0] = 0.0;
				}
			}
			else if (customVars[1] > 0.0) {
				currentAnim = 1;
				posError.x += customVars[2] * frameCount;
				customVars[1] -= frameCount;
				if (customVars[1] <= 0) {
					customVars[0] = 180.0;
					customVars[1] = 30.0;
					customVars[2] *= -1;
					position.y -= 7;
				}
			}
		}
		break;
	case PLATFORM:
		if (key == "BRIDGE") {
			entityPtrType platform = player->platformPtr();
			collisionRect.x = 0;
			collisionRect.y = 0;
			collisionRect.w = customVars[0] * 16;
			collisionRect.h = 16;
			if (platform) {
				//Smooth on/off timer
				customVars.back() = std::min(1.0, customVars.back() + ((time - last_time) / (1000.0 / 60.0)) / 30);

				//Player index
				customVars[1] = (player->getPosition().x - getPosition().x) / 16.0;
				collisionRect.x = std::max(customVars[1] * 16, 0.0);
				collisionRect.x = std::min(collisionRect.x, int(customVars[0] - 1) * 16);
				collisionRect.w = 16;
				std::vector < int > maxDepression(customVars[0], 2);
				for (int i = 0; i < maxDepression.size(); i++) {
					if (i < maxDepression.size() / 2)
						maxDepression[i] = 2 * (i + 1);
					else if (i > maxDepression.size() / 2)
						maxDepression[i] = 2 * (maxDepression.size() - i);
					else
						maxDepression[i] = maxDepression.size();
				}
				int thisMax = maxDepression[(int)std::min(customVars[1], double(customVars[0] - 1))];
				for (int i = 0; i < std::min(customVars[1] + 1, double(customVars[0] - 1)); i++) {
					customVars[i + 2] = customVars.back() * thisMax * sin(M_PI / 2 * (1 + i) / (1 + customVars[1]));
				}
				for (int i = customVars[1] + 1; i < customVars[0]; i++) {
					customVars[i + 2] = customVars.back() * thisMax * sin(M_PI / 2 * (customVars[0] - i) / (customVars[0] - customVars[1]));
				}
				if (customVars[1] + 2 < customVars.size())
					collisionRect.y = customVars[customVars[1] + 2];
			}
			else {
				customVars.back() = std::max(0.0, customVars.back() - ((time - last_time) / (1000.0 / 60.0)) / 120);
				customVars[1] = -1.0;
				for (int i = 2; i < customVars.size(); i++) {
					customVars[i] = customVars[i] * sqrt(customVars.back());
				}
			}
		}
		break;
	case GOALPOST:
		if (customVars[0] > 0.0) {
			customVars[0] -= frameCount;
			animations[0]->SetDelay(240.0 - customVars[0] / 1.5);
			customVars[1] = 1.0;
		}
		else {
			animations[0]->SetDelay(-1);
			if (customVars[1] && animations[0]->getFrame() != 4) {
				animations[0]->SetDelay(240.0);
			}
		}
		break;
	default:
		break;
	}
}

void PhysicsEntity::customInit() {
	const std::string& key = prop.key;
	switch(eType){
	case RING:
		//INIT_RING
		customVars.resize(1);
		customVars[0] = -1;
		break;
	case SPRING:
		//INIT_SPRING
		customVars.resize(2);
		customVars[1] = 0;
		break;
	case PATHSWITCH:
		//INIT_PATHSWITCH
		customVars.resize(2);
		customVars[1] = static_cast <double>(false);
		break;
	case ENEMY:
		if (key == "BEEBADNIK") {
			//INIT_BEE
			customVars.resize(2);
			customVars[0] = 0.0;
			customVars[1] = 0.0;
		}
		else if (key == "CRABBADNIK") {
			//INIT_CRAB
			customVars.resize(3);
			customVars[0] = 180.0;
			customVars[1] = 30.0;
			customVars[2] = 1.0;
		}
		break;
	case PLATFORM:
		if (key == "BRIDGE") {
			//INIT_BRIDGE
			customVars.resize(std::size_t(customVars[0] + 3), 0);
			customVars[1] = -1.0;
			customVars.back() = 0.0;
			position.w = customVars[0] * 16;
		}
		break;
	case GOALPOST:
		customVars.resize(2);
		animations[0]->SetDelay(-1);
		customVars[0] = 0.0;
		customVars[0] = 0.0;
		break;
	}
}

void PhysicsEntity::setGravity(double g) {
	gravity = g;
}

void PhysicsEntity::setCustom(int index, double value) {
	customVars[index] = value;
}

void PhysicsEntity::Render(SDL_Rect& camPos, double ratio, bool absolute) {
	if (absolute) {
		animations[currentAnim]->Render(&convertRect(position), 0, window, NULL, 1.0 / ratio);
		return;
	}
	if (!animations.empty()) {
		int rot = 0;
		SDL_Rect relativePos = GetRelativePos(camPos);
		std::string key = getKey();
		if (eType == SPRING) {
			switch (static_cast <char> (customVars[0])) {
			case 'u':
				relativePos.y -= (customVars[1] != 0) << 3;
				break;
			case 'd':
				rot = 180;
				relativePos.y += (customVars[1] != 0) << 3;
				break;
			case 'l':
				rot = -90;
				relativePos.x -= (customVars[1] != 0) << 3;
				break;
			case 'r':
				rot = 90;
				relativePos.x += (customVars[1] != 0) << 3;
				break;
			}
			animations[currentAnim]->Render(&relativePos, rot, window, NULL, 1.0 / ratio);
		}
		else if (key == "BRIDGE") {
			int xStart = relativePos.x;
			int yStart = relativePos.y;
			for (int& x = relativePos.x; x < customVars[0] * 16 + xStart; x += 16) {
				relativePos.y = yStart + customVars[(x - xStart) / 16 + 2];
				animations[currentAnim]->Render(&relativePos, rot, window, NULL, 1.0 / ratio);
			}
		}
		else {
			animations[currentAnim]->Render(&relativePos, rot, window, NULL, 1.0 / ratio);
		}
	}
}

SDL_Rect PhysicsEntity::getRelativePos(const SDL_Rect& objPos, const SDL_Rect& camPos) {
	return SDL_Rect{ objPos.x - camPos.x, objPos.y - camPos.y, objPos.w, objPos.h };
}

//Returns direction of THIS entity relative to objCollide
SDL_Point PhysicsEntity::calcRectDirection(SDL_Rect& objCollide) {
	SDL_Rect thisCollide = getCollisionRect();

	SDL_Point ret{ 0, 0 };

	if (thisCollide.x + thisCollide.w < objCollide.x + objCollide.w / 2) {
		//If centers do not overlap, return dist between right edge and center (negative)
		ret.x = -1 * (objCollide.x + objCollide.w / 2 - (thisCollide.x + thisCollide.w));
	}
	else if (thisCollide.x > objCollide.x + objCollide.w / 2) {
		//If centers do not overlap, return dist between left edge and center
		ret.x = thisCollide.x - (objCollide.x + objCollide.w / 2);
	}

	if (thisCollide.y + thisCollide.h < objCollide.y + objCollide.h / 2) {
		//Dist between bottom edge and center
		ret.y = -1 * (objCollide.y + objCollide.h / 2 - (thisCollide.y + thisCollide.h));
	}
	else if (thisCollide.y > objCollide.y + objCollide.h / 2) {
		ret.y = thisCollide.y - (objCollide.y + objCollide.h / 2);
	}

	return ret;
}