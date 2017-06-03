#pragma once
#include <cmath>
#include <algorithm>
#include "PhysicsEntity.h"
#include "InputComponent.h"
#include "Ground.h"
#include "Camera.h"
#include "Miscellaneous.h"
#include "Constants.h"

enum Mode { GROUND, LEFT_WALL, CEILING, RIGHT_WALL };

namespace player_constants {
	namespace animation_paths {
		const std::string TORNADO_PATH = ASSET"Tornado.png";
		const std::string IDLE_PATH = ASSET"Tails_Idle.png";
		const std::string WALK_PATH = ASSET"Tails_Walk.png";
		const std::string RUN_PATH = ASSET"Tails_Run.png";
		const std::string ROLL_BODY_PATH = ASSET"Tails_Roll_Body.png";
		const std::string ROLL_TAILS_PATH = ASSET"Tails_Roll_Tails.png";
		const std::string CROUCH_PATH = ASSET"Tails_Crouch.png";
		const std::string LOOK_UP_PATH = ASSET"Tails_Look_Up.png";
		const std::string SPINDASH_PATH = ASSET"Tails_Spindash.png";
		const std::string FLY_PATH = ASSET"Tails_Fly.png";
		const std::string FLY_TIRED_PATH = ASSET"Tails_Fly_Tired.png";
		const std::string CORKSCREW_PATH = ASSET"Tails_Corkscrew.png";
		const std::string HURT_PATH = ASSET"Tails_Hurt.png";
		const std::string ACT_CLEAR_PATH = ASSET"Tails_Act_Clear.png";
	}

	namespace physics {
		const double DEFAULT_ACCELERATION  = 0.046875;
		const double DEFAULT_DECCELERATION = 0.5;
		const double DEFAULT_FRICTION      = DEFAULT_ACCELERATION;
		const double DEFAULT_TOP_SPEED     = 6.0;
		const double DEFAULT_SLOPE         = 0.125;
		const double DEFAULT_GRAVITY       = 0.21875;

		const double AIR_ACCELERATION      = 2 * DEFAULT_ACCELERATION;
		const double AIR_DECCELERATION     = AIR_ACCELERATION;
		const double AIR_FRICTION          = 0.96875;

		const double FLIGHT_GRAVITY        = 0.03125;
	}

	const int ROLL_VERTICAL_OFFSET = 4;
}

class Player : public PhysicsEntity
{
public:
	enum class State : char { IDLE, WALKING, JUMPING, FLYING, CROUCHING, SPINDASH, ROLLING, ROLLJUMPING, LOOKING_UP };

	Player();

	typedef PhysicsEntity::entityPtrType entityPtrType;

	int  getRings() { return rings; };
	void switchPath() { path = !path; };
	
	void update(Camera&);
	void render(SDL_Rect& cam, double screenRatio);
	bool grounded() { return onGround; };
	bool isRolling() { return state == State::ROLLING; };

	void setActType(int aType);
	void setRings(int r) { rings = r; };
	void setJumping(bool j) { jumping = j; };
	void setControlLock(int c) { controlLock = c; };
	void setPath(int p) { path = p; };
	void setRolling(bool r) { state = r ? State::ROLLING : state; };
	void setFlightTime(double f) { flightTime = f; };
	void setCorkscrew(bool c) { corkscrew = c; };
	void setActCleared(bool b);

	void setOnGround(bool g) { onGround = g; };
	bool getOnGround() { return onGround || platform; };

	void setGsp(double g) { gsp = g; };
	double getGsp() { return gsp; };

	Mode getMode() { return collideMode; };

	double getAngle() { return hexToDeg(angle); };

	SDL_RendererFlip getFlip() { return SDL_RendererFlip(horizFlip); };

	int getDamageCountdown() { return damageCountdown; };

	SDL_Rect getCollisionRect();

	void hitPlatform(const entityPtrType entity);
	void hitWall(const entityPtrType entity);
	
	bool canDamage();
	entityPtrType& platformPtr() { return platform; };
	entityPtrType& wallPtr() { return wall; };
	int lookDirection() { return looking; };
	
	std::string collideGround(const std::vector < std::vector < Ground > >& tiles);
	int checkSensor(char sensor, const std::vector < std::vector < Ground > >& tiles, double& ang, bool* isTopOnly = nullptr);
	
	bool addRing(int num = 1);
	

	std::vector<std::unique_ptr<PhysicsEntity>>::iterator hitEnemy(std::vector < std::unique_ptr < PhysicsEntity > >& entities, std::vector<std::unique_ptr<PhysicsEntity>>::iterator currentEntity, std::vector < PhysStruct >& phys_paths, std::vector < PhysProp >& props, SDL_Window* window, SDL_Point enemyCenter);
	std::vector<std::unique_ptr<PhysicsEntity>>::iterator Damage(std::vector < std::unique_ptr < PhysicsEntity > >& entities, std::vector < PhysStruct >& phys_paths, std::vector < PhysProp >& props, std::vector<std::unique_ptr<PhysicsEntity>>::iterator& iter, int enemyX, SDL_Window* window);

	bool prevOnPlatform;

	static std::string modeToString(Mode m);

private:
	int rings;
	int damageCountdown;
	int actType;
	int controlLock;
	int looking;

	bool onGround;
	bool onGroundPrev;
	bool jumping;
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
	double flightTime;

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

	State state;

	void walkLeftAndRight(const InputComponent& input, double thisAccel, double thisDecel, double thisFrc);

	void updateIfWalkOrIdle(const InputComponent& input, double thisAccel, double thisDecel, double thisFrc);

	void updateInAir(const InputComponent& input, double thisAccel, double thisDecel, double thisFrc);

	static bool isOffsetState(const State& state);

	static int getHeight(const std::vector < std::vector < Ground > >& ground, SDL_Point block, SDL_Point tile, bool side, bool pathC, int xStart, int xEnd, int yStart, int yEnd, bool& flip);
};

