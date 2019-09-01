#include "stdafx.h"
#include <algorithm>
#include <cmath>
#include "Player.h"
#include "EntityTypes.h"
#include "Functions.h"
#include "Ground.h"
#include "CollisionTile.h"
#include "Miscellaneous.h"
#include "InputComponent.h"
#include "Camera.h"
#include "Drawing.h"

Player::Player() {
	using namespace player_constants::animation_paths;
	using namespace std::chrono_literals;
	const std::vector < AnimStruct > PLAYER_ANIMATIONS = {
		{ TORNADO_PATH, 50ms, 4 },		//0
		{ IDLE_PATH, 133ms, 5 },
		{ WALK_PATH, 133ms, 7 },
		{ RUN_PATH,  133ms, 4 },
		{ ROLL_BODY_PATH, 64ms, 6 },	//4
		{ ROLL_TAILS_PATH, 128ms, 3 },
		{ CROUCH_PATH, 133ms, 5 },
		{ SPINDASH_PATH, 30ms, 5 },
		{ FLY_PATH, 50ms, 2 },		//8
		{ FLY_TIRED_PATH, 60ms, 4 },
		{ LOOK_UP_PATH, 133ms, 5 },
		{ CORKSCREW_PATH, 50ms, 11 },
		{ HURT_PATH, 32ms, 2 },		//12
		{ ACT_CLEAR_PATH, 150ms, 3 }
	};
	
	for (const AnimStruct& i : PLAYER_ANIMATIONS) {
		animations.push_back(std::make_unique<Animation>(i));
	}

	position = { 20, 0 };
	hitbox = HitboxForm(Rect{ 0, 19, 120, 30 });

	customData = NoCustomData{};
}

void Player::setActType(unsigned char aType) {
	actType = aType;
	if (actType == 1) {
		setAnimation(0);
		gravity = 0;
	}
	else if (actType == 2) {
		hitbox = HitboxForm(Rect{ 0, 0, 36, 31 });
		setAnimation(1);
		gravity = 0.21875;
	}
}

void Player::setAngle(double ang) {
	angle = ang;
}

double Player::getAngle() const {
	return angle;
}

void Player::takeDamage(EntityManager& manager, int enemyCenterX) {
	double toRad = M_PI / 180;
	flightTime.stop();
	state = State::IDLE;
	gravity = 0.21875;
	onGround = false;
	jumping = false;

	if (!damageCountdown.isTiming()) {
		for (int i = 0; i < std::min(rings, 32); ++i) {
			const auto& ringProperties = entity_property_data::getEntityTypeData("RING");
			auto temp = std::make_unique<PhysicsEntity>( "RING", std::vector< char >{}, position, true);
			const int speed = ((i >= 16) ? 2 : 4);
			const int dir = ((i % 2 == 0) ? -1 : 1);
			const double angle = 101.25 + 22.5 * ((i % 16) / 2);
			temp->setVelocity({ dir * sin(angle * toRad) * speed, cos(angle * toRad) * speed });
			temp->setGravity(0.09375);
			temp->shouldSave = false;
			manager.AddEntity(std::move(temp));
		}
		damageCountdown.start();
		rings = 0;
		velocity.y = -4;
		velocity.x = 2 * signum(position.x - enemyCenterX);
		velocity.x = (velocity.x == 0) ? 1.0 : velocity.x;
		setAnimation(12);
	}
}

int Player::getXRadius() const {
	 return 9 - 2 * (state == State::ROLLING || state == State::ROLLJUMPING);
}

int Player::getYRadius() const {
	int yRadius = 0;
	if (currentAnim.size() == 2 && currentAnim[0] == 5 && currentAnim[1] == 4) {
		return 14;
	}
	else if (currentAnim.size() == 2 && currentAnim[0] == 5 && currentAnim[1] == 7) {
		return 21;
	}
	for (auto index : currentAnim) {
		const auto& anim = animations[index];
		yRadius = std::max(yRadius, anim->GetSize().y - anim->getOffset().value_or(SDL_Point{ 0, 0 }).y);
	}
	return yRadius;
}

void Player::hitEnemy(EntityManager& manager, PhysicsEntity& enemy) {
	if (canDamageEnemy()) {
		destroyEnemy(manager, enemy);
	}
	else {
		takeDamage(manager, enemy.getPosition().x);
	}
}

