#pragma once
#include <queue>
#include "PhysicsEntity.h"
#include "Constants.h"

class Ground;
class InputComponent;
class Camera;

enum Mode { GROUND, LEFT_WALL, CEILING, RIGHT_WALL };

namespace player_constants {
	namespace animation_paths {
		const std::string TORNADO_PATH = TAILS_PATH"Tornado.png";
		const std::string IDLE_PATH = TAILS_PATH"Idle.png";
		const std::string WALK_PATH = TAILS_PATH"Walk.png";
		const std::string RUN_PATH = TAILS_PATH"Run.png";
		const std::string ROLL_BODY_PATH = TAILS_PATH"Roll_Body.png";
		const std::string ROLL_TAILS_PATH = TAILS_PATH"Roll_Tails.png";
		const std::string CROUCH_PATH = TAILS_PATH"Crouch.png";
		const std::string LOOK_UP_PATH = TAILS_PATH"Look_Up.png";
		const std::string SPINDASH_PATH = TAILS_PATH"Spindash.png";
		const std::string FLY_PATH = TAILS_PATH"Fly.png";
		const std::string FLY_TIRED_PATH = TAILS_PATH"Fly_Tired.png";
		const std::string CORKSCREW_PATH = TAILS_PATH"Corkscrew.png";
		const std::string HURT_PATH = TAILS_PATH"Hurt.png";
		const std::string ACT_CLEAR_PATH = TAILS_PATH"Act_Clear.png";
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
	typedef PhysicsEntity::entityPtrType entityPtrType;

	enum class State : char { IDLE, WALKING, JUMPING, FLYING, CROUCHING, SPINDASH, ROLLING, ROLLJUMPING, LOOKING_UP, NUM_STATES };

	Player();

	int  getRings() { return rings; };
	
	void update(std::vector < std::vector < Ground > >& tiles, EntityManager& manager);
	void render(SDL_Rect& cam, double screenRatio);
	bool grounded() { return onGround; };
	bool isRolling() { return state == State::ROLLING; };

	void setActType(unsigned char aType);
	void setRings(int r) { rings = r; };
	void setJumping(bool j) { jumping = j; };
	void setPath(int p) { path = p; };
	void setRolling(bool r) { state = r ? State::ROLLING : state; };
	void setFlightTime(int f) { flightTime.duration = Timer::DurationType{ f }; };
	void setCorkscrew(bool c) { corkscrew = c; };
	void setActCleared(bool b);

	void setOnGround(bool g) { onGround = g; };
	bool getOnGround() const { return onGround; };

	void setGsp(double g) { gsp = g; };
	double getGsp() const { return gsp; };

	Mode getMode() { return collideMode; };

	double getAngle();

	SDL_RendererFlip getFlip() const { return SDL_RendererFlip(horizFlip); };

	int getDamageCountdown() const { return damageCountdown; };

	SDL_Rect getCollisionRect() const;
	
	bool canDamageEnemy() const;
	int lookDirection() const;

	void destroyEnemy(EntityManager& manager, PhysicsEntity& entity);
	
	void addCollision(std::unique_ptr<PhysicsEntity>& entity);

	void doCorkscrew();

	void setAnimation(std::size_t index);

	std::string collideGround(const std::vector < std::vector < Ground > >& tiles, std::vector < SDL_Rect >& platforms, std::vector < SDL_Rect >& walls);

	int checkSensor(char sensor, const std::vector < std::vector < Ground > >& tiles, double& ang, bool* isTopOnly = nullptr);
	
	bool addRing(int num = 1);
	

	void hitEnemy(EntityManager& manager, PhysicsEntity& enemyCenter);
	void takeDamage(EntityManager& manager, int enemyCenterX);

	int getYRadius() const;

	static std::string modeToString(Mode m);

private:
	std::queue < PhysicsEntity* > collisionQueue;

	//typedef std::optional< std::tuple< int, double, bool > > SensorResult;

	int rings;
	int damageCountdown;
	int actType;
	int controlLock;

	bool onGround;
	bool onGroundPrev;
	bool jumping;
	bool corkscrew;
	bool ceilingBlocked;
	bool actCleared;

	double gsp;
	double spindash;
	Timer flightTime;

	double angle;
	int displayAngle;

	bool horizFlip;

	Mode collideMode;

	double jmp;
	
	bool path;

	State state;

	void handleCollisions(std::vector < std::vector < Ground > >& tiles, EntityManager& manager);

	void walkLeftAndRight(const InputComponent& input, double thisAccel, double thisDecel, double thisFrc);

	void updateIfWalkOrIdle(const InputComponent& input, double thisAccel, double thisDecel, double thisFrc, double slp);

	void updateInAir(const InputComponent& input, double thisAccel, double thisDecel, double thisFrc);

	static int getHeight(const std::vector < std::vector < Ground > >& ground, SDL_Point block, SDL_Point tilePosition, bool side, bool pathC, int xStart, int xEnd, int yStart, int yEnd, bool& flip);
};

double hexToDeg(double hex);

double hexToRad(double hex);

int signum(int a);

int signum(double a);

bool isOffsetState(const Player::State& state);
