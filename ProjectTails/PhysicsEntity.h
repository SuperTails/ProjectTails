#pragma once
#include "prhsGameLib.h"
#include "EntityTypes.h"
#include "Functions.h"
#include "Animation.h"
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>

struct AnimStruct;
class Animation;

struct doublePoint {
	double x;
	double y;
	doublePoint(const std::pair< int, int >& point) : x(point.first), y(point.second) {};
	doublePoint(double x_, double y_) : x{ x_ }, y{ y_ } {};
	doublePoint() = default;
	doublePoint(const doublePoint&) = default;
	doublePoint(doublePoint&&) = default;
	doublePoint& operator= (const doublePoint&) = default;
};

struct PhysStruct {
	entity_property_data::EntityTypeId typeId;
	std::vector< char > flags;
	SDL_Rect position;
	bool temporary;
};

class Player;
struct EntityManager;

class PhysicsEntity : public PRHS_Entity
{
	std::string dataKey;

public:
	typedef const std::unique_ptr < PhysicsEntity >* entityPtrType;
	typedef std::vector < std::unique_ptr < PhysicsEntity > >* entityListPtr;
	typedef std::vector < std::unique_ptr < PhysicsEntity > >::iterator entityListIter;
	
	PhysicsEntity();
	PhysicsEntity(const PhysStruct& p);
	PhysicsEntity(const PhysicsEntity& arg);
	PhysicsEntity(PhysicsEntity&& other);
	PhysicsEntity(SDL_Point pos, bool multi, SDL_Point tileSize = { 16, 16 });
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

	SDL_Rect getCollisionRect() const;

	const std::string& getKey() const;

	std::size_t numAnims() { return animations.size(); };

	void setCollisionRect(SDL_Rect rect) { collisionRect = rect; };

	void setVelocity(doublePoint vel) { velocity = vel; };

	void setGravity(double g);

	void custom(Player& player, EntityManager& manager);

	entity_property_data::CustomData& getCustom() { return customData; };

	const entity_property_data::CustomData& getCustom() const { return customData; };

	template < typename T >
	T& getCustom() { return std::get<T>(getCustom()); };

	void customInit();

	const std::vector< char >& getFlags() const { return flags; };

	doublePoint getVelocity() const { return velocity; };

	bool isInvisible() { return invis; };

	SDL_Rect getPosition() const;

	doublePoint& getPosError() { return posError; };

	void setAnim(const std::vector< std::size_t >& a);

	void renderRaw(const Camera& camera) const;

	SDL_Point calcRectDirection(SDL_Rect& objCollide);

	PhysStruct toPhysStruct() const;

	friend void swap(PhysicsEntity& lhs, PhysicsEntity& rhs) noexcept;

	static SDL_Rect getRelativePos(const SDL_Rect& objPos, const SDL_Rect& camPos);

	doublePoint velocity;
	bool shouldSave;
	bool canCollide;

	static SDL_Point xyPoint(SDL_Rect rect) { return SDL_Point{ rect.x, rect.y }; };

	std::vector< std::size_t > currentAnim; 
	
	static entity_property_data::CustomData createCustomData(const PhysStruct& physStruct);
private:
	bool destroyAfterLoop;

protected:
	entity_property_data::CustomData customData;
	std::vector < std::unique_ptr < Animation > > animations;
	SDL_Rect collisionRect{ 0, 0, 0, 0 };
	std::vector< char > flags;
	bool invis = false;
	double gravity = 0.0;
	doublePoint posError = { 0.0, 0.0 };
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