void Player::update(std::vector < std::vector < Ground > >& tiles, EntityManager& manager) {
	using namespace player_constants;
	using namespace physics;

	InputComponent& input = globalObjects::input;

	const auto deltaTime = Timer::getFrameTime();

	if (deltaTime.count() == 0) {
		return;
	}
	
	if (onGround) {
		if (!onGroundPrev) {
			if (velocity.x == 0 && velocity.y == 0) {
				gsp = 0.0;
			}
			else {
				// Get direction of the velocity
				double velocityAngle = atan2(-velocity.y, velocity.x);
				
				// Compute the dot product between the velocity and the ground
				gsp = sqrt(pow(velocity.x, 2) + pow(velocity.y, 2)) * cos(velocityAngle - hexToRad(angle));
			}
		}
		velocity.x = gsp *  cos(hexToRad(angle));
		velocity.y = gsp * -sin(hexToRad(angle));
	}
	

	switch (actType) {
	//Tornado
	case 1:
		if (input.GetKeyState(InputComponent::LEFT)) {
			velocity.x = 1;
		}
		else if (input.GetKeyState(InputComponent::RIGHT)) {
			velocity.x = 2;
		}
		else if (std::abs(velocity.x - 1.5) <= 0.05) {
			velocity.x = 1.5;
		}
		else {
			velocity.x = (0.9 * (velocity.x - 1.5)) + 1.5;
		}
		if (input.GetKeyState(InputComponent::LEFT)) {
			velocity.y = 2;
			break;
		}
		else if (input.GetKeyState(InputComponent::RIGHT)) {
			velocity.y = -2;
			break;
		}
		else if (std::abs(velocity.y) <= 0.5) {
			velocity.y = 0;
			break;
		}
		velocity.y *= 0.9;
		break;
	//Normal act
	case 2:
		{
		if (!onGroundPrev && onGround) {
			if (velocity.x == 0) {
				state = State::IDLE;
			}
			else {
				state = State::WALKING;
			}
		}

		// Prevent walking past the stage border
		if (position.x <= 10) {
			position.x = 10;
			gsp = std::max(0.0, gsp);
			velocity.x = std::max(0.0, velocity.x);
		}

		double accel = (onGround ? DEFAULT_ACCELERATION : AIR_ACCELERATION); 
		double decel = (onGround ? DEFAULT_DECCELERATION : AIR_DECCELERATION);
		double frc = (onGround ? DEFAULT_FRICTION : AIR_FRICTION);
		double top = DEFAULT_TOP_SPEED;
		double slp = DEFAULT_SLOPE;

		// Reset gravity unless flying
		if (state != State::FLYING) {
			gravity = DEFAULT_GRAVITY;
		}

		// Set miscellaneous variables
		if (onGround) {
			flightTime.stop();
			jmp = 0;
			corkscrew = false;
		}

		const double FRAME_TIME_MS = 1000.0 / 60.0;

		const double thisFrameCount = deltaTime.count() / FRAME_TIME_MS;

		double thisAccel = thisFrameCount * accel;
		double thisDecel = thisFrameCount * decel;
		double thisFrc   = thisFrameCount * frc;

		double anim_steps = std::max(8.0 - std::abs(gsp), 1.0);

		if (controlLock.update()) {
			controlLock.stop();
		}

		using namespace std::chrono_literals;

		switch (state) {
		case State::IDLE:

			// If pressing down, crouch.
			if (getKeyState(input, InputComponent::DOWN)) {
				state = State::CROUCHING;
				setAnimation(6);
				break;
			}
			// If pressing up, look up
			else if (getKeyState(input, InputComponent::UP)) {
				state = State::LOOKING_UP;
				break;
			}

			updateIfWalkOrIdle(input, thisAccel, thisDecel, thisFrc, slp);

			if (gsp != 0) {
				state = State::WALKING;
				break;
			}

			setAnimation(1);

			break;
		case State::WALKING:

			// If moving slowly enough, just crouch. Otherwise, roll.
			if (getKeyState(input, InputComponent::DOWN)) {
				if (std::abs(gsp) < 0.5) {
					state = State::CROUCHING;
					gsp = 0.0;
					setAnimation(6);
					break;
				}
				else {
					state = State::ROLLING;
					setAnimation(5 + 4 * animations.size());
					break;
				}
			}

			updateIfWalkOrIdle(input, thisAccel, thisDecel, thisFrc, slp);

			if (gsp == 0) {
				state = State::IDLE;
				break;
			}

			setAnimation(2 + (std::abs(gsp) >= 0.9 * top));
			animations[currentAnim.front()]->setDelay(Animation::DurationType(static_cast<int>(anim_steps * FRAME_TIME_MS)));

			if (gsp < 0) {
				horizFlip = true;
			}
			else if (gsp > 0) {
				horizFlip = false;
			}

			break;
		case State::JUMPING:

			// Start flying
			if (getKeyPress(input, InputComponent::JUMP) && !corkscrew) {
				flightTime.start();
				gravity = 0.03125;
				state = State::FLYING;
				break;
			}
			// Allow for variable jump heights
			else if (!input.GetKeyState(InputComponent::JUMP) && jumping && velocity.y < -4.0 && !corkscrew) {
				velocity.y = -4.0;
			}

			setAnimation(5 + 4 * animations.size());
			animations[5]->setDelay(128ms);

			break;
		case State::FLYING:
			thisFrc = pow(frc, thisFrameCount);

			if (velocity.y < -1) {
				gravity = FLIGHT_GRAVITY;
			}
			
			if (flightTime.update()) {
				flightTime.stop();
			}
			
			// If there is flight time remaining, check for the jump button being pressed
			if (flightTime.isTiming() && getKeyPress(input, InputComponent::JUMP)) {
				if (velocity.y >= -1) {
					gravity = -0.125;
				}
			}

			// Change flip only if velocity crosses zero, not just touches it
			if (velocity.x < 0) {
				horizFlip = true;
			}
			else if (velocity.x > 0) {
				horizFlip = false;
			}

			setAnimation(flightTime.isTiming() ? 8 : 9);

			break;
		case State::CROUCHING:

			// If no longer pressing down, return to normal
			if (!getKeyState(input, InputComponent::DOWN)) {
				state = State::IDLE;
				setAnimation(1);
				break;
			}
			// If jump is pressed, start a spindash
			else if (getKeyPress(input, InputComponent::JUMP)) {
				state = State::SPINDASH;
				spindash = 2.0;
				setAnimation(7);
				break;
			}

			setAnimation(6);

			break;
		case State::SPINDASH:

			// If no longer pressing down, start rolling
			if (!getKeyState(input, InputComponent::DOWN)) {
				state = State::ROLLING;
				setAnimation(5 + 4 * animations.size());
				std::cout << spindash << "\n";
				gsp = 8.0 + floor(spindash / 2.0);
				gsp = (horizFlip ? -gsp : gsp);
				spindash = -1.0;
				break;
			}
			// If jump is pressed, add speed
			else if (getKeyPress(input, InputComponent::JUMP)) {
				spindash += 2.0;
			}

			// Make sure spindash decays
			spindash -= (floor(spindash * 8.0) / 256.0) * thisFrameCount;

			setAnimation(5 + 7 * animations.size());
			animations[5]->setDelay(60ms);

			break;
		case State::ROLLING:
			decel = 0.125;
			top = 16;

			thisDecel = thisFrameCount * decel;

			// Check for moving too slowly on a wall or ceiling
			if (std::abs(gsp) < 2.0 && collideMode != GROUND) {
				if (angle >= 0x40 && angle <= 0xC0) {
					collideMode = GROUND;
					angle = 0.0;
					onGround = false;
				}
				else {
					onGround = true;
				}
				controlLock.start();
			}
			// Normal input to the left
			else if (getKeyState(input, InputComponent::LEFT) && !controlLock.isTiming()) {
				if (gsp >= thisDecel) {
					gsp -= thisDecel;
				}
			}
			// Normal input to the right
			else if (getKeyState(input, InputComponent::RIGHT) && !controlLock.isTiming()) {
				if (gsp <= thisDecel) {
					gsp += thisDecel;
				}
			}

			// Calculate slope value
			if (signum(gsp) == signum(sin(hexToRad(angle)))) {
				// Uphill
				slp = 0.078125;
			}
			else {
				// Downhill
				slp = 0.3125;
			}

			// Slope calculations
			gsp -= sin(hexToRad(angle)) * slp * thisFrameCount;
			
			// If friction would cause a sign change, stop
			if (onGround) {
				if (std::abs(gsp) < thisFrc / 2.0) {
					gsp = 0.0;
					state = State::IDLE;
					setAnimation(0);
				}
				else {
					gsp -= thisFrc * signum(gsp) / 2.0;
				}
			}

			// Going too slow should cause us to stop rolling
			if (std::abs(gsp) < 0.5 && onGround) {
				state = (gsp == 0 ? State::IDLE : State::WALKING);
				setAnimation(gsp == 0 ? 1 : 2);
				break;
			}

			// If the jump key gets pressed then initiate a rolljump
			if (getKeyPress(input, InputComponent::JUMP) && !controlLock.isTiming() && !ceilingBlocked && onGround) {
				jump(true);
			}


			setAnimation(5 + 4 * animations.size());
			animations[5]->setDelay(128ms);

			break;
		case State::ROLLJUMPING:

			setAnimation(5 + 4 * animations.size());
			animations[5]->setDelay(128ms);

			break;
		case State::LOOKING_UP:

			// If no longer pressing up, return to normal
			if (!getKeyState(input, InputComponent::UP)) {
				state = State::IDLE;
				break;
			}

			setAnimation(10);

			break;
		default:
			throw std::invalid_argument("Invalid Player state value " + std::to_string(static_cast< int >(state)));
		}

		// Reset jump
		if (onGround) {
			jmp = 0.0;
		}
		// Update in air
		else {
			const double thisAccel = thisFrameCount * accel;
			const double thisDecel = thisFrameCount * decel;
			const double thisFrc = pow(frc, thisFrameCount);
			updateInAir(input, thisAccel, thisDecel, thisFrc);
		}

		// Overriding default animations
		if (actCleared) {
			setAnimation(13);
			gsp = 0.0;
			velocity.x = 0.0;
		}
		else if (damageCountdown.isTiming()) {
			setAnimation(12);
		}
		else if (corkscrew) {
			setAnimation(11);
		}

		// Reset act-clear animation
		if (currentAnim.front() == 13 && (animations[13]->GetLooped() || animations[13]->getFrame() == 2)) {
			animations[13]->SetFrame(2);
			animations[13]->setDelay(-1ms);
		}

		if (state != State::ROLLING) { 
			gsp = std::clamp(gsp, -16.0, 16.0);
		}

		velocity.y = std::clamp(velocity.y, -16.0, 16.0);
		}
	}

	if (damageCountdown.isTiming()) {
		gsp = 0.0;
		invis = (damageCountdown.timeRemaining().count() % 132) > 66;
	}
	else {
		invis = false;
	}

	if (std::abs(velocity.y) < 1e-10) {
		velocity.y = 0.0;
	}

	if (damageCountdown.update()) {
		damageCountdown.stop();
	}

	handleCollisions(tiles, manager);

	PhysicsEntity::update(this, &manager);

	if (onGround) {
		velocity.y -= gravity * (Timer::getFrameTime().count() * 60.0 / 1000.0);
	}


}

