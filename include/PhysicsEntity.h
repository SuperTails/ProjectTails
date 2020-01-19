#pragma once
#include "prhsGameLib.h"
#include "EntityTypes.h"
#include "Functions.h"
#include "Animation.h"
#include "Hitbox.h"
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>

struct AnimStruct;
class Animation;
class Player;
class Camera;
struct EntityManager;

class PhysicsEntity
{
	std::string dataKey;

public:
	typedef const std::unique_ptr < PhysicsEntity >* entityPtrType;
	typedef std::vector < std::unique_ptr < PhysicsEntity > >* entityListPtr;
	typedef std::vector < std::unique_ptr < PhysicsEntity > >::iterator entityListIter;
	
	PhysicsEntity();
	PhysicsEntity(const PhysicsEntity& arg);
	PhysicsEntity(PhysicsEntity&& other);
	PhysicsEntity(entity_property_data::EntityTypeId typeId, std::vector< char > flags, Point position);
	PhysicsEntity(Point pos, bool multi, SDL_Point tileSize = { 16, 16 });
	virtual ~PhysicsEntity() = default;

	void update(Player* player = nullptr, EntityManager* manager = nullptr);

	std::unique_ptr < Animation >& getAnim(std::size_t index);
	const std::unique_ptr < Animation >& getAnim(std::size_t index) const;
	//std::unique_ptr < Animation >& getAnim();
	//const std::unique_ptr < Animation >& getAnim() const;

	template < typename... Args >
	void AddAnim(const Args&...);

	void Render(const Camera& camera);

	void destroy();

	SDL_Rect getRelativePos(const SDL_Rect& p) const;

	const std::string& getKey() const;

	std::size_t numAnims() { return animations.size(); };

	void setVelocity(Vector2 vel) { velocity = vel; };

	void setHitbox(HitboxForm box) { hitbox = box; };

	HitboxForm getHitbox() const { return hitbox; };

	AbsoluteHitbox getAbsHitbox() const { return AbsoluteHitbox(getHitbox(), getPosition()); };

	void setGravity(double g);

	void custom(Player& player, EntityManager& manager);

	entity_property_data::CustomData& getCustom() { return customData; };

	const entity_property_data::CustomData& getCustom() const { return customData; };

	template < typename T >
	T& getCustom() { return std::get<T>(getCustom()); };

	void customInit();

	const std::vector< char >& getFlags() const { return flags; };
	void setFlags(const std::vector< char >& f) { flags = f; };

	Vector2 getVelocity() const { return velocity; };

	Point getPosition() const;

	void setAnim(const std::vector< std::size_t >& a);

	void renderRaw(const Camera& camera) const;

	//SDL_Point calcRectDirection(SDL_Rect& objCollide);

	friend void swap(PhysicsEntity& lhs, PhysicsEntity& rhs) noexcept;

	static SDL_Rect getRelativePos(const SDL_Rect& objPos, const SDL_Rect& camPos);

	Vector2 velocity;
	bool canCollide;

	static SDL_Point xyPoint(SDL_Rect rect) { return SDL_Point{ rect.x, rect.y }; };

	std::vector< std::size_t > currentAnim; 
	
	// TODO: What is this again?
	// static entity_property_data::CustomData createCustomData(const PhysStruct& physStruct);
	
	entity_property_data::CustomData createCustomData(const std::string& typeId, const std::vector< char >& flags);

	Point position;

private:
	bool destroyAfterLoop;

protected:
	HitboxForm hitbox;

	entity_property_data::CustomData customData;
	std::vector < std::unique_ptr < Animation > > animations;
	std::vector< char > flags;
	double gravity = 0.0;
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

SDL_Point getCenterDifference(const SDL_Rect& r1, const SDL_Rect& r2);

SDL_Point calcRectDirection(const SDL_Rect& r1, const SDL_Rect& r2);

bool canBeStoodOn(const PhysicsEntity& entity);

bool canBePushedAgainst(const PhysicsEntity& entity);

enum class Side { RIGHT, BOTTOM, LEFT, TOP, MIDDLE };

Side getCollisionSide(const SDL_Rect& r1, const SDL_Rect& r2); 
