#include "stdafx.h"
#include "PhysicsEntity.h"
#include "Player.h"
#include "Functions.h"
#include "Animation.h"
#include "Timer.h"
#include <cmath>
#include <algorithm>

PhysicsEntity::PhysicsEntity()
{
	position = { 0,0,0,0 };
	velocity = { 0,0 };
	self = nullptr;
	destroyAfterLoop = false;
}

PhysicsEntity::PhysicsEntity(PhysicsEntity&& other) :
	PhysicsEntity()
{
	swap(*this, other);
#if _DEBUG
	std::cout << "PhysicsEntity move constructor was called.\n";
#endif
}

PhysicsEntity::PhysicsEntity(const PhysicsEntity& arg) :
	invis(arg.invis),
	posError(arg.posError),
	dataKey(arg.dataKey),
	velocity(arg.velocity),
	shouldSave(arg.shouldSave),
	currentAnim(arg.currentAnim),
	collisionRect(arg.collisionRect),
	canCollide(true),
	destroyAfterLoop(arg.destroyAfterLoop),
	gravity(arg.gravity),
	flags(arg.flags),
	customData(arg.customData),
	self(nullptr)
{
	for (const auto& animation : arg.animations) {
		animations.emplace_back(std::make_unique<Animation>(*animation));
	}

	position = arg.position;
	previousPosition = arg.previousPosition;
};

PhysicsEntity::PhysicsEntity(const PhysStruct& p) :
	invis(false),
	shouldSave(!p.temporary),
	currentAnim(0),
	canCollide(true),
	destroyAfterLoop(false),
	self(nullptr),
	flags(p.flags),
	posError{ 0.0, 0.0 },
	dataKey(p.typeId),
	collisionRect(entity_property_data::getEntityTypeData(dataKey).collisionRect),
	customData(createCustomData(p))
{
	const auto& entityTypeData = entity_property_data::getEntityTypeData(dataKey);
	for (const auto& animData : entityTypeData.animationTypes) {
		animations.emplace_back(std::make_unique<Animation>(animData));
		animations.back()->stop();
	}

	if (!animations.empty()) {
		animations[currentAnim % animations.size()]->start();
	}

	position.w = collisionRect.w;
	position.h = collisionRect.h;

	previousPosition = convertRect(p.position);
	position = convertRect(p.position);
}

PhysicsEntity::PhysicsEntity(SDL_Rect pos, bool multi, SDL_Point tileSize) :
	self(nullptr),
	velocity{ 0.0, 0.0 },
	posError{ 0.0, 0.0 },
	currentAnim(0)
{
	position = convertRect(pos);
	animations.emplace_back(new Animation(tileSize));
	if (multi)
		animations.emplace_back(new Animation(tileSize));
};
 
void PhysicsEntity::update(Player* player, EntityManager* manager) {
	previousPosition = position;

	const double frameTime = Timer::getFrameTime().count();

	posError.x += frameTime * velocity.x / (1000.0 / 60.0);
	posError.y += frameTime * velocity.y / (1000.0 / 60.0);

	position.x += floor(posError.x);
	position.y += floor(posError.y);

	posError.x -= floor(posError.x);
	posError.y -= floor(posError.y);

	velocity.y += gravity * frameTime / (1000.0 / 60.0);

	if (player != nullptr && manager != nullptr ) {
		custom(*player, *manager);
	}

	applyToCurrentAnimations([](auto& animation) { animation->Update(); });
	if (!animations.empty() && animations[currentAnim % animations.size()]->GetLooped() && destroyAfterLoop && manager != nullptr) {
		manager->MarkAsDestroyed(this);
	}
}

std::size_t PhysicsEntity::currentAnimIndex() const noexcept {
	return currentAnim;
}

PhysicsEntity::entityPtrType& PhysicsEntity::platformPtr() {
	return self;
}

void PhysicsEntity::AddAnim(AnimStruct n) {
	animations.emplace_back(new Animation(n));
	animations.back()->stop();
}

void PhysicsEntity::Render(SDL_Rect& camPos, double ratio) {
	renderWithDefault(*this, camPos);
}

SDL_Rect PhysicsEntity::getRelativePos(const SDL_Rect& p) const {
	return getRelativePos(getPosition(), p);
}

SDL_Rect PhysicsEntity::getRelativePos(const SDL_Rect& a, const SDL_Rect& b) {
	return { a.x - b.x, a.y - b.y, a.w, a.h };
}

void PhysicsEntity::destroy() {
	velocity.x = 0;
	velocity.y = 0;
	canCollide = false;
	if (currentAnim != animations.size() - 1) {
		setAnim(animations.size() - 1);
		animations[currentAnim]->SetFrame(0);
		animations[currentAnim]->start();
		destroyAfterLoop = true;
	}
}

SDL_Rect PhysicsEntity::getPosition() const {
	return SDL_Rect{ position.x, position.y,
		std::max((!animations.empty()) ? animations[currentAnim % animations.size()]->GetSize().x : 0, collisionRect.w),
		std::max((!animations.empty()) ? animations[currentAnim % animations.size()]->GetSize().y : 0, collisionRect.h) };
}

void PhysicsEntity::setAnim(int a) {
	animations[currentAnim % animations.size()]->stop();
	currentAnim = a;
	animations[currentAnim % animations.size()]->start();
}

void PhysicsEntity::renderRaw(const SDL_Rect& cameraPosition) const {
	SDL_Point relativePosition = getXY(getRelativePos(cameraPosition));
	applyToCurrentAnimations([&](const auto& animation) {
		animation->Render(relativePosition, 0, nullptr, 1.0 / globalObjects::ratio);
	});
}

SDL_Rect PhysicsEntity::getCollisionRect() const {
	return SDL_Rect{ position.x + collisionRect.x, position.y + collisionRect.y, collisionRect.w, collisionRect.h };
}

const std::string& PhysicsEntity::getKey() const {
	return dataKey;
}

std::unique_ptr < Animation >& PhysicsEntity::getAnim(int index) {
	return animations[index];
}

const std::unique_ptr < Animation >& PhysicsEntity::getAnim(int index) const {
	return animations[index];
}

std::unique_ptr < Animation >& PhysicsEntity::getAnim() {
	return animations[currentAnim % animations.size()];
}

const std::unique_ptr < Animation >& PhysicsEntity::getAnim() const {
	return animations[currentAnim % animations.size()];
}

void PhysicsEntity::custom(Player& player, EntityManager& manager) {
	auto updateAny = [&](auto& data) {
		data.update(*this, manager, player);
	};
	std::visit(updateAny, customData);
}

void PhysicsEntity::setGravity(double g) {
	gravity = g;
}

entity_property_data::CustomData PhysicsEntity::createCustomData(const PhysStruct& physStruct) {
	using namespace entity_property_data;
	Keys entityDataKey = getEntityTypeData(physStruct.typeId).behaviorKey;
	return entity_property_data::createCustomFromKey(entityDataKey, physStruct.flags);
}

//Returns direction of THIS entity relative to objCollide
SDL_Point PhysicsEntity::calcRectDirection(SDL_Rect& objCollide) {
	return ::calcRectDirection(getCollisionRect(), objCollide);
}

PhysStruct PhysicsEntity::toPhysStruct() const {
	return PhysStruct{ getKey(), getFlags(), getPosition(), shouldSave };
}

void swap(PhysicsEntity& lhs, PhysicsEntity& rhs) noexcept {
	using std::swap;

	swap(static_cast<PRHS_Entity&>(lhs), static_cast<PRHS_Entity&>(rhs));

	swap(lhs.velocity, rhs.velocity);
	swap(lhs.shouldSave, rhs.shouldSave);
	swap(lhs.canCollide, rhs.canCollide);
	swap(lhs.self, rhs.self);
	swap(lhs.dataKey, rhs.dataKey);
	swap(lhs.destroyAfterLoop, rhs.destroyAfterLoop);
	swap(lhs.customData, rhs.customData);
	swap(lhs.animations, rhs.animations);
	swap(lhs.collisionRect, rhs.collisionRect);
	swap(lhs.currentAnim, rhs.currentAnim);
	swap(lhs.invis, rhs.invis);
	swap(lhs.gravity, rhs.gravity);
	swap(lhs.posError, rhs.posError);
}

SDL_Point getCenterDifference(const SDL_Rect& r1, const SDL_Rect& r2) {
	return SDL_Point { (r1.x + r1.w / 2) - (r2.x + r2.w / 2), (r1.y + r1.h / 2) - (r2.y + r2.h / 2) };
}

SDL_Point calcRectDirection(const SDL_Rect& r1, const SDL_Rect& r2) {
	const SDL_Rect& thisCollide = r1; 
	const SDL_Rect& objCollide = r2;

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

bool canBeStoodOn(const PhysicsEntity& entity) {
	using namespace entity_property_data;
	const auto& entityType = entity.getKey();
	return (canBePushedAgainst(entity) || entityType == "PLATFORM");
}

bool canBePushedAgainst(const PhysicsEntity& entity) {
	using namespace entity_property_data;
	const auto& entityType = entity.getKey();
	return (entityType == "MONITOR" || entityType == "SPIKES" || entityType == "SPRING");
}

Side getCollisionSide(const SDL_Rect& r1, const SDL_Rect& r2) {
	SDL_Point rectDirection = calcRectDirection(r1, r2);
	if (rectDirection.x == 0 && rectDirection.y == 0) {
		return Side::MIDDLE;
	}
	else if (abs(rectDirection.x) > abs(rectDirection.y)) {
		return ((rectDirection.x < 0) ? Side::LEFT : Side::RIGHT);
	}
	else {
		return ((rectDirection.y < 0) ? Side::TOP : Side::BOTTOM);
	}
}

void EntityManager::MarkAsDestroyed(const PhysicsEntity* entity) {
	const auto& toMark = std::find_if(entityList.begin(), entityList.end(), [&entity](const auto& obj) { return entity == obj.get(); });

	if (toMark == entityList.end()) {
		throw std::logic_error("Attempt to mark invalid entity");
	}

	toDestroy[std::distance(entityList.begin(), toMark)] = true;
}

void EntityManager::AddEntity(std::unique_ptr < PhysicsEntity >&& entity) {
	toAdd.emplace_back(std::move(entity));
}

void renderWithDefault(const PhysicsEntity& entity, const SDL_Rect& cameraPosition) {
	using namespace entity_property_data;
	auto keyValue = static_cast< std::size_t >(getEntityTypeData(entity.getKey()).behaviorKey);
	helpers::getOne<int>([&](auto& p) { renderWithDefaultImpl(entity, cameraPosition, p); return int{}; }, keyValue);
}