void Player::jump(bool rolljump) {
	onGround = false;
	jumping = true;
	state = (rolljump ? State::ROLLJUMPING : State::JUMPING);
	collideMode = GROUND;
	jmp = 6.5;
	velocity.x -= jmp * sin(hexToDeg(angle) * M_PI / 180.0);
	velocity.y -= jmp * cos(hexToDeg(angle) * M_PI / 180.0);
	angle = 0.0;
}

bool Player::addRing(int num) {
	if (!damageCountdown.isTiming() || damageCountdown.timeRemaining().count() < 100) {
		rings += num;
		return true;
	}
	return false;
}

void Player::addCollision(std::unique_ptr<PhysicsEntity>& entity) {
	collisionQueue.push(entity.get());
}

void Player::doCorkscrew() {
	corkscrew = true;
	setAnimation(11);
}

void Player::setAnimation(std::size_t index) {
	std::vector< std::size_t > nextAnim;
	for (; index != 0; index /= animations.size()) {
		nextAnim.push_back(index % animations.size());
	}
	if (!currentAnim.empty() && !nextAnim.empty()) {
		Animation& currentAnimation = *animations[currentAnim.front()];
		Animation& nextAnimation = *animations[nextAnim.front()];
		if (currentAnimation.getNumFrames() == nextAnimation.getNumFrames()) {
			nextAnimation.synchronize(currentAnimation);
		}
	}
	const int oldYRadius = getYRadius();
	currentAnim = nextAnim;
	position.y += oldYRadius - getYRadius();
}

void Player::handleCollisions(std::vector < std::vector < Ground > >& tiles, EntityManager& manager) {
	bool hurt = false;
	int damageCenter = -1;
	std::vector < AbsoluteHitbox > platforms;
	std::vector < AbsoluteHitbox > walls;

	collideCeilings(tiles);
	const int yRadius = getYRadius();
	const int xRadius = getXRadius();
	
	SDL_Point offset = animations[currentAnim.front()]->getOffset().value_or(SDL_Point{ 0, 0 }); 
	if (currentAnim.front() == 5) {
		offset = animations[currentAnim[1]]->getOffset().value_or(SDL_Point{ 0, 0 });
	}

	Rect rawShape{ double(-xRadius), double(-offset.y), 2.0 * xRadius, double(yRadius + offset.y) };
	rawShape = rotate90(static_cast< int >(collideMode), rawShape, { 0.0, 0.0 });
	hitbox = HitboxForm(rawShape);

	for (PhysicsEntity& entity = *collisionQueue.front(); !collisionQueue.empty(); collisionQueue.pop()) {
		const auto& entityKey = entity.getKey();
		const auto& entityTypeData = entity_property_data::getEntityTypeData(entityKey);

		AbsoluteHitbox entityCollision = entity.getAbsHitbox();

		SDL_Point entityCenter;

		SDL_Point dir;

		if (canBePushedAgainst(entity)) {
			platforms.push_back(entityCollision);	
			walls.push_back(entityCollision);
		}
		else if (canBeStoodOn(entity)) {
			platforms.push_back(entityCollision);
		}

		entity_property_data::helpers::getOne< int >([&](auto& p) { p.onPlayerTouch(entity, manager, *this); return int{}; }, entity.getCustom(), entity.getCustom().index());

		if (entityKey == "RINGMONITOR") {
			if (canDamageEnemy()) {
				hitEnemy(manager, entity);
				addRing(10);
			}
		}

		/*if (entity_property_data::isHazard(entityKey)) {
			std::cout << "Collision with hazard: " << entityKey << "\n";
			entityCenter = getXY(entityCollision) + (SDL_Point{ entityCollision.w, entityCollision.h } / 2);

			if (entity_property_data::isEnemy(entityKey)) {
				hitEnemy(manager, entity);
			}
			else {
				takeDamage(manager, entity.getCollisionRect().x + entity.getCollisionRect().w / 2);
			}
		}*/
	}

	if (hurt) {
		takeDamage(manager, damageCenter);
	}

	collideGround(tiles, platforms, walls);
}

std::optional< std::pair< SDL_Point, double > > findGroundHeight(const std::vector< std::vector< Ground > >& tiles, Point position, Mode mode, HitboxForm hitbox, bool useOneWayPlatforms, bool path, Direction relativeDir) {
	Direction dir = Direction::DOWN;
	switch (mode) {
	case Mode::GROUND:
		dir = Direction::DOWN;
		break;
	case Mode::LEFT_WALL:
		dir = Direction::LEFT;
		break;
	case Mode::CEILING:
		dir = Direction::UP;
		break;
	case Mode::RIGHT_WALL:
		dir = Direction::RIGHT;
		break;
	}
	dir = static_cast< Direction >((static_cast< int >(dir) + static_cast< int >(relativeDir) + 6) % 4);

	Rect box = hitbox.getBox();
	bool side = false;
	int startCoord = 0;
	int endCoord = 0;
	if (mode == Mode::GROUND || mode == Mode::CEILING) {
		side = false;
		startCoord = box.x + position.x;
		endCoord = box.x + box.w + position.x;
	}
	else {
		side = true;
		startCoord = box.y + position.y;
		endCoord = box.y + box.h + position.y;
	}

	std::vector< std::pair< SDL_Point, double > > results;
	for (int c = startCoord; c < endCoord; ++c) {
		auto point = (side ? SDL_Point{ int(position.x), c } : SDL_Point{ c, int(position.y) });
		auto result = collideLine(point, 32, dir, tiles, useOneWayPlatforms, path);
		if (result) {
			results.push_back(*result);
		}
	}

	if (results.empty()) {
		return {};
	}

	// Rotate all points to standard orientation for easy ordering
	std::transform(results.begin(), results.end(), results.begin(),
		[=](auto p) -> std::pair< SDL_Point, double >{ return { rotate90(-static_cast<int>(mode), p.first), p.second }; }
	);

	// Find maximum height
	auto maxElem = std::min_element(results.begin(), results.end(), [](auto lhs, auto rhs) { return lhs.first.y < rhs.first.y; });
	int maxHeight = maxElem->first.y;

	// Remove points not at the maximum height
	if (auto iter = std::remove_if(results.begin(), results.end(), [=](auto p) { return p.first.y != maxHeight; }); iter != results.end()) {
		results.erase(iter, results.end());
	}

	std::transform(results.begin(), results.end(), results.begin(),
		[=](auto p) -> std::pair< SDL_Point, double >{ return { rotate90(static_cast< int >(mode), p.first), p.second }; }
	);

	double averageAngle = 0.0;
	for (auto result : results) {
		averageAngle += result.second;
	}
	averageAngle /= results.size();

	// TODO: calculate angle as well by averaging the results
	return { { results.front().first, averageAngle } };
}

//Returns height of A, height of B, and angle
void Player::collideGround(const std::vector < std::vector < Ground > >& tiles, std::vector < AbsoluteHitbox >& platforms, std::vector < AbsoluteHitbox >& walls) {
	onGroundPrev = onGround;
	const int xRadius = getXRadius();
	const int yRadius = getYRadius();

	Mode last_mode = collideMode;
	//Calculate which mode to be in based on angle
	collideMode = Mode((static_cast<int>(angle + 0x20) & 0xFF) >> 6);

	collideWalls(tiles, walls);

	collideCeilings(tiles);

	//SensorResult floor = std::max(checkSensor(Sensor::A, tiles), checkSensor(Sensor::B, tiles));
	
	SensorResult floor{};
	if (auto result = findGroundHeight(tiles, getPosition(), collideMode, getHitbox(), false, getPath(), Direction::DOWN)) {
		Side side;
		switch (collideMode) {
		case GROUND:
			result->first.x = getPosition().x;
			side = Side::TOP;
			break;
		case LEFT_WALL:
			result->first.y = getPosition().y;
			side = Side::RIGHT;
			break;
		case CEILING:
			result->first.x = getPosition().x;
			side = Side::BOTTOM;
			break;
		case RIGHT_WALL:
			result->first.y = getPosition().y;
			side = Side::LEFT;
			break;
		}
		floor.emplace(result->first, result->second, side);
	}

	// Check platform sensors
	if (collideMode == GROUND && velocity.y >= 0.0) {
		for (const AbsoluteHitbox& entityCol: platforms) {
			Rect entityCollision = entityCol.box.getBox();
			const int entityRadius = entityCollision.w / 2;
			const int entityTop = entityCollision.y + 1;
			const int entityCenter = entityCollision.x + entityRadius;

			const int rightDir = position.x + xRadius - entityCenter;
			const int leftDir = position.x - xRadius - entityCenter;

			if (rightDir < -entityRadius || leftDir > entityRadius) {
				continue;
			}
			else {
				floor = std::max(floor, SensorResult{{ { int(position.x), entityTop }, 0.0, Side::TOP }});
			}
		}
	}
	
	if (onGround) {
		if (floor) {
			angle = std::get< double >(*floor);

			double temp1;

			const auto playerRadius = rotate90(static_cast< int >(collideMode), SDL_Point{ 0, yRadius - 1 });
			Point fracPart = { std::modf(position.x, &temp1), std::modf(position.y, &temp1) };
			SDL_Point temp = std::get< SDL_Point >(*floor) - playerRadius;
			position = { double(temp.x), double(temp.y) };
			position += fracPart;
		}
		else {
			onGround = false;
			angle = 0.0;
			collideMode = GROUND;
		}
	}
	else if (floor && velocity.y >= 0.0) {
		const int height = std::get< SDL_Point >(*floor).y - (position.y + yRadius);
		if (height <= 2) {
			angle = std::get< double >(*floor);
			onGround = true;
			jumping = false;
			position.y += height;
		}
	}

}

void Player::collideCeilings(const std::vector< std::vector< Ground > >& tiles) {
	//Push the player out of ceilings
	if (const auto ceiling = std::max(checkSensor(Sensor::C, tiles), checkSensor(Sensor::D, tiles))) {
		const auto [point, angle, direction] = *ceiling;
		const int topDistance = position.y - getAbsHitbox().box.getBox().y;
		const auto playerRadius = rotate90(static_cast< int >(collideMode), SDL_Point{ 0, topDistance });
		const auto newPosition = point + playerRadius;
		if (SensorResult{{ SDL_Point{ int(position.x), int(position.y) }, angle, direction }} < newPosition) {
			double temp;
			position = Point{ double(newPosition.x), double(newPosition.y) } + (position - Point{ std::modf(position.x, &temp), std::modf(position.y, &temp) });
			restrictVelocityDirection(velocity, { 0, 1 }, static_cast< int >(collideMode));
		}
	}
}

void Player::collideWalls(const std::vector< std::vector< Ground > >& tiles, const std::vector< AbsoluteHitbox >& walls) {
	//Check for left wall
	SensorResult lWall = checkSensor(Sensor::E, tiles);

	//Check for right wall
	SensorResult rWall = checkSensor(Sensor::F, tiles);

	if (collideMode == Mode::GROUND) {
		for (const AbsoluteHitbox& entityCol : walls) {
			Rect entityCollision = entityCol.box.getBox();
			if (position.y + getYRadius() < entityCollision.y || position.y + 4 > entityCollision.y + entityCollision.h) {
				continue;
			}

			if (position.x < entityCollision.x + entityCollision.w / 2) {
				const SDL_Point pos{ int(entityCollision.x), int(position.y) };
				const auto entityWall = SensorResult{{ pos, 32.0, Side::LEFT }};
				rWall = std::max(rWall, entityWall);
			}
			else {
				const SDL_Point pos{ int(entityCollision.x + entityCollision.w), int(position.y) };
				const auto entityWall = SensorResult{{ pos, 192.0, Side::RIGHT }};
				lWall = std::max(lWall, entityWall);
			}
		}
	}

	const auto rWallRadius = rotate90(static_cast< int >(collideMode), SDL_Point{  10, 0 });
	const auto lWallRadius = rotate90(static_cast< int >(collideMode), SDL_Point{ -10, 0 });
	const auto playerRightEdge = rotate90(static_cast< int >(collideMode), static_cast< SDL_Point >(position) + SDL_Point{  10, 0 }, static_cast< SDL_Point >(position));
	const auto playerLeftEdge  = rotate90(static_cast< int >(collideMode), static_cast< SDL_Point >(position) + SDL_Point{ -10, 0 }, static_cast< SDL_Point >(position));

	double temp;

	//Push the player out of walls
	if (rWall && !(rWall < playerRightEdge)) {
		position = static_cast< Point >(std::get<0>(*rWall) - rWallRadius) + Point{ std::modf(position.x, &temp), std::modf(position.y, &temp) };
		restrictVelocityDirection(velocity, { -1, 0 }, static_cast< int >(collideMode));
	}
	if (lWall && !(lWall < playerLeftEdge)) {
		position = static_cast< Point >(std::get<0>(*lWall) - lWallRadius) + Point{ std::modf(position.x, &temp), std::modf(position.y, &temp) };
		restrictVelocityDirection(velocity, {  1, 0 }, static_cast< int >(collideMode));
	}
}

static CollisionTile getTile(SDL_Point position, const std::vector< std::vector< Ground > >& tiles, bool path) {
	if (position.x < 0 || position.y < 0) {
		return { 0, 0 };
	}
	if (position.x >= tiles.size() * GROUND_PIXEL_WIDTH || position.y >= tiles[0].size() * GROUND_PIXEL_WIDTH) {
		return { 0, 0 };
	}

	SDL_Point blockPos = position / GROUND_PIXEL_WIDTH;

	const Ground& block = tiles[blockPos.x][blockPos.y];

	if (block.empty()) {
		return { 0, 0 };
	}

	SDL_Point tilePos = SDL_Point{ int(position.x % GROUND_PIXEL_WIDTH), int(position.y % GROUND_PIXEL_WIDTH) } / TILE_WIDTH;

	return block.getTile(tilePos.x, tilePos.y, path);
}

Player::SensorResult Player::checkSensor(const SDL_Point& position, const SDL_Point& radii, const Vector2& velocity, Mode mode, Sensor sensor, bool path, const std::vector < std::vector < Ground > >& tiles) {
	const auto [xRange, yRange, iterOp] = getRange(position, radii, mode, sensor);
	
	const auto [xStart, xEnd] = xRange;
	const auto [yStart, yEnd] = yRange;

	if (xStart < 0 || yStart < 0) {
		return SensorResult{};
	}

	auto block = SDL_Point{ xStart, yStart } / GROUND_PIXEL_WIDTH;
	if (block.x >= tiles.size() || block.y >= tiles[0].size()) {
		return SensorResult{};
	}

	const bool side = (iterOp == Direction::LEFT) || (iterOp == Direction::RIGHT);
	auto tile = (SDL_Point{ xStart, yStart } - block * GROUND_PIXEL_WIDTH) / TILE_WIDTH;
	
	std::optional< std::pair< int, double > > maxHeight;
	
	const int startTileX = xStart / TILE_WIDTH;
	const int startTileY = yStart / TILE_WIDTH;

	const int endTileX = xEnd / TILE_WIDTH;
	const int endTileY = yEnd / TILE_WIDTH;


	while (true) {
		const bool outOfBounds = (block.x < 0 || block.x >= tiles.size()) || (block.y < 0 || block.y >= tiles[0].size());

		if (!outOfBounds) {
			const Ground& currentBlock = tiles[block.x][block.y];
			int tileHeight = 0;
			bool flip = false;

			if (!currentBlock.empty()) {
				std::tie(tileHeight, flip) = Player::getHeight(tiles, block, tile, side, path, xRange, yRange);

				if (currentBlock.getFlag(tile.x, tile.y, path) & static_cast<int>(Ground::Flags::TOP_SOLID)) {
					const int topPos = -tileHeight + (tile.y + 1) * int(TILE_WIDTH) + block.y * int(GROUND_PIXEL_WIDTH);
					const int distance = std::abs(position.y + radii.y - topPos);
					const int maxOvershoot = 2 + velocity.y * Timer::getFrameTime().count();
					if (iterOp != Direction::UP || velocity.y < 0.0 || distance > maxOvershoot) {
						tileHeight = 0;
					}
				}
			}

			if (tileHeight != 0) {
				const bool isFlippedAway = ((iterOp == Direction::UP || iterOp == Direction::LEFT) ? (flip) : (!flip));
				if (isFlippedAway) {
					tileHeight = TILE_WIDTH;
				}

				const int iterTile = (side ? tile.x : tile.y);
				const int blockPos = static_cast< int >(GROUND_PIXEL_WIDTH) * (side ? block.x : block.y);
				const double angle = currentBlock.getTileAngle(tile.x, tile.y, path);
				
				if (iterOp == Direction::UP || iterOp == Direction::LEFT) {
					maxHeight.emplace((iterTile + 1) * TILE_WIDTH + blockPos - tileHeight, angle);
				}
				else {
					maxHeight.emplace((iterTile + 0) * TILE_WIDTH + blockPos + tileHeight, angle);
				}
			}
		}

		switch (iterOp) {
		case Direction::UP:
			--tile.y;
			break;
		case Direction::RIGHT:
			++tile.x;
			break;
		case Direction::DOWN:
			++tile.y;
			break;
		case Direction::LEFT:
			--tile.x;
			break;
		}

		if (tile.x == GROUND_WIDTH) {
			tile.x = 0;
			++block.x;
		}
		else if (tile.x == -1) {
			tile.x = GROUND_WIDTH - 1;
			--block.x;
		}

		if (tile.y == GROUND_WIDTH) {
			tile.y = 0;
			++block.y;
		}
		else if (tile.y == -1) {
			tile.y = GROUND_WIDTH - 1;
			--block.y;
		}

		const SDL_Point currentPos{
			static_cast< int >((xStart % TILE_WIDTH) + tile.x * TILE_WIDTH + block.x * GROUND_PIXEL_WIDTH),
			static_cast< int >((yStart % TILE_WIDTH) + tile.y * TILE_WIDTH + block.y * GROUND_PIXEL_WIDTH) };

		/*if (const SDL_Point result = directionCompare(currentPos, { xEnd, yEnd }, iterOp); result.x < 0 || result.y < 0) {
			if (maxHeight) {
				return {{ { maxHeight->first, position.y }, maxHeight->second, Side(iterOp) }};
			}
			else {
				return {};
			}
		}*/

		if (signum(xStart - xEnd) * ((xStart % static_cast< int >(TILE_WIDTH)) + tile.x * static_cast< int >(TILE_WIDTH) + block.x * static_cast< int >(GROUND_PIXEL_WIDTH)) < signum(xStart - xEnd) * xEnd) {
			if (maxHeight) {
				return {{ { maxHeight->first, position.y }, maxHeight->second, Side(iterOp) }};
			}
			else {
				return {};
			}
		}

		if (signum(yStart - yEnd) * ((yStart % static_cast< int >(TILE_WIDTH)) + tile.y * static_cast< int >(TILE_WIDTH) + block.y * static_cast< int >(GROUND_PIXEL_WIDTH)) < signum(yStart - yEnd) * yEnd) {
			if (maxHeight) {
				return {{ { position.x, maxHeight->first }, maxHeight->second, Side(iterOp) }};
			}
			else {
				return {};
			}
		}
	}
}

std::tuple< std::pair< int, int >, std::pair< int, int >, Direction > Player::getRange(const SDL_Point& position, const SDL_Point& radii, Mode mode, Sensor sensor) {
	 return [&]() -> std::tuple< std::pair< int, int >, std::pair< int, int >, Direction > {
		const auto x = position.x;
		const auto y = position.y;
		SDL_Point start = [&]() -> SDL_Point {
			switch (sensor) {
			case Sensor::A:
				return { x - radii.x, y + 36 };
			case Sensor::B:
				return { x + radii.x, y + 36 };
			case Sensor::C:
				return { x - radii.x, y - 36 };
			case Sensor::D:
				return { x + radii.x, y - 36 };
			case Sensor::E:
				return { x - 16, y + 4 };
			case Sensor::F:
				return { x + 16, y + 4 };
			}
		}();
		start = rotate90(int(mode), start, { x, y });
		SDL_Point end = [&]() -> SDL_Point {
			switch (sensor) {
			case Sensor::A:
				return { x - radii.x, y };
			case Sensor::B:
				return { x + radii.x, y };
			case Sensor::C:
				return { x - radii.x, y };
			case Sensor::D:
				return { x + radii.x, y };
			case Sensor::E:
				return { x, y + 4 };
			case Sensor::F:
				return { x, y + 4 };
			}
		}();
		end = rotate90(int(mode), end, { x, y });
		Direction dir = [&]() -> Direction {
			switch (sensor) {
			case Sensor::A:
			case Sensor::B:
				return Direction::UP;
			case Sensor::C:
			case Sensor::D:
				return Direction::DOWN;
			case Sensor::E:
				return Direction::RIGHT;
			case Sensor::F:
				return Direction::LEFT;
			}
		}();
		dir = Direction((int(mode) + int(dir)) % 4);
		return { { start.x, end.x }, { start.y, end.y }, dir };
	}();

}

std::tuple< std::pair< int, int >, std::pair< int, int >, Direction > Player::getRange(Sensor sensor) const {
	return getRange(static_cast< SDL_Point >(position), { getXRadius(), getYRadius() }, collideMode, sensor);
}

Player::SensorResult Player::checkSensor(Player::Sensor sensor, const std::vector < std::vector < Ground > >& tiles) const {
	return checkSensor(static_cast< SDL_Point >(position), { getXRadius(), getYRadius() }, velocity, collideMode, sensor, path, tiles);
}

std::optional< SDL_Point > Player::getSensorPoint(Player::Sensor sensor, const std::vector< std::vector< Ground > >& tiles) const {
	if (const auto result = checkSensor(sensor, tiles)) {
		const auto side = std::get< Side >(*result);
		if (side == Side::TOP || side == Side::BOTTOM) {
			return { { std::get<0>(std::get<0>(getRange(sensor))), std::get<0>(*result).y } }; 
		}
		else {
			return { { std::get<0>(*result).x, std::get<0>(std::get<1>(getRange(sensor))) } };
		}
	}
	else {
		return {};
	}
}

std::pair<int, bool > Player::getHeight(const std::vector< std::vector < Ground > >& ground, SDL_Point blockPosition, SDL_Point tilePosition, bool side, bool path, std::pair< int, int > xRange, std::pair< int, int > yRange) {
	const auto& block = ground[blockPosition.x][blockPosition.y];
	const auto& tile = block.getTile(tilePosition.x, tilePosition.y, path);
	const auto  tileFlags = block.getFlag(tilePosition.x, tilePosition.y, path);

	const auto blockCoord = (side ? blockPosition.y : blockPosition.x) * GROUND_PIXEL_WIDTH;
	const auto start = (side ? yRange.first : xRange.first);
	const auto heightIndex = (start - blockCoord) % TILE_WIDTH;
	//const bool mirrored = tileFlags & (side ? SDL_FLIP_VERTICAL : SDL_FLIP_HORIZONTAL);
	//const auto tileHeight = tile.getHeight(mirrored ? (TILE_WIDTH - 1 - heightIndex) : (heightIndex), side);
	
	const auto tileHeight = ::getHeight(tile, heightIndex, side ? Direction::RIGHT: Direction::DOWN);
	
	const bool flipped = tileFlags & (side ? SDL_FLIP_HORIZONTAL : SDL_FLIP_VERTICAL);

	return { tileHeight, flipped };
}

std::string Player::modeToString(Mode m) {
	switch (m) {
	case GROUND:
		return std::string("Ground");
	case RIGHT_WALL:
		return std::string("Right Wall");
	case CEILING:
		return std::string("Ceiling");
	case LEFT_WALL:
		return std::string("Left Wall");
	default:
		return std::string("Invalid");
	}
}

void Player::render(const Camera& cam) {
	if (invis)
		return;
	angle = (angle == 255.0 || angle == 1.0) ? 0.0 : angle;
	const auto pos = position - cam.getPosition();
	const double frames = Timer::getFrameTime().count() / (1000.0 / 60.0);
	displayAngle = angle;
	displayAngle += 256;
	displayAngle %= 256;
	const int rot = ((state == State::ROLLING || state == State::ROLLJUMPING) ? 0 : 45 * (displayAngle / 32.0)); //For all rotations
	const SDL_RendererFlip flip = static_cast< SDL_RendererFlip >(SDL_FLIP_HORIZONTAL & horizFlip);
	for (auto index : currentAnim) {
		using namespace animation_effects;
		AnimationEffectList effects;
		if (index == 5) {
			SDL_Point tailPos(pos);
			SDL_Point tailCenter = { 0, 0 };
			SDL_RendererFlip tailFlip;
			int tailRot = 360-90;
			if (currentAnim.size() == 2 && currentAnim[1] == 4) {
				if (velocity.x != 0.0 || velocity.y != 0.0) {
					tailRot += atan2(-velocity.y, -velocity.x) * 180.0 / M_PI;
				}
				tailFlip = static_cast < SDL_RendererFlip > (SDL_FLIP_HORIZONTAL & (velocity.x > 0.0));
			}
			else {
				tailPos.x += 2;
				//tailPos.x -= centerOffset.x + 5 + (horizFlip * 39);
				tailPos.y += (horizFlip ? 5 : 10);
				tailRot = 90 * (horizFlip ? -1 : 1);
				tailFlip = static_cast < SDL_RendererFlip >(SDL_FLIP_HORIZONTAL & !horizFlip);
			}
			effects.emplace_back(Rotation{ tailCenter, tailRot });
			animations[index]->Render(tailPos, 0, nullptr, cam.scale, tailFlip, effects);
		}
		else {
			effects.emplace_back(Rotation{ { 0, 0 }, rot });
			animations[index]->Render(static_cast< SDL_Point >(pos), 0, nullptr, cam.scale, flip, effects);
		}
	}

	if (globalObjects::debug) {
		drawing::drawPoint(globalObjects::renderer, cam, getPosition(), drawing::Color{ 0, 0, 0 }, 2);
		hitbox.render(cam, position);
	}
}

// TODO: Reimplement this functionality
/*SDL_Rect Player::getCollisionRect() const {
	switch (collideMode) {
	case GROUND:
		return { position.x + collisionRect.x, position.y + collisionRect.y, collisionRect.w, collisionRect.h };
	case LEFT_WALL:
		return { position.x - collisionRect.y - collisionRect.h, position.y + collisionRect.x, collisionRect.h, collisionRect.w };
	case CEILING:
		return { position.x + collisionRect.x, position.y - collisionRect.y - collisionRect.h, collisionRect.w, collisionRect.h };
	case RIGHT_WALL:
		return { position.x + collisionRect.y, position.y - collisionRect.x - collisionRect.w, collisionRect.h, collisionRect.w };
	}
}*/

void Player::setActCleared(bool b)
{
	actCleared = b;
	if (!b) {
		using namespace std::chrono_literals;
		animations[13]->setDelay(150ms);
		animations[13]->setLooped(false);
	}
}

bool Player::canDamageEnemy() const {
	return (!corkscrew && jumping && state != State::FLYING) || (state == State::ROLLING || state == State::ROLLJUMPING);
}

int Player::lookDirection() const {
	if (state == State::LOOKING_UP) {
		return 1;
	}
	else if (state == State::CROUCHING) {
		return -1;
	}
	else {
		return 0;
	}
}

void Player::destroyEnemy(EntityManager& manager, PhysicsEntity& entity) {
	entity.destroy();
	if (position.y > entity.getPosition().y || velocity.y < 0) {
		velocity.y -= signum(velocity.y);
	}
	else if (globalObjects::input.GetKeyState(InputComponent::JUMP)) {
		velocity.y *= -1;
	}
	else {
		velocity.y = std::max(-velocity.y, -4.0);
	}
}

void Player::walkLeftAndRight(const InputComponent& input, double thisAccel, double thisDecel, double thisFrc) {
	if (std::abs(gsp) < 2.0 && collideMode != GROUND && !controlLock.isTiming()) {
		if (64 <= angle && angle <= 192) {
			collideMode = GROUND;
			angle = 0.0;
			onGround = false;
		}
		controlLock.start();
		return;
	}
	if (getKeyState(input, InputComponent::LEFT) && !controlLock.isTiming()) {
		if (gsp <= 0.0) {
			gsp -= thisAccel;
		}
		else if (std::abs(gsp - thisDecel) != gsp - thisDecel) {
			gsp = 0.0;
		}
		else {
			gsp -= thisDecel;
		}
	}
	else if (getKeyState(input, InputComponent::RIGHT) && !controlLock.isTiming()) {
		if (gsp >= 0.0) {
			gsp += thisAccel;
		}
		else if (std::abs(gsp + thisDecel) != gsp + thisDecel) {
			gsp = 0.0;
		}
		else {
			gsp += thisDecel;
		}
	}
	else if (std::abs(gsp) < thisFrc) {
		gsp = 0.0;
	}
	else {
		gsp -= thisFrc * signum(gsp);
	}
}

void Player::updateIfWalkOrIdle(const InputComponent& input, double thisAccel, double thisDecel, double thisFrc, double slp) {
	// Perform normal actions
	walkLeftAndRight(input, thisAccel, thisDecel, thisFrc);

	using namespace player_constants::physics;

	// Do slope calculations
	if (gsp != 0.0 || (16 <= angle  && angle <= 240)) {
		const double thisSlp = slp * Timer::getFrameTime().count() / (1000.0 / 60.0);
		const double speedDifference = -thisSlp * sin(hexToRad(angle));
		gsp += speedDifference;
		if (std::abs(gsp) >= DEFAULT_TOP_SPEED) {
			gsp = signum(gsp) * DEFAULT_TOP_SPEED;
		}
	}

	// Initiate a jump
	if (input.GetKeyPress(InputComponent::JUMP) && onGround) {
		jump(false);
	};
}

void Player::updateInAir(const InputComponent & input, double thisAccel, double thisDecel, double thisFrc) {
	if (getKeyState(input, InputComponent::LEFT) && state != State::ROLLJUMPING) {
		velocity.x -= thisAccel;
	}
	if (getKeyState(input, InputComponent::RIGHT) && state != State::ROLLJUMPING) {
		velocity.x += thisAccel;
	}
	if (-4 < velocity.y && velocity.y < 0 && std::abs(velocity.x) >= 0.125) {
		velocity.x *= thisFrc;
	}
}

void Player::restrictVelocityDirection(Vector2& point, SDL_Point dir, int rotation) {
	const auto velocityDir = rotate90(rotation, dir);
	point.x = (velocityDir.x ? (velocityDir.x * std::max(velocityDir.x * point.x, 0.0)) : point.x);
	point.y = (velocityDir.y ? (velocityDir.y * std::max(velocityDir.y * point.y, 0.0)) : point.y);
}

bool Player::getKeyPress(const InputComponent& input, int key) const {
	return input.GetKeyPress(key) && !damageCountdown.isTiming();
}

bool Player::getKeyState(const InputComponent& input, int key) const {
	return input.GetKeyState(key) && !damageCountdown.isTiming();
}

SDL_Point directionCompare(SDL_Point a, SDL_Point b, Direction direction) {
	a = rotate90(static_cast< int >(direction), a);
	b = rotate90(static_cast< int >(direction), b);
	return { signum(b.x - a.x), signum(b.y - a.y) };
}

double hexToDeg(double hex) {
	return (256 - hex) * 1.40625;
}

double hexToRad(double hex) {
	return hexToDeg(hex) * M_PI / 180.0;
}

int signum(int a) {
	return (0 < a) - (a < 0);
};

int signum(double a) {
	return (0.0 < a) - (a < 0.0);
};

bool isOffsetState(const Player::State& state) {
	return (state == Player::State::ROLLING || state == Player::State::CROUCHING || state == Player::State::ROLLJUMPING);
};

bool operator< (const Player::SensorResult& a, const Player::SensorResult& b) {
	if (a && b) {
		assert(std::get<Player::Side>(*a) == std::get<Player::Side>(*b));

		const auto& posA = std::get< SDL_Point >(*a);
		const auto& posB = std::get< SDL_Point >(*b);

		switch(std::get<Player::Side>(*a)) {
		case Player::Side::TOP:
			return (posA.y > posB.y);
		case Player::Side::RIGHT:
			return (posA.x < posB.x);
		case Player::Side::BOTTOM:
			return (posA.y < posB.y);
		case Player::Side::LEFT:
			return (posA.x > posB.x);
		}
	}
	else if (a) {
		// value < nothing 
		return false;
	}
	else if (b) {
		// nothing < value
		return true;
	}
	else {
		// nothing < nothing
		return false;
	}
}

bool operator< (const Player::SensorResult& a, const SDL_Point& b) {
	if (a) {
		const auto [pos, angle, side] = *a;
		switch (side) {
		case Player::Side::TOP:
			return pos.y > b.y;
		case Player::Side::RIGHT:
			return pos.x < b.x;
		case Player::Side::BOTTOM:
			return pos.y < b.y;
		case Player::Side::LEFT:
			return pos.x > b.x;
		}
	}
	else {
		return true;
	}
}

std::ostream& operator<< (std::ostream& str, Player::Sensor sensor) {
	static const std::array< std::string, 6 > names{ "A", "B", "C", "D", "E", "F" };
	str << names[std::size_t(sensor)];
	return str;
}

std::ostream& operator<< (std::ostream& str, Direction direction) {
	static const std::array< std::string, 4 > names{ "UP", "RIGHT", "DOWN", "LEFT" }; 
	str << names[std::size_t(direction)];
	return str;
}

std::ostream& operator<< (std::ostream& str, Player::Side side) {
	static const std::array< std::string, 4 > names{ "TOP", "RIGHT", "BOTTOM", "LEFT" };
	str << names[std::size_t(side)];
	return str;
}

std::optional< std::pair< SDL_Point, double > > collideLine(SDL_Point lineBegin, int maxLength, Direction direction, const std::vector< std::vector< Ground > >& ground, bool useOneWayPlatforms, bool path) {
	const bool upDown = (direction == Direction::UP || direction == Direction::DOWN);
	SDL_Point directionVec;
	switch (direction) {
	case Direction::UP:
		directionVec = SDL_Point{  0, -1 };
		break;
	case Direction::DOWN:
		directionVec = SDL_Point{  0,  1 };
		break;
	case Direction::LEFT:
		directionVec = SDL_Point{ -1,  0 };
		break;
	case Direction::RIGHT:
		directionVec = SDL_Point{  1,  0 };
		break;
	}
	const SDL_Point lineEnd = lineBegin + directionVec * maxLength;
	SDL_Point current = lineEnd;

	const int idx = upDown ? (lineBegin.x % TILE_WIDTH) : (lineBegin.y % TILE_WIDTH);

	CollisionTile currentTile = getTile(current, ground, path);
	SDL_Point tileCorner = (current / TILE_WIDTH) * TILE_WIDTH;
	std::optional< SDL_Point > surface = surfacePos(currentTile, idx, direction);

	if (!surface) {
		return {};
	}

	auto isPast = [&]() {
		SDL_Point difference = current - lineEnd;
		int coord = upDown ? difference.y : difference.x;
		return maxLength < std::abs(coord);
	};

	SDL_Point lastCorner = tileCorner;
	do {
		lastCorner = tileCorner;
		current = tileCorner + *surface - directionVec;
		tileCorner = (current / TILE_WIDTH) * TILE_WIDTH;
		currentTile = getTile(current, ground, path);
		surface = surfacePos(currentTile, idx, direction);
	} while (surface && lastCorner != tileCorner && !isPast());

	return { { current, currentTile.getAngle(direction) } };
}
