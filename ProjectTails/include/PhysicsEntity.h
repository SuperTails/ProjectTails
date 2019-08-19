#pragma once
#include "prhsGameLib.h"
#include "EntityTypes.h"
#include "Functions.h"
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>

struct AnimStruct;
class Animation;

struct doublePoint {
	double x;
	double y;
};

struct PhysStruct {
	entity_property_data::EntityTypeId typeId;
	std::vector < char > flags;
	SDL_Rect position;
	bool temporary;
};

class Player;
struct EntityManager;

class PhysicsEntity : public PRHS_Entity
{
public:
	typedef const std::unique_ptr < PhysicsEntity >* entityPtrType;
	typedef std::vector < std::unique_ptr < PhysicsEntity > >* entityListPtr;
	typedef std::vector < std::unique_ptr < PhysicsEntity > >::iterator entityListIter;
	
	PhysicsEntity();
	PhysicsEntity(const PhysStruct& p);
	PhysicsEntity(const PhysicsEntity& arg);
	PhysicsEntity(PhysicsEntity&& other);
	PhysicsEntity(SDL_Rect pos, bool multi, SDL_Point tileSize = { 16, 16 });
	virtual ~PhysicsEntity() = default;

	void update(Player* player = nullptr, EntityManager* manager = nullptr);

	std::unique_ptr < Animation >& getAnim(int index);
	const std::unique_ptr < Animation >& getAnim(int index) const;
	std::unique_ptr < Animation >& getAnim();
	const std::unique_ptr < Animation >& getAnim() const;
	std::size_t currentAnimIndex() const noexcept;
	void AddAnim(AnimStruct);

	void Render(SDL_Rect& camPos, double ratio);

	void destroy();

	SDL_Rect getRelativePos(const SDL_Rect& p) const;

	SDL_Rect getCollisionRect() const;

	const std::string& getKey() const;

	std::size_t numAnims() { return animations.size(); };

	void setCollisionRect(SDL_Rect rect) { collisionRect = rect; };

	void setVelocity(doublePoint vel) { velocity = vel; };

	void setGravity(double g);

	void custom(Player& player, EntityManager& manager);

	entity_property_data::CustomData& getCustom() { return customData; };

	void customInit();

	const std::vector < char >& getFlags() const { return flags; };

	doublePoint getVelocity() { return velocity; };

	bool isInvisible() { return invis; };

	SDL_Rect getPosition() const;

	doublePoint& getPosError() { return posError; };

	void setAnim(int a);

	void renderRaw(const SDL_Rect& cameraPosition) const;

	SDL_Point calcRectDirection(SDL_Rect& objCollide);

	virtual entityPtrType& platformPtr();

	PhysStruct toPhysStruct() const;

	friend void swap(PhysicsEntity& lhs, PhysicsEntity& rhs) noexcept;

	static SDL_Rect getRelativePos(const SDL_Rect& objPos, const SDL_Rect& camPos);

	doublePoint velocity;
	bool shouldSave;
	bool canCollide;

	static SDL_Point xyPoint(SDL_Rect rect) { return SDL_Point{ rect.x, rect.y }; };
	
	template < typename F >
	auto applyToCurrentAnimations(F&& f) const -> 
	typename std::enable_if_t< decltype(entity_property_data::isConstAnimationFunction(f))::value, void > {
		if (animations.empty()) {
			return;
		}
		int temp = currentAnim;
		do {
			f(animations[temp % animations.size()]);
			temp /= animations.size();
		} while (temp != 0);
	}

	template < typename F >
	auto applyToCurrentAnimations(F&& f) ->
	typename std::enable_if_t< !decltype(entity_property_data::isConstAnimationFunction(f))::value, void > {
		if (animations.empty()) {
			return;
		}
		int temp = currentAnim;
		do {
			f(animations[temp % animations.size()]);
			temp /= animations.size();
		} while (temp != 0);
	}
	
	static entity_property_data::CustomData createCustomData(const PhysStruct& physStruct);
private:

	entityPtrType self;
	std::string dataKey;
	bool destroyAfterLoop;

protected:
	entity_property_data::CustomData customData;
	std::vector < std::unique_ptr < Animation > > animations;
	SDL_Rect collisionRect;
	std::vector < char > flags;
	int currentAnim;
	bool invis;
	double gravity;
	doublePoint posError;
};

struct EntityManager {
	EntityManager() = delete;
	EntityManager(const EntityManager&) = delete;
	EntityManager(EntityManager&&) = delete;
	EntityManager(std::vector < std::unique_ptr < PhysicsEntity > >& entities, std::vector < bool >& destroy, std::vector < std::unique_ptr < PhysicsEntity > >& add) :
		entityList(entities),
		toDestroy(destroy),
		toAdd(add)
	{};

	void MarkAsDestroyed(const PhysicsEntity* entity);

	void AddEntity(std::unique_ptr < PhysicsEntity >&& entity);
private:
	std::vector < std::unique_ptr < PhysicsEntity > >& entityList;
	std::vector < bool >& toDestroy;
	std::vector < std::unique_ptr < PhysicsEntity > >& toAdd;
};

template < typename Custom > auto renderWithDefaultImpl(const PhysicsEntity& entity, const SDL_Rect& cameraPosition, const Custom& custom)
-> typename std::enable_if_t<decltype(entity_property_data::hasRender(custom))::value, void> {
	custom.render(entity, cameraPosition);
}

template < typename Custom > auto renderWithDefaultImpl(const PhysicsEntity& entity, const SDL_Rect& cameraPosition, const Custom& custom)
-> typename std::enable_if_t<!decltype(entity_property_data::hasRender(custom))::value, void> {
	entity.renderRaw(cameraPosition);
}

void renderWithDefault(const PhysicsEntity& entity, const SDL_Rect& cameraPosition);

SDL_Point getCenterDifference(const SDL_Rect& r1, const SDL_Rect& r2);

SDL_Point calcRectDirection(const SDL_Rect& r1, const SDL_Rect& r2);

bool canBeStoodOn(const PhysicsEntity& entity);

bool canBePushedAgainst(const PhysicsEntity& entity);

enum class Side { RIGHT, BOTTOM, LEFT, TOP, MIDDLE };

Side getCollisionSide(const SDL_Rect& r1, const SDL_Rect& r2); 
