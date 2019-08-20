#pragma once
#include <queue>
#include "PhysicsEntity.h"
#include "Constants.h"

class Ground;
class InputComponent;
class Camera;
class PlayerState;

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

		const double ROLLING_DECCELERATION = 0.125;
		const double ROLLING_FRICTION      = DEFAULT_FRICTION / 2.0;

		const double AIR_ACCELERATION      = 2 * DEFAULT_ACCELERATION;
		const double AIR_DECCELERATION     = AIR_ACCELERATION;
		const double AIR_FRICTION          = 0.96875;

		const double FLIGHT_GRAVITY        = 0.03125;
	}

	const int ROLL_VERTICAL_OFFSET = 6;
}

class Player : public PhysicsEntity
{
	enum class Side { TOP, RIGHT, BOTTOM, LEFT };
	friend std::ostream& operator<< (std::ostream& str, Side side);
	typedef std::optional< std::tuple< SDL_Point, double, Side > > SensorResult;
public:

	typedef PhysicsEntity::entityPtrType entityPtrType;

	enum class State : char { IDLE, WALKING, JUMPING, FLYING, CROUCHING, SPINDASH, ROLLING, ROLLJUMPING, LOOKING_UP, NUM_STATES };

	Player();

	int  getRings() { return rings; };
	
	void update(std::vector < std::vector < Ground > >& tiles, EntityManager& manager);
	void render(const Camera& cam);
	bool grounded() { return onGround; };
	bool isRolling() { return state == State::ROLLING; };

	void jump(bool rolljump);

	void setActType(unsigned char aType);
	void setRings(int r) { rings = r; };
	void setJumping(bool j) { jumping = j; };
	void setPath(int p) { path = p; };
	void setRolling(bool r) { state = (r ? State::ROLLING : state); };
	void setFlightTime(int f) { flightTime.duration = Timer::DurationType{ f }; };
	void setCorkscrew(bool c) { corkscrew = c; };
	void setActCleared(bool b);

	void setOnGround(bool g) { onGround = g; };
	bool getOnGround() const { return onGround; };

	void setGsp(double g) { gsp = g; };
	double getGsp() const { return gsp; };

	double getSpindash() const { return spindash; };

	Mode getMode() { return collideMode; };

	void setAngle(double angle);
	double getAngle() const;

	bool getPath() const { return path; };

	SDL_RendererFlip getFlip() const { return SDL_RendererFlip(horizFlip); };

	void setFlip(bool flip) { horizFlip = flip; }

	Timer getDamageCountdown() const { return damageCountdown; };

	bool canDamageEnemy() const;
	int lookDirection() const;

	void destroyEnemy(EntityManager& manager, PhysicsEntity& entity);
	
	void addCollision(std::unique_ptr< PhysicsEntity >& entity);

	void doCorkscrew();

	void setAnimation(std::size_t index);

	void collideGround(const std::vector < std::vector < Ground > >& tiles, std::vector < AbsoluteHitbox >& platforms, std::vector < AbsoluteHitbox >& walls);

	bool addRing(int num = 1);
	

	void hitEnemy(EntityManager& manager, PhysicsEntity& enemyCenter);
	void takeDamage(EntityManager& manager, int enemyCenterX);

	int getXRadius() const;
	int getYRadius() const;

	static std::string modeToString(Mode m);

	enum class Sensor { A, B, C, D, E, F };
	enum class Direction { UP, RIGHT, DOWN, LEFT };

	static SensorResult checkSensor(const SDL_Point& position, const SDL_Point& radii, const Vector2& velocity, Mode mode, Sensor sensor, bool path, const std::vector < std::vector < Ground > >& tiles);

	SensorResult checkSensor(Sensor sensor, const std::vector < std::vector < Ground > >& tiles) const;

	std::optional< SDL_Point > getSensorPoint(Sensor sensor, const std::vector< std::vector< Ground > >& tiles) const;

	static std::tuple< std::pair< int, int >, std::pair< int, int >, Direction > getRange(const SDL_Point& position, const SDL_Point& radii, Mode mode, Sensor sensor);

	std::tuple< std::pair< int, int >, std::pair< int, int >, Direction > getRange(Sensor sensor) const;

	//void setState(std::unique_ptr< PlayerState >&& newState);

	Timer getControlLock() const { return controlLock; }

	void setControlLock(const Timer& timer) { controlLock = timer; }

	void startControlLock() { controlLock.start(); }

	bool isLocked() const { return controlLock.isTiming(); }

private:
	template < typename F >
	void applyResult(const SensorResult& result, int distance, F&& onSurfaceHit);

	std::queue< PhysicsEntity* > collisionQueue;

	int rings = 0;
	Timer damageCountdown{ 2000 };
	int actType = 0;
	Timer controlLock{ 400 };

	bool onGround = false;
	bool onGroundPrev = false;
	bool jumping = false;
	bool corkscrew = false;
	bool ceilingBlocked = false;
	bool actCleared = false;

	double gsp = 0.0;
	double spindash = -1.0;
	Timer flightTime{ 8000 };

	double angle = 0.0;
	int displayAngle = 0;

	bool horizFlip = false;

	Mode collideMode = Mode::GROUND;

	double jmp = 0.0;
	
	bool path = 0;

	State state = State::IDLE;
	
	//std::unique_ptr< PlayerState > state = nullptr;

	void handleCollisions(std::vector < std::vector < Ground > >& tiles, EntityManager& manager);

	void collideWalls(const std::vector< std::vector< Ground > >& tiles, const std::vector< AbsoluteHitbox >& walls);

	void collideCeilings(const std::vector< std::vector< Ground > >& tiles);

	void walkLeftAndRight(const InputComponent& input, double thisAccel, double thisDecel, double thisFrc);

	void updateIfWalkOrIdle(const InputComponent& input, double thisAccel, double thisDecel, double thisFrc, double slp);

	void updateInAir(const InputComponent& input, double thisAccel, double thisDecel, double thisFrc);

	static void restrictVelocityDirection(Vector2& point, SDL_Point dir, int rotation);

	bool getKeyPress(const InputComponent& input, int key) const;

	bool getKeyState(const InputComponent& input, int key) const;

	static std::pair< int, bool > getHeight(const std::vector< std::vector < Ground > >& ground, SDL_Point block, SDL_Point tilePosition, bool side, bool path, std::pair< int, int > xRange, std::pair< int, int > yRange);

	__attribute__((const)) friend bool operator< (const SensorResult& a, const SensorResult& b);

	__attribute__((const)) friend bool operator< (const SensorResult& a, const SDL_Point& b);
};

SDL_Point directionCompare(SDL_Point a, SDL_Point b, Player::Direction direction);

double hexToDeg(double hex);

double hexToRad(double hex);

int signum(int a);

int signum(double a);

bool isOffsetState(const Player::State& state);

std::ostream& operator<< (std::ostream& str, Player::Sensor sensor);

std::ostream& operator<< (std::ostream& str, Player::Direction sensor);

std::ostream& operator<< (std::ostream& str, Player::Side side);