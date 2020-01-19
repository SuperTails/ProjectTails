#include "stdafx.h"
#include <algorithm>
#include <cmath>
#include "Player.h"
#include "EntityTypes.h"
#include "Text.h"
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
			auto temp = std::make_unique<PhysicsEntity>( "RING", std::vector< char >{}, position);
			const int speed = ((i >= 16) ? 2 : 4);
			const int dir = ((i % 2 == 0) ? -1 : 1);
			const double angle = 101.25 + 22.5 * ((i % 16) / 2);
			temp->setVelocity({ dir * sin(angle * toRad) * speed, cos(angle * toRad) * speed });
			temp->setGravity(0.09375);
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
				gsp = sqrt(pow(velocity.x, 2) + pow(velocity.y, 2)) * cos(velocityAngle + hexToRad(angle));
			}
		}
		velocity.x = gsp * cos(hexToRad(angle));
		velocity.y = gsp * sin(hexToRad(angle));
	}
	

	switch (actType) {
	//Tornado
	case 1:
		if (input.getKeyState(InputComponent::KeyMap::LEFT)) {
			velocity.x = 1;
		}
		else if (input.getKeyState(InputComponent::KeyMap::RIGHT)) {
			velocity.x = 2;
		}
		else if (std::abs(velocity.x - 1.5) <= 0.05) {
			velocity.x = 1.5;
		}
		else {
			velocity.x = (0.9 * (velocity.x - 1.5)) + 1.5;
		}
		if (input.getKeyState(InputComponent::KeyMap::LEFT)) {
			velocity.y = 2;
			break;
		}
		else if (input.getKeyState(InputComponent::KeyMap::RIGHT)) {
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
			if (getKeyState(input, InputComponent::KeyMap::DOWN)) {
				state = State::CROUCHING;
				setAnimation(6);
				break;
			}
			// If pressing up, look up
			else if (getKeyState(input, InputComponent::KeyMap::UP)) {
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
			if (getKeyState(input, InputComponent::KeyMap::DOWN)) {
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
			if (getKeyPress(input, InputComponent::KeyMap::JUMP) && !corkscrew) {
				flightTime.start();
				gravity = 0.03125;
				state = State::FLYING;
				break;
			}
			// Allow for variable jump heights
			else if (!getKeyState(input, InputComponent::KeyMap::JUMP) && jumping && velocity.y < -4.0 && !corkscrew) {
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
			if (flightTime.isTiming() && getKeyPress(input, InputComponent::KeyMap::JUMP)) {
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
			if (!getKeyState(input, InputComponent::KeyMap::DOWN)) {
				state = State::IDLE;
				setAnimation(1);
				break;
			}
			// If jump is pressed, start a spindash
			else if (getKeyPress(input, InputComponent::KeyMap::JUMP)) {
				state = State::SPINDASH;
				spindash = 2.0;
				setAnimation(7);
				break;
			}

			setAnimation(6);

			break;
		case State::SPINDASH:

			// If no longer pressing down, start rolling
			if (!getKeyState(input, InputComponent::KeyMap::DOWN)) {
				state = State::ROLLING;
				setAnimation(5 + 4 * animations.size());
				std::cout << spindash << "\n";
				gsp = 8.0 + floor(spindash / 2.0);
				gsp = (horizFlip ? -gsp : gsp);
				spindash = -1.0;
				break;
			}
			// If jump is pressed, add speed
			else if (getKeyPress(input, InputComponent::KeyMap::JUMP)) {
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
					std::cout << "Player fell off of wall or ceiling\n";
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
			else if (getKeyState(input, InputComponent::KeyMap::LEFT) && !controlLock.isTiming()) {
				if (gsp >= thisDecel) {
					gsp -= thisDecel;
				}
			}
			// Normal input to the right
			else if (getKeyState(input, InputComponent::KeyMap::RIGHT) && !controlLock.isTiming()) {
				if (gsp <= thisDecel) {
					gsp += thisDecel;
				}
			}

			// Calculate slope value
			if (signum(gsp) == signum(sin(-hexToRad(angle)))) {
				// Uphill
				slp = 0.078125;
			}
			else {
				// Downhill
				slp = 0.3125;
			}

			// Slope calculations
			gsp -= sin(-hexToRad(angle)) * slp * thisFrameCount;
			
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
			if (getKeyPress(input, InputComponent::KeyMap::JUMP) && !controlLock.isTiming() && !ceilingBlocked && onGround) {
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
			if (!getKeyState(input, InputComponent::KeyMap::UP)) {
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
	velocity.x -= jmp * sin(-hexToRad(angle));
	velocity.y -= jmp * cos(-hexToRad(angle));
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

	const int yRadius = getYRadius();
	const int xRadius = getXRadius();


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
	const Direction dir = directionFromRelative(relativeDir, mode);

	if (!hitbox.getAABoundingBox()) {
		return {};
	}

	Rect box = *hitbox.getAABoundingBox();

	bool side = false;
	int startCoord = 0;
	int endCoord = 0;
	if (dir == Direction::DOWN || dir == Direction::UP) {
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

	// lhs < rhs <=> lhs lower than rhs <=> lhs is less 'up' than rhs <=> directionCompare < 0
	auto compare = [dir](auto lhs, auto rhs) {
		const Direction oppositeDir = static_cast< Direction >((static_cast< int >(dir) + 2) % 4);
		int heightComp = directionCompare(lhs.first, rhs.first, oppositeDir);
		int posComp = directionCompare(lhs.first, rhs.first, static_cast< Direction >((static_cast< int >(dir) + 1) % 4));
		
		return heightComp < 0 || (!(heightComp > 0) && posComp < 0);
	};

	auto [minElem, maxElem] = std::minmax_element(results.begin(), results.end(), compare);
	auto minPoint = *minElem, maxPoint = *maxElem;

	if (auto iter = std::remove_if(results.begin(), results.end(), [=](auto p) { return directionCompare(p.first, maxPoint.first, dir) != 0; }); iter != results.end()) {
		results.erase(iter, results.end());
	}

	double avgX = 0.0;
	double avgY = 0.0;
	for (auto result : results) {
		avgX += std::cos(hexToRad(result.second));
		avgY += std::sin(hexToRad(result.second));
	}
	avgX /= results.size();
	avgY /= results.size();

	/*double ang2 = 0.0;
	if (flat) {
		ang2 = 64.0 * static_cast< int >(mode);
	}
	else if (SDL_Point diff = minElem->first - maxElem->first; diff != SDL_Point{ 0, 0 }) {
		ang2 = std::atan2(diff.y, diff.x);
	}*/

	double averageAngle = radToHex(std::atan2(avgY, avgX));
	//double averageAngle = wrap(radToHex(ang2), 256);

	return { { maxPoint.first, averageAngle } };
}

//Returns height of A, height of B, and angle
void Player::collideGround(const std::vector < std::vector < Ground > >& tiles, std::vector < AbsoluteHitbox >& platforms, std::vector < AbsoluteHitbox >& walls) {
	onGroundPrev = onGround;
	const int xRadius = getXRadius();
	const int yRadius = getYRadius();

	Mode last_mode = collideMode;
	//Calculate which mode to be in based on angle
	collideMode = Mode((static_cast<int>(angle + 0x20) & 0xFF) >> 6);
	
	SDL_Point offset = animations[currentAnim.front()]->getOffset().value_or(SDL_Point{ 0, 0 }); 
	if (currentAnim.front() == 5) {
		offset = animations[currentAnim[1]]->getOffset().value_or(SDL_Point{ 0, 0 });
	}

	Rect rawShape{ double(-xRadius), double(-offset.y), 2.0 * xRadius, double(yRadius + offset.y) };
	rawShape = rotate90(static_cast< int >(collideMode), rawShape);
	hitbox = HitboxForm(rawShape);

	collideWall(tiles, walls, false);
	collideWall(tiles, walls, true);

	std::optional< std::pair< Point, double > > floor;
	if (auto result = findGroundHeight(tiles, getPosition(), collideMode, getHitbox(), false, getPath(), Direction::DOWN)) {
		floor.emplace(static_cast< Point >(result->first), result->second);
		if (collideMode == Mode::GROUND || collideMode == Mode::CEILING) {
			floor->first.x = getPosition().x;
		}
		else {
			floor->first.y = getPosition().y;
		}
	}

	// Check platform sensors
	if (collideMode == GROUND && velocity.y >= 0.0) {
		for (const AbsoluteHitbox& entityCol: platforms) {
			Rect entityCollision = *entityCol.hitbox.getAABoundingBox();
			const int entityRadius = entityCollision.w / 2;
			const int entityTop = entityCollision.y + 1;
			const int entityCenter = entityCollision.x + entityRadius;

			const int rightDir = position.x + xRadius - entityCenter;
			const int leftDir = position.x - xRadius - entityCenter;

			if (rightDir < -entityRadius || leftDir > entityRadius) {
				continue;
			}
			else if (!floor || directionCompare(floor->first, Point{ position.x, double(entityTop) }, Direction::DOWN) < 0) {
				floor.emplace(Point{ position.x, double(entityTop) }, 0.0);
			}
		}
	}
	
	if (onGround) {
		if (floor) {
			angle = wrap(floor->second, 256.0);

			const auto playerRadius = rotate90(static_cast< int >(collideMode), Point{ 0, double(yRadius - 1) });
			position = (floor->first) - playerRadius;
		}
		else {
			onGround = false;
			std::cout << "Player lost floor\n";
			angle = 0.0;
			collideMode = GROUND;
		}
	}
	else if (floor && velocity.y >= 0.0) {
		const int height = floor->first.y - (position.y + yRadius);
		if (height <= 2) {
			angle = std::get< double >(*floor);
			onGround = true;
			jumping = false;
			position.y += height;
		}
	}

}

//Push the player out of ceilings
void Player::collideCeilings(const std::vector< std::vector< Ground > >& tiles) {
	if (getMode() != Mode::GROUND) {
		return;
	}

	if (auto result = findGroundHeight(tiles, getPosition(), getMode(), getHitbox(), false, getPath(), Direction::UP)) {
		SDL_Point ceiling = result->first;
		ceiling.x = getPosition().x;
		const int topDistance = position.y - getAbsHitbox().hitbox.getAABoundingBox()->y;
		if (getPosition().y - topDistance < ceiling.y) {
			position.y = ceiling.y + topDistance;
			velocity.y = std::max(0.0, velocity.y);
		}
	}
}

Direction directionFromRelative(Direction rel, Mode mode) {
	// increases clockwise
	// 0 is to the right
	return static_cast< Direction >(wrap(static_cast< int >(rel) + static_cast< int >(mode), 4));
}

HitboxForm Player::getWallHitbox() const {
	Rect box = *getHitbox().getAABoundingBox();

	switch (getMode()) {
	case Mode::CEILING:
		box.y += 10;
	case Mode::GROUND:
		box.h -= 10;
		break;
	case Mode::LEFT_WALL:
		box.x += 10;
	case Mode::RIGHT_WALL:
		box.w -= 10;
		break;
	}

	return HitboxForm{ box };

}

// TODO: Entity wall collision code, reimplement
/*
if (collideMode == Mode::GROUND) {
	for (const AbsoluteHitbox& entityCol : walls) {
		Rect entityCollision = entityCol.box.getBox();
		if (position.y + getYRadius() < entityCollision.y || position.y + 4 > entityCollision.y + entityCollision.h) {
			continue;
		}

		if (position.x < entityCollision.x + entityCollision.w / 2) {
			const Point pos{ entityCollision.x, position.y };
			if (!rWall || directionCompare(*rWall, pos, rWallDir) > 0) {
				rWall.emplace(pos);
			}
		}
		else {
			const Point pos{ entityCollision.x + entityCollision.w, position.y };
			if (!lWall || directionCompare(*lWall, pos, lWallDir) > 0) {
				lWall.emplace(pos);
			}
		}
	}
}
*/

void Player::collideWall(const std::vector< std::vector< Ground > >& tiles, const std::vector< AbsoluteHitbox >& walls, bool left) {
	const Direction wallDir = left ? Direction::LEFT : Direction::RIGHT;
	const HitboxForm wallBox = getWallHitbox();

	std::optional< Point > wall{};
	if (auto result = findGroundHeight(tiles, getPosition(), getMode(), wallBox, false, getPath(), wallDir)) {
		wall.emplace(static_cast< Point >(result->first));
	}
	
	// TODO: Entity collisions
	
	if (wall) {
		const Rect box = *wallBox.getAABoundingBox();
		const Point corner1 = getPosition() + Point{ box.x, box.y };
		const Point corner2 = corner1 + Point{ box.w, box.h };
		
		// directionCompare(corner, wall, oppDir) < 0 when corner is *less* intrusive than wall
		// Pick the edge diff that is closest to the player/most 'intrusive'
		const Direction oppDir = directionFromRelative(static_cast< Direction >((static_cast< int >(wallDir) + 2) % 4), collideMode);
		double comp = std::min(directionCompare(corner1, *wall, oppDir), directionCompare(corner2, *wall, oppDir));
		// comp is positive when the wall is *outside* the player's bounding box, negative when *intersecting* with the player
		
		if (comp <= 0.0) {
			Vector2 push = directionVector(oppDir);
			push.x *= -comp;
			push.y *= -comp;

			position += push;
			restrictVelocityDirection(velocity, { (left ? 1 : -1), 0 }, static_cast< int >(collideMode));
		}
	}

}

CollisionTile getTile(SDL_Point position, const std::vector< std::vector< Ground > >& tiles, bool path) {
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

// One-way tile algorithm:
/*if (currentBlock.getFlag(tile.x, tile.y, path) & static_cast<int>(Ground::Flags::TOP_SOLID)) {
	const int topPos = -tileHeight + (tile.y + 1) * int(TILE_WIDTH) + block.y * int(GROUND_PIXEL_WIDTH);
	const int distance = std::abs(position.y + radii.y - topPos);
	const int maxOvershoot = 2 + velocity.y * Timer::getFrameTime().count();
	if (iterOp != Direction::UP || velocity.y < 0.0 || distance > maxOvershoot) {
		tileHeight = 0;
	}
}*/

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
	if (damageCountdown.isTiming() && (damageCountdown.timeRemaining().count() % 132) > 66) {
		return;
	}

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
		getWallHitbox().render(cam, position);

		SDL_SetRenderDrawColor(globalObjects::renderer, 255, 127, 0, SDL_ALPHA_OPAQUE);
		const int maxLength = 32;
		const std::array< Point, 4 > offsets = { Point{ 0, maxLength }, { 0, -maxLength }, { maxLength, 0 }, { -maxLength, 0 } };
		for (Point offset : offsets) {
			drawing::drawLine(globalObjects::renderer, cam,
				getPosition(),
				getPosition() + offset,
				drawing::Color{ 255, 127, 0 }
			);
		}

	}
}

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
	else if (globalObjects::input.getKeyState(InputComponent::KeyMap::JUMP)) {
		velocity.y *= -1;
	}
	else {
		velocity.y = std::max(-velocity.y, -4.0);
	}
}

void Player::walkLeftAndRight(const InputComponent& input, double thisAccel, double thisDecel, double thisFrc) {
	if (std::abs(gsp) < 2.0 && collideMode != GROUND && !controlLock.isTiming()) {
		if (64 <= angle && angle <= 192) {
			std::cout << "Player fell off of slope\n";
			collideMode = GROUND;
			angle = 0.0;
			onGround = false;
		}
		controlLock.start();
		return;
	}
	if (getKeyState(input, InputComponent::KeyMap::LEFT) && !controlLock.isTiming()) {
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
	else if (getKeyState(input, InputComponent::KeyMap::RIGHT) && !controlLock.isTiming()) {
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
		const double speedDifference = -thisSlp * sin(-hexToRad(angle));
		gsp += speedDifference;
		if (std::abs(gsp) >= DEFAULT_TOP_SPEED) {
			gsp = signum(gsp) * DEFAULT_TOP_SPEED;
		}
	}

	// Initiate a jump
	if (input.getKeyPress(InputComponent::KeyMap::JUMP) && onGround) {
		jump(false);
	};
}

void Player::updateInAir(const InputComponent & input, double thisAccel, double thisDecel, double thisFrc) {
	if (getKeyState(input, InputComponent::KeyMap::LEFT) && state != State::ROLLJUMPING) {
		velocity.x -= thisAccel;
	}
	if (getKeyState(input, InputComponent::KeyMap::RIGHT) && state != State::ROLLJUMPING) {
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

bool Player::getKeyPress(const InputComponent& input, InputComponent::KeyMap key) const {
	return input.getKeyPress(key) && !damageCountdown.isTiming();
}

bool Player::getKeyState(const InputComponent& input, InputComponent::KeyMap key) const {
	return input.getKeyState(key) && !damageCountdown.isTiming();
}

int directionCompare(SDL_Point a, SDL_Point b, Direction direction) {
	a = rotate90(-static_cast< int >(direction), a);
	b = rotate90(-static_cast< int >(direction), b);
	return b.y - a.y;
}

double directionCompare(Point a, Point b, Direction direction) {
	a = rotate90(-static_cast< int >(direction), a);
	b = rotate90(-static_cast< int >(direction), b);
	return b.y - a.y;
}

double hexToDeg(double hex) {
	return hex / 256.0 * 360.0;
}

double hexToRad(double hex) {
	return hex / 256.0 * (2.0 * M_PI);
}

double radToHex(double rad) {
	return rad / (2.0 * M_PI) * 256.0;
}

double degToHex(double deg) {
	return deg / 360.0 * 256.0;
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

// For example, if direction is UP, returns true if a is less 'up' than b
bool lessThanInDirection(SDL_Point a, SDL_Point b, Direction direction) {
	a = rotate90(-static_cast< int >(direction), a);
	b = rotate90(-static_cast< int >(direction), b);
	return a.y > b.y;
}

bool lessThanInDirection(Point a, Point b, Direction direction) {
	a = rotate90(-static_cast< int >(direction), a);
	b = rotate90(-static_cast< int >(direction), b);
	return a.y > b.y;
}

bool shouldUseOneWays(Vector2 velocity, Direction dir) {
	return velocity.y >= 0 && dir == Direction::DOWN;
}

std::ostream& operator<< (std::ostream& str, Direction direction) {
	static const std::array< std::string, 4 > names{ "UP", "RIGHT", "DOWN", "LEFT" }; 
	str << names[std::size_t(direction)];
	return str;
}

std::optional< std::pair< SDL_Point, double > > collideLine(SDL_Point lineBegin, int maxLength, Direction direction, const std::vector< std::vector< Ground > >& ground, bool useOneWayPlatforms, bool path) {
	const bool upDown = (direction == Direction::UP || direction == Direction::DOWN);
	SDL_Point directionVec = static_cast< SDL_Point >(directionVector(direction));

	const SDL_Point lineEnd = lineBegin + directionVec * maxLength;
	const int idx = upDown ? (lineBegin.x % TILE_WIDTH) : (lineBegin.y % TILE_WIDTH);

	auto nextSurface = [&ground, &path, &useOneWayPlatforms, &idx, &direction](auto curr) -> std::optional< SDL_Point > {
		CollisionTile tile = getTile(curr, ground, path);
		if (!useOneWayPlatforms && (tile.flags & static_cast< int >(Ground::Flags::TOP_SOLID))) {
			tile = { 0, 0 };
		}
		return surfacePos(tile, idx, direction);
	};

	SDL_Point current = lineEnd;
	std::optional< SDL_Point > surface = nextSurface(current);

	if (!surface) {
		return {};
	}

	auto isPast = [&upDown, &lineEnd, &maxLength](auto curr) {
		const SDL_Point difference = curr - lineEnd;
		const int coord = upDown ? difference.y : difference.x;
		return maxLength < std::abs(coord);
	};

	SDL_Point lastPos = current;
	do {
		lastPos = std::exchange(current, (current / TILE_WIDTH) * TILE_WIDTH + *surface - directionVec);
		surface = nextSurface(current);
	} while (surface && (lastPos / TILE_WIDTH) != (current / TILE_WIDTH) && !isPast(current));

	double angle = getTile(current, ground, path).getAngle(direction);
	if (getTile(current, ground, path).getIndex() == 0) {
		angle = getTile(current + directionVec * TILE_WIDTH, ground, path).getAngle(direction);
	}

	return { { current, angle } };
}

Point directionVector(Direction dir) {
	std::array< Point, 4 > dirs = { Vector2{ 0, -1 }, { 1, 0 }, { 0, 1 }, { -1, 0 } };

	return dirs[static_cast< std::size_t >(dir)];
}
