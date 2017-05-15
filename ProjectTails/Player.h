#pragma once
#include "PhysicsEntity.h"
#include "InputComponent.h"
#include "Ground.h"
#include <cmath>
#include <algorithm>
#include "Camera.h"
#include "Miscellaneous.h"

enum Mode { GROUND, LEFT_WALL, CEILING, RIGHT_WALL };

class Player: public PhysicsEntity
{
public:
	Player();

	typedef PhysicsEntity::entityPtrType entityPtrType;

	int GetRings() { return rings; };
	void SetRings(int r) { rings = r; };
	void UpdateP(Camera&);
	std::vector<std::unique_ptr<PhysicsEntity>>::iterator Damage(std::vector < std::unique_ptr < PhysicsEntity > >& entities, std::vector < PhysStruct >& phys_paths, std::vector < PhysProp >& props, std::vector<std::unique_ptr<PhysicsEntity>>::iterator& iter, int enemyX, SDL_Window* window);
	void SetActType(int aType);
	void Render(SDL_Rect& cam, double screenRatio);
	bool grounded() { return onGround; };
	bool isRolling() { return rolling; };
	void setJumping(bool j) { jumping = j; };
	void setControlLock(int c) { controlLock = c; };
	void switchPath() { path = !path; };
	void setPath(int p) { path = p; };
	void setRolling(bool r) { rolling = r; };
	void setFlying(double f) { flying = f; };
	void setCorkscrew(bool c) { corkscrew = c; };
	void hitPlatform(const entityPtrType entity);
	void hitWall(const entityPtrType entity);
	void setActCleared(bool b);
	bool canDamage();
	entityPtrType& platformPtr() { return platform; };
	entityPtrType& wallPtr() { return wall; };
	int lookDirection() { return looking; };
	Mode getMode() { return collideMode; };
	std::string CollideGround(std::vector < std::vector < Ground > >& tiles);
	int checkSensor(char sensor, std::vector < std::vector < Ground > >& tiles, double& ang);
	SDL_RendererFlip getFlip() { return SDL_RendererFlip(horizFlip); };
	double getAngle() { return hexToDeg(angle); };
	double getGsp() { return gsp; };
	bool AddRing(int num = 1);
	int GetDamageCountdown() { return damageCountdown; };
	SDL_Rect getCollisionRect();
	static std::string modeToString(Mode m);
	void setOnGround(bool g) { onGround = g; };
	void setGsp(double g) { gsp = g; };
	std::vector<std::unique_ptr<PhysicsEntity>>::iterator hitEnemy(std::vector < std::unique_ptr < PhysicsEntity > >& entities, std::vector<std::unique_ptr<PhysicsEntity>>::iterator currentEntity, std::vector < PhysStruct >& phys_paths, std::vector < PhysProp >& props, SDL_Window* window, SDL_Point enemyCenter);

	bool prevOnPlatform;

private:
	int rings;
	int damageCountdown;
	int actType;
	int controlLock;
	int looking;

	bool onGround;
	bool onGroundPrev;
	bool jumping;
	bool spinDebounce;
	bool rolling;
	bool corkscrew;
	bool ceilingBlocked;
	bool actCleared;
	
	entityPtrType platform;
	entityPtrType wall;

	double accel;
	double decel;
	double gsp;
	double frc;
	double top;
	double slp;
	double spindash;
	double flying;

	double angle;
	int displayAngle;

	SDL_Point centerOffset;

	static double inline hexToDeg(double hex) { return (256 - hex) * 1.40625; };
	static int inline signum(int a) { return (0 < a) - (a < 0); };
	static int inline signum(double a) { return (0.0 < a) - (a < 0.0); };

	double inline getAngleSupp(double hex) { return 128 - hex; };

	bool horizFlip;

	Mode collideMode;

	double jmp;
	
	bool path;

	static int getHeight(std::vector < std::vector < Ground > >& ground, SDL_Point block, SDL_Point tile, bool side, bool pathC, int xStart, int xEnd, int yStart, int yEnd, bool& flip);

};

