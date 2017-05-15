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
	int num;
	bool loaded;
	std::vector < char > flags;
	PhysStruct(SDL_Rect p, PhysProp pr, int n, std::vector < char > f) : pos(p), prop(pr), num(n), loaded(false), flags(f) {};
};
class PhysicsEntity : public PRHS_Entity
{
public:
	typedef const std::unique_ptr < PhysicsEntity >* entityPtrType;
	typedef std::vector < std::unique_ptr < PhysicsEntity > >* entityListPtr;
	typedef std::vector < std::unique_ptr < PhysicsEntity > >::iterator entityListIter;
	
	PhysicsEntity();
	PhysicsEntity(PhysStruct, SDL_Window*);
	PhysicsEntity(const PhysicsEntity& arg);
	PhysicsEntity(PhysicsEntity&& other);
	PhysicsEntity(SDL_Rect pos, SDL_Window* win, bool multi, SDL_Point tileSize = { 16, 16 });
	~PhysicsEntity();

	//Returns true if this entity needs to be destroyed
	bool Update(bool updateTime = true, PhysicsEntity* player = nullptr, entityListPtr entityList = nullptr, entityListIter* iter = nullptr);

	std::unique_ptr < Animation >& GetAnim(int index);
	std::unique_ptr < Animation >& GetAnim();
	void AddAnim(AnimStruct, SDL_Window*);

	void Render(SDL_Rect& camPos, double ratio, bool absolute = false);

	SDL_Renderer* getRenderer() { return renderer; };
	void SetTime(Uint32 t) { time = t; };
	EntType GetType() { return eType; };

	void Destroy(double ratio, SDL_Window* window);

	SDL_Rect GetRelativePos(const SDL_Rect& p) const;

	PhysProp GetProp() { return prop; };

	SDL_Rect getCollisionRect();

	int numAnims() { return animations.size(); };

	void setCollisionRect(SDL_Rect rect) { collisionRect = rect; };

	void setVelocity(doublePoint vel) { velocity = vel; };

	void setGravity(double g);

	void custom(PhysicsEntity* player, std::vector < double > args = std::vector < double >(), entityListPtr entityList = nullptr, entityListIter* iter = nullptr);

	void setCustom(int index, double value);

	char getCustom(int index) { return customVars[index]; };

	void customInit();

	doublePoint getVelocity() { return velocity; };

	bool isInvisible() { return invis; };

	SDL_Rect getPosition();

	doublePoint& getPosError() { return posError; };

	void setAnim(int a) { currentAnim = a; };

	SDL_Point calcRectDirection(SDL_Rect& objCollide);

	virtual entityPtrType& platformPtr();

	const std::string& getKey() { return prop.key; };

	static SDL_Rect getRelativePos(const SDL_Rect& objPos, const SDL_Rect& camPos);

	static void setEntityList(std::vector < std::unique_ptr < PhysicsEntity > >* actEntities) { actEntityList = actEntities; };

	bool loaded;
	doublePoint velocity;
	int num;
	std::vector < SDL_Point > anim_sizes;
	bool canCollide;

	static SDL_Point xyPoint(SDL_Rect rect) { return SDL_Point{ rect.x, rect.y }; };
	
	static std::unordered_map < std::string, PhysProp* >* physProps;

private:
	static std::vector < std::unique_ptr < PhysicsEntity > >* actEntityList;
	std::vector < AnimStruct > anim_data;
	entityPtrType self;
	PhysProp prop;
	EntType eType;
	bool destroyAfterLoop;
	
protected:
	std::vector < std::unique_ptr < Animation > > animations;
	SDL_Rect collisionRect;
	std::vector < double > customVars;
	Uint32 time;
	Uint32 last_time;
	SDL_Window* window;
	int currentAnim;
	bool invis;
	double gravity;
	doublePoint posError;
};