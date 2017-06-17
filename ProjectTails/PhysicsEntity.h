#pragma once
#include "prhsGameLib.h"
#include <vector>
#include <string>
#include "Animation.h"
#include <cmath>
#include <memory>
#include <algorithm>
#include <unordered_set>
#include <unordered_map>
struct doublePoint {
	double x;
	double y;
};
enum EntType { PLAYER, ENEMY, RING, WEAPON, PATHSWITCH, SPRING, PLATFORM, SPIKES, MONITOR, GOALPOST };
//Stores key, vel, anim, entity type, collisionRect, and gravity
struct PhysProp {
	std::string key;
	std::vector < AnimStruct > anim;
	doublePoint vel;
	SDL_Rect collision;
	double gravity;
	EntType eType;
};

struct PhysStructInit {
	SDL_Rect pos;
	std::vector < char > flags;
	std::string prop;
	PhysStructInit() : pos{ -1, -1, -1, -1 }, flags(), prop() {};
	PhysStructInit(const PhysStructInit& other) : pos(other.pos), flags(other.flags), prop(other.prop) {};
	PhysStructInit(SDL_Rect& p, std::vector < char >& f, std::string& pr) : pos(p), flags(f), prop(pr) {};
};

struct PhysStruct {
	SDL_Rect pos;
	PhysProp prop;
	bool shouldSave;
	std::vector < char > flags;
	PhysStruct(SDL_Rect p, PhysProp pr, bool s, std::vector < char > f) : pos(p), prop(pr), shouldSave(s), flags(f) {};
};

namespace entity_property_data {
	namespace keys {
		const char* BEE_BADNIK = "BEEBADNIK";
	}
	namespace indices {
		enum class BeeBadnik { FRAMES_UNTIL_MOVE, HAS_FIRED };
		enum class CrabBadnik { FRAMES_UNTIL_WALK, FRAMES_UNTIL_STOP, WALK_DIRECTION };
		enum class Bridge { WIDTH, PLAYER_POSITION, };
		enum class Goalpost { FRAMES_UNTIL_STOP, FINISHED_SPINNING };
	}
}

class Player;
struct EntityManager;

class PhysicsEntity : public PRHS_Entity
{
public:
	typedef const std::unique_ptr < PhysicsEntity >* entityPtrType;
	typedef std::vector < std::unique_ptr < PhysicsEntity > >* entityListPtr;
	typedef std::vector < std::unique_ptr < PhysicsEntity > >::iterator entityListIter;
	
	PhysicsEntity();
	PhysicsEntity(PhysStruct);
	PhysicsEntity(const PhysicsEntity& arg);
	PhysicsEntity(PhysicsEntity&& other);
	PhysicsEntity(SDL_Rect pos, bool multi, SDL_Point tileSize = { 16, 16 });
	virtual ~PhysicsEntity() = default;

	void update(bool updateTime = true, Player* player = nullptr, EntityManager* manager = nullptr);

	std::unique_ptr < Animation >& GetAnim(int index);
	std::unique_ptr < Animation >& GetAnim();
	void AddAnim(AnimStruct);

	void Render(SDL_Rect& camPos, double ratio, bool absolute = false);

	SDL_Renderer* getRenderer() const { return renderer; };
	void setTime(Uint32 t) { time = t; };
	EntType getType() const { return eType; };

	void destroy();

	SDL_Rect GetRelativePos(const SDL_Rect& p) const;

	PhysProp GetProp() { return prop; };

	SDL_Rect getCollisionRect();

	std::size_t numAnims() { return animations.size(); };

	void setCollisionRect(SDL_Rect rect) { collisionRect = rect; };

	void setVelocity(doublePoint vel) { velocity = vel; };

	void setGravity(double g);

	void custom(Player* player, EntityManager* manager);

	void setCustom(int index, double value);

	char getCustom(int index) { return static_cast<char>(customVars[index]); };

	void customInit();

	std::vector < char > getFlags() { return flags; };

	doublePoint getVelocity() { return velocity; };

	bool isInvisible() { return invis; };

	SDL_Rect getPosition();

	doublePoint& getPosError() { return posError; };

	void setAnim(int a) { currentAnim = a; };

	SDL_Point calcRectDirection(SDL_Rect& objCollide);

	virtual entityPtrType& platformPtr();

	const std::string& getKey() { return prop.key; };

	friend void swap(PhysicsEntity& lhs, PhysicsEntity& rhs) noexcept;

	static SDL_Rect getRelativePos(const SDL_Rect& objPos, const SDL_Rect& camPos);

	bool loaded;
	doublePoint velocity;
	bool shouldSave;
	bool canCollide;

	static SDL_Point xyPoint(SDL_Rect rect) { return SDL_Point{ rect.x, rect.y }; };
	
	static std::unordered_map < std::string, PhysProp* >* physProps;

private:
	std::vector < AnimStruct > anim_data;
	entityPtrType self;
	PhysProp prop;
	EntType eType;
	bool destroyAfterLoop;
	
protected:
	std::vector < std::unique_ptr < Animation > > animations;
	SDL_Rect collisionRect;
	std::vector < double > customVars;
	std::vector < char > flags;
	Uint32 time;
	Uint32 last_time;
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

	void MarkAsDestroyed(const PhysicsEntity* entity) {
		if (entityList.empty()) {
			throw "Attempt to mark invalid entity.";
		}

		const PhysicsEntity* firstEntity = &(*entityList.front());

		std::size_t distance = std::distance(firstEntity, entity);

		if (distance >= entityList.size()) {
			throw "Attempt to mark invalid entity.";
		}

		toDestroy[distance] = true;
	}

	void AddEntity(std::unique_ptr < PhysicsEntity >&& entity) {
		toAdd.emplace_back(std::move(entity));
	}
private:
	std::vector < std::unique_ptr < PhysicsEntity > >& entityList;
	std::vector < bool >& toDestroy;
	std::vector < std::unique_ptr < PhysicsEntity > >& toAdd;
};