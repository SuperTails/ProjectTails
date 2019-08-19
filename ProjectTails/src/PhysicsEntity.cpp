#include "stdafx.h"
#include "PhysicsEntity.h"
#include "Player.h"
#include "Functions.h"
#include "Animation.h"
#include "Timer.h"
#include "Camera.h"
#include <cmath>
#include <algorithm>

void renderWithDefault(const PhysicsEntity& entity, const Camera& camera);

PhysicsEntity::PhysicsEntity()
{
	position = { 0  , 0   };
	velocity = { 0.0, 0.0 };
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
	customData(arg.customData)
{
	for (const auto& animation : arg.animations) {
		AddAnim(*animation);
	}

	position = arg.position;
	previousPosition = arg.previousPosition;
};

PhysicsEntity::PhysicsEntity(const PhysStruct& p) :
	invis(false),
	shouldSave(!p.temporary),
	canCollide(true),
	destroyAfterLoop(false),
	flags(p.flags),
	posError{ 0.0, 0.0 },
	dataKey(p.typeId),
	velocity(entity_property_data::getEntityTypeData(dataKey).defaultVelocity),
	gravity(entity_property_data::getEntityTypeData(dataKey).defaultGravity),
	collisionRect(entity_property_data::getEntityTypeData(dataKey).collisionRect),
	customData(createCustomData(p))
{
	const auto& entityTypeData = entity_property_data::getEntityTypeData(dataKey);
	for (const auto& animData : entityTypeData.animationTypes) {
		animations.emplace_back(std::make_unique< Animation >(animData));
		animations.back()->stop();
	}
	
	if (!animations.empty()) {
		currentAnim.push_back(0);
	}

	for (auto index : currentAnim) {
		animations[index]->start();
	}

	position = { p.position.x, p.position.y };
	previousPosition = position;
}

PhysicsEntity::PhysicsEntity(SDL_Point pos, bool multi, SDL_Point tileSize) :
	velocity{ 0.0, 0.0 },
	posError{ 0.0, 0.0 },
	currentAnim(0)
{
	position = pos;
	AddAnim(tileSize);
	if (multi) {
		AddAnim(tileSize);
	}
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

	if (player && manager) {
		custom(*player, *manager);
	}

	for (auto index : currentAnim) {
		animations[index]->Update();
	}
	
	if (destroyAfterLoop && manager) {
		for (auto index : currentAnim) {
			if (animations[index]->GetLooped()) {
				manager->MarkAsDestroyed(this);
			}
		}
	}
}

template < typename... Args >
void PhysicsEntity::AddAnim(const Args&... args) {
	animations.emplace_back(std::make_unique< Animation >(args...));
	animations.back()->stop();
}

void PhysicsEntity::Render(const Camera& camera) {
	renderWithDefault(*this, camera);
	if (globalObjects::debug) {
		SDL_Rect collision = (getCollisionRect() - camera.getPosition()) * camera.scale;
		SDL_SetRenderDrawColor(globalObjects::renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
		SDL_RenderDrawRect(globalObjects::renderer, &collision);
	}
}

SDL_Rect PhysicsEntity::getRelativePos(const SDL_Rect& p) const {
	return getRelativePos(getPosition(), p);
}

SDL_Rect PhysicsEntity::getRelativePos(const SDL_Rect& a, const SDL_Rect& b) {
	return { a.x - b.x, a.y - b.y, a.w, a.h };
}

void PhysicsEntity::destroy() {
	velocity = { 0.0, 0.0 };
	canCollide = false;
	if (currentAnim.size() != 1 || currentAnim.back() != animations.size() - 1) {
		setAnim({ animations.size() - 1 });
		animations[currentAnim.back()]->SetFrame(0);
		animations[currentAnim.back()]->start();
		destroyAfterLoop = true;
	}
}

SDL_Rect PhysicsEntity::getPosition() const {
	SDL_Point size{ 0, 0 };
	std::for_each(currentAnim.begin(), currentAnim.end(), [&](auto index) {
		size.x = std::max({ size.x, animations[index]->GetSize().x, collisionRect.w });	
		size.y = std::max({ size.y, animations[index]->GetSize().y, collisionRect.h });
	});
	return SDL_Rect{ position.x, position.y, size.x, size.y };
}

void PhysicsEntity::setAnim(const std::vector< std::size_t >& a) {
	for (auto index : currentAnim) {
		animations[index]->stop();
	}
	currentAnim = a;
	for (auto index : currentAnim) {
		animations[index]->start();
	}
}

void PhysicsEntity::renderRaw(const Camera& camera) const {
	const SDL_Point relativePosition = position - camera.getPosition();
	for (auto index : currentAnim) {
		animations[index]->Render(relativePosition, 0, nullptr, camera.scale);
	}
}

SDL_Rect PhysicsEntity::getCollisionRect() const {
	return SDL_Rect{ position.x + collisionRect.x, position.y + collisionRect.y, collisionRect.w, collisionRect.h };
}

const std::string& PhysicsEntity::getKey() const {
	return dataKey;
}

std::unique_ptr < Animation >& PhysicsEntity::getAnim(std::size_t index) {
	return animations[index];
}

const std::unique_ptr < Animation >& PhysicsEntity::getAnim(std::size_t index) const {
	return animations[index];
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
	Key entityDataKey = getEntityTypeData(physStruct.typeId).behaviorKey;
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
	const auto toMark = std::find_if(entityList.begin(), entityList.end(), [&](const auto& obj) { return entity == obj.get(); });

	if (toMark == entityList.end()) {
		throw std::invalid_argument("Attempt to mark invalid entity");
	}

	toDestroy[std::distance(entityList.begin(), toMark)] = true;
}

void EntityManager::AddEntity(std::unique_ptr< PhysicsEntity >&& entity) {
	toAdd.emplace_back(std::move(entity));
}

#ifdef __clang__
template < typename Custom > auto renderWithDefaultImpl(const PhysicsEntity& entity, const Camera& camera, const Custom& custom)
-> typename std::enable_if_t<decltype(entity_property_data::hasRender< Custom >)::value, void> {
	custom.render(entity, camera);
}

template < typename Custom > auto renderWithDefaultImpl(const PhysicsEntity& entity, const Camera& camera, const Custom& custom)
-> typename std::enable_if_t<!decltype(entity_property_data::hasRender< Custom >)::value, void> {
	entity.renderRaw(camera);
}
#else 
template < typename Custom >
void renderWithDefaultImpl(const PhysicsEntity& entity, const Camera& camera, const Custom& custom)
requires entity_property_data::Renderable< Custom > {
	custom.render(entity, camera);
}

template < typename Custom >
void renderWithDefaultImpl(const PhysicsEntity& entity, const Camera& camera, const Custom& custom) {
	entity.renderRaw(camera);
}
#endif

void renderWithDefault(const PhysicsEntity& entity, const Camera& camera) {
	using namespace entity_property_data;
	const std::size_t keyValue = entity.getCustom().index();
	helpers::getOne<int>([&](const auto& p) { renderWithDefaultImpl(entity, camera, p); return int{}; }, entity.getCustom(), keyValue);
}
