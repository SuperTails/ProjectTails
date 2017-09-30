#include "stdafx.h"
#include <algorithm>
#include <cmath>
#include "EntityTypes.h"
#include "Player.h"
#include "Functions.h"
#include "Ground.h"
#include "CollisionTile.h"
#include "Miscellaneous.h"
#include "InputComponent.h"
#include "Camera.h"

Player::Player() :
	horizFlip(false),
	path(0),
	controlLock(0),
	spindash(-1),
	flightTime(8000),
	corkscrew(false),
	ceilingBlocked(false),
	displayAngle(0.0),
	actCleared(false),
	state(State::IDLE),
	rings(0),
	damageCountdown(0)
{
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

	position = PRHS_Rect{ 20, 0, 120, 64, 0 };
	collisionRect = SDL_Rect{ 0, 19, 120, 30 };

	customData = NoCustomData{};
}

void Player::setActType(unsigned char aType) {
	actType = aType;
	if (actType == 1) {
		setAnimation(0);
		gravity = 0;
	}
	else if (actType == 2) {
		collisionRect = { 0, 0, 36, 31 };
		setAnimation(1);
		gravity = 0.21875;
	}
}

double Player::getAngle() {
	return hexToDeg(angle);
}

void Player::takeDamage(EntityManager& manager, int enemyCenterX) {
	double toRad = M_PI / 180;
	double angle = 101.25;
	bool n = false;
	int speed = 4;
	flightTime.stop();
	gravity = 0.21875;
	onGround = false;
	jumping = false;

	if (damageCountdown == 0) {
		for (int i = 0; (i < rings) && (i < 32); ++i) {
			const auto& ringProperties = entity_property_data::getEntityTypeData("RING");
			PhysStruct p { "RING", {}, { position.x, position.y, 16, 16 }, true };
			auto temp = std::make_unique<PhysicsEntity>(p);
			doublePoint vel { -1 * sin(angle * toRad) * speed, cos(angle * toRad) * speed };
			temp->setVelocity(vel);
			temp->setGravity(0.09375);
			temp->shouldSave = false;
			if (n) {
				temp->setVelocity({ vel.x * -1, vel.y });
				angle += 22.5;
			}
			n = !n;
			if (i + 1 == 16) {
				speed = 2;
				angle = 101.25;
			}

			manager.AddEntity(std::move(temp));
		}
		rings = 0;
		damageCountdown = 2000;
		velocity.y = -4;
		velocity.x = 2 * signum(position.x - enemyCenterX);
		velocity.x = (velocity.x == 0) ? 1.0 : velocity.x;
		setAnimation(12);
	}
}

int Player::getYRadius() const {
	int yRadius = 0;
	applyToCurrentAnimations([&](const auto& anim) { yRadius = std::max(yRadius, anim->GetSize().y - anim->getOffset()->y); });
	if (currentAnim == 5 + 4 * animations.size()) {
		return 14;
	}
	else if (currentAnim == 5 + 7 * animations.size()) {
		return 21;
	}
	return yRadius;
}

void Player::hitEnemy(EntityManager& manager, PhysicsEntity& enemy) {
	if (canDamageEnemy()) {
		destroyEnemy(manager, enemy);
	}
	else {
		takeDamage(manager, enemy.getCollisionRect().x + enemy.getCollisionRect().y / 2 );
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
		velocity.x = gsp * cos(hexToRad(angle));
		velocity.y = -1 * gsp * sin(hexToRad(angle));
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
		velocity.y = 0.9 * (velocity.y);
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

		double thisAdd = 2.0 * thisFrameCount;

		double anim_steps = std::max(8.0 - std::abs(gsp), 1.0);

		controlLock = std::max<int>(0, controlLock - deltaTime.count());

		using namespace std::chrono_literals;

		switch (state) {
		case State::IDLE:

			// If pressing down, crouch.
			if (input.GetKeyState(InputComponent::DOWN)) {
				state = State::CROUCHING;
				position.y += ROLL_VERTICAL_OFFSET;
				break;
			}
			// If pressing up, look up
			else if (input.GetKeyState(InputComponent::UP)) {
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
			if (input.GetKeyState(InputComponent::DOWN)) {
				if (std::abs(gsp) < 0.5) {
					state = State::CROUCHING;
					position.y += ROLL_VERTICAL_OFFSET;
					gsp = 0.0;
					break;
				}
				else {
					state = State::ROLLING;
					break;
				}
			}

			updateIfWalkOrIdle(input, thisAccel, thisDecel, thisFrc, slp);

			if (gsp == 0) {
				state = State::IDLE;
				break;
			}

			setAnimation(2 + (std::abs(gsp) >= 0.9 * top));
			animations[currentAnim]->setDelay(Animation::DurationType(static_cast<int>(anim_steps * FRAME_TIME_MS)));

			if (gsp < 0) {
				horizFlip = true;
			}
			else if (gsp > 0) {
				horizFlip = false;
			}

			break;
		case State::JUMPING:

			// Start flying
			if (input.GetKeyPress(InputComponent::JUMP) && !corkscrew) {
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
			else {
				// If there is flight time remaining, check for the jump button being pressed
				if (input.GetKeyPress(InputComponent::JUMP)) {
					if (velocity.y >= -1) {
						gravity = -0.125;
					}
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
			if (!input.GetKeyState(InputComponent::DOWN)) {
				state = State::IDLE;
				position.y -= ROLL_VERTICAL_OFFSET;
				break;
			}
			// If jump is pressed, start a spindash
			else if (input.GetKeyPress(InputComponent::JUMP)) {
				state = State::SPINDASH;
				position.y -= ROLL_VERTICAL_OFFSET;
				spindash = thisAdd;
				break;
			}

			setAnimation(6);

			break;
		case State::SPINDASH:

			// If no longer pressing down, start rolling
			if (!input.GetKeyState(InputComponent::DOWN)) {
				state = State::ROLLING;
				position.y += ROLL_VERTICAL_OFFSET;
				std::cout << spindash << "\n";
				gsp = 8.0 + floor(spindash) / 2;
				gsp *= (horizFlip * -2 + 1);
				spindash = -1.0;
			}
			// If jump is pressed, add speed
			else if (input.GetKeyPress(InputComponent::JUMP)) {
				spindash += thisAdd;
			}


			{
			// Make sure spindash decays
			const double thisDecay = 256.0 / thisFrameCount;
			spindash -= double((floor(spindash * 8)) / thisDecay);
			}

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
				controlLock = 400;
			}
			// Normal input to the left
			else if (input.GetKeyState(InputComponent::LEFT) && !controlLock) {
				if (gsp >= thisDecel) {
					gsp -= thisDecel;
				}
			}
			// Normal input to the right
			else if (input.GetKeyState(InputComponent::RIGHT) && !controlLock) {
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
			gsp -= sin(hexToDeg(angle) * M_PI / 180.0) * slp * thisFrameCount;
			
			// If friction would cause a sign change, stop
			if (onGround) {
				if (std::abs(gsp) < thisFrc / 2) {
					gsp = 0.0;
					state = State::IDLE;
					position.y -= ROLL_VERTICAL_OFFSET;
				}
				else {
					gsp -= thisFrc * signum(gsp) / 2.0;
				}
			}

			// Going too slow should cause us to stop rolling
			if (std::abs(gsp) < 0.5 && onGround) {
				position.y -= ROLL_VERTICAL_OFFSET;
				state = (gsp == 0 ? State::IDLE : State::WALKING);
				break;
			}

			// If the jump key gets pressed then initiate a rolljump
			if (input.GetKeyPress(InputComponent::JUMP) && !controlLock && !ceilingBlocked && onGround) {
				onGround = false;
				state = State::ROLLJUMPING;
				collideMode = GROUND;
				jmp = 6.5;
				velocity.x -= jmp * sin(hexToDeg(angle) * M_PI / 180.0);
				velocity.y -= jmp * cos(hexToDeg(angle) * M_PI / 180.0);
				angle = 0.0;
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
			if (!input.GetKeyState(InputComponent::UP)) {
				state = State::IDLE;
				break;
			}

			setAnimation(10);

			break;
		default:
			throw "Invalid Player state!";
		}

		// Reset jump
		if (onGround) {
			jmp = 0.0;
		}
		// Update in air
		else {
			double thisAccel = thisFrameCount * accel;
			double thisDecel = thisFrameCount * decel;
			double thisFrc = pow(frc, thisFrameCount);
			updateInAir(input, thisAccel, thisDecel, thisFrc);
		}

		// Stop the player if they are damaged
		if (damageCountdown > 0) {
			gsp = 0.0;
			damageCountdown = std::max<int>(0, damageCountdown - deltaTime.count());
		}
		
		// Overriding default animations
		if (actCleared) {
			setAnimation(13);
			gsp = 0.0;
			velocity.x = 0.0;
		}
		else if (damageCountdown) {
			setAnimation(12);
		}
		else if (corkscrew) {
			setAnimation(11);
		}

		// Reset act-clear animation
		if (currentAnim == 13 && (animations[currentAnim]->GetLooped() || animations[currentAnim]->getFrame() == 2)) {
			animations[13]->SetFrame(2);
			animations[13]->setDelay(-1ms);
		}

		if (state != State::ROLLING) { 
			std::clamp(gsp, -16.0, 16.0);
		}

		std::clamp(velocity.y, -16.0, 16.0);
		}
	}

	//2 seconds
	if (damageCountdown > 0) {
		if ((damageCountdown % 132) > 66) {
			invis = true;
		}
		else {
			invis = false;
		}
		damageCountdown = std::max<int>(0, damageCountdown - deltaTime.count());
	}
	else {
		invis = false;
	}

	if (std::abs(velocity.y) < 1e-10) {
		velocity.y = 0.0;
	}


	handleCollisions(tiles, manager);

	PhysicsEntity::update(this, &manager);

	if (onGround) {
		velocity.y -= gravity * (Timer::getFrameTime().count() * 1000.0 / 60.0);
	}
}

bool Player::addRing(int num) {
	if (damageCountdown < 100) {
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
	Animation& currentAnimation = *animations[currentAnim % animations.size()];
	Animation& nextAnimation = *animations[index % animations.size()];
	if (currentAnimation.getNumFrames() == nextAnimation.getNumFrames()) {
		//nextAnimation.SetFrame(currentAnimation.getFrame());
		nextAnimation.synchronize(currentAnimation);
	}
	const int oldYRadius = getYRadius();
	currentAnim = index;
	position.y += oldYRadius - getYRadius();
}

void Player::handleCollisions(std::vector < std::vector < Ground > >& tiles, EntityManager& manager) {
	bool hurt = false;
	int damageCenter = -1;
	std::vector < SDL_Rect > platforms;
	std::vector < SDL_Rect > walls;

	const int yRadius = getYRadius();

	int xRadius = 9 - 2 * (state == State::ROLLING || state == State::ROLLJUMPING);

	SDL_Point offset = *animations[currentAnim % animations.size()]->getOffset(); 
	collisionRect = SDL_Rect{ -xRadius, -offset.y, 2 * xRadius, yRadius + offset.y }; 
	SDL_Rect playerCollide = getCollisionRect();

	while (!collisionQueue.empty()) {
		PhysicsEntity& entity = *collisionQueue.front();
		
		const auto& entityKey = entity.getKey();
		const auto& entityTypeData = entity_property_data::getEntityTypeData(entityKey);

		SDL_Rect entityCollision = entity.getCollisionRect();

		SDL_Point entityCenter;

		SDL_Point dir;

		Side collisionSide = getCollisionSide(collisionRect, entityCollision);

		if (canBePushedAgainst(entity)) {
			platforms.push_back(entityCollision);	
			walls.push_back(entityCollision);
		}
		else if (canBeStoodOn(entity)) {
			platforms.push_back(entityCollision);
		}

		if (entityKey == "RING" && addRing()) {
			std::cout << "Collision with ring\n";
			std::get<Ring>(entity.getCustom()).pickedUp = true;
		}
		else if (entityKey == "PATHSWITCH") {
			const bool prevPath = path;
			std::get<Pathswitch>(entity.getCustom()).setPath(path);
			if (path != prevPath) {
				std::cout << std::boolalpha << "Path changed to " << path << " from previous " << prevPath << std::noboolalpha << "\n";
			}
		}
		else if (entityKey == "SPRINGYELLOW") {
			setOnGround(false);
			corkscrew = true;
			std::get<Spring>(entity.getCustom()).bounceEntity(entity, static_cast<PhysicsEntity&>(*this));
		}
		else if (entityKey == "RINGMONITOR") {
			if (canDamageEnemy()) {
				hitEnemy(manager, entity);
				addRing(10);
			}
		}
		else if (entity_property_data::isHazard(entityKey)) {
			std::cout << "Collision with hazard: " << entityKey << "\n";
			entityCenter = getXY(entityCollision) + (SDL_Point{ entityCollision.w, entityCollision.h } / 2);

			if (entity_property_data::isEnemy(entityKey)) {
				hitEnemy(manager, entity);
			}
			else {
				takeDamage(manager, entity.getCollisionRect().x + entity.getCollisionRect().w / 2);
			}
		}
		/*switch () { 
		case SPIKES:
			dir = calcRectDirection(entityCollision);
			if (dir.y < 0 && dir.x >= -entityCollision.w / 2 && dir.x <= entityCollision.w / 2) {
				hurt = true;
				damageCenter = entityCollision.x + entityCollision.w;
			}
			break;
		case MONITOR:
			if (canDamage()) {
				addRing(10);
				velocity.y *= 0.9;
				entity.destroy();
			}
			break;
		case GOALPOST:
			if (entity.getCustom(1) == 0.0) {
				entity.setCustom(0, 300.0);
				//SoundHandler::actFinish();
			}
			break;
		default:
			throw "Invalid collision type.";
			break;
		}*/

		collisionQueue.pop();
	}

	if (hurt) {
		takeDamage(manager, damageCenter);
	}

	collideGround(tiles, platforms, walls);
}

//Returns height of A, height of B, and angle
std::string Player::collideGround(const std::vector < std::vector < Ground > >& tiles, std::vector < SDL_Rect >& platforms, std::vector < SDL_Rect >& walls) {
	onGroundPrev = onGround;
	int height1(-1), height2(-1), height3(-1), height4(-1), wallHeightLeft(-1), wallHeightRight(-1);
	double ang1, ang2, ang3, ang4;
	double sensorDir(-1);
	int h;
	int yRadius = getYRadius();
	int xRadius(9 - 2 * (state == State::ROLLING || state == State::ROLLJUMPING));

	bool onGround1, onGround2, onGround3, onGround4, wallCollideLeft, wallCollideRight;
	bool topOnly[6];
	std::memset(topOnly, 0, 6);

	const int topOnlyMaxOvershoot = 2 + std::max(0, position.y - previousPosition.y);

	Mode last_mode = collideMode;
	//Calculate which mode to be in based on angle
	collideMode = Mode((static_cast<int>(angle + 0x20) & 0xFF) >> 6);

	static Timer logData{ 16 };

	if (globalObjects::input.GetKeyPress(InputComponent::M)) {
		std::cout << "Panic\n";
		logData.start();
	}

	//Check for left floor:
	h = checkSensor('A', tiles, ang1, &topOnly[0]);
	height1 = abs(h);
	onGround1 = (h >= 0);

	//Check for right floor:
	h = checkSensor('B', tiles, ang2, &topOnly[1]);
	height2 = abs(h);
	onGround2 = (h >= 0);

	if (logData.isTiming()) {
		if (logData.update()) {
			logData.stop();
		}
		std::cout << "position: " << position.x % GROUND_PIXEL_WIDTH << ", " << position.y % GROUND_PIXEL_WIDTH << "\n";
		std::cout << "heights: " << height1 % GROUND_PIXEL_WIDTH << ", " << height2 % GROUND_PIXEL_WIDTH << "\n";
		std::cout << "angles: " << int(ang1) << ", " << int(ang2) << "\n";
		std::cout << "collide mode: " << modeToString(collideMode) << "\n\n";
	}

	std::string output;

	int platformHeight = -1;
	bool platformOnGround = false;

	// Check platform sensors
	if (collideMode == GROUND && velocity.y >= 0.0) {
		for (SDL_Rect& entityCollision : platforms) {
			const int entityRadius = entityCollision.w / 2;
			const int entityCenter = entityCollision.x + entityRadius;
			const int entityTop = entityCollision.y + 1;

			const int rightDir = position.x + xRadius - entityCenter;
			const int leftDir = position.x - xRadius - entityCenter;

			const int sensorDistance = entityTop - position.y - yRadius;

			// Continue if not close enough to the platform
			if (sensorDistance < -topOnlyMaxOvershoot || sensorDistance > topOnlyMaxOvershoot) {
				continue;
			}
			else if (rightDir < -entityRadius || leftDir > entityRadius) {
				// No collision
				continue;
			}
			else if (platformHeight == -1 || platformHeight - (position.y + yRadius) > sensorDistance || !platformOnGround) {
				platformHeight = entityTop;
				platformOnGround = true;
			}
		}
	}

	auto getCollisionHeight = [this,yRadius,topOnlyMaxOvershoot](int rawHeight, bool ground, bool top) {
		rawHeight -= position.y + yRadius;
		if (!ground || (top && (velocity.y < 0.0 || rawHeight < -topOnlyMaxOvershoot || rawHeight > topOnlyMaxOvershoot))) {
			return std::optional<int>{};
		}
		else {
			return std::optional<int>{ rawHeight + position.y + yRadius }; 
		}
	};

	if (collideMode == GROUND && platformOnGround) {
		auto sensor1 = getCollisionHeight(height1, onGround1, topOnly[0]);
		auto sensor2 = getCollisionHeight(height2, onGround2, topOnly[1]);

		auto sensorPlatform = getCollisionHeight(platformHeight, platformOnGround, true);

		// Only valid sensor is the platform sensor
		if (!(sensor1 || sensor2)) {
			height1 = sensorPlatform.value_or(-1);
			onGround1 = sensorPlatform.has_value();
			topOnly[0] = true;

			height2 = -1;
			onGround2 = false;
		}
		else {
			/*if (sensor1) {
				*sensor1 += position.y + yRadius;
			}
			if (sensor2) {
				*sensor2 += position.y + yRadius;
			}*/
			
			if (!sensor1 || (sensor2 && *sensor1 > *sensor2)) {
				sensor1 = sensor2;
			}

			height1 = sensor1.value_or(-1);
			onGround1 = sensor1.has_value();

			height2 = sensorPlatform.value_or(-1);
			onGround2 = sensorPlatform.has_value();
		}
	}

	if (onGround) {
		if (onGround1 || onGround2) {
			switch (collideMode) {
			case GROUND:
				//Get dist of sensor
				height1 -= position.y + yRadius;
				height2 -= position.y + yRadius;

				//Make sure d1 is closer
				if ((height1 < height2 && onGround1) || !onGround2) {
					angle = ang1;
				}
				else {
					angle = ang2;
					std::swap(height1, height2);
					std::swap(topOnly[0], topOnly[1]);
				}

				if (topOnly[0] && (height1 < -topOnlyMaxOvershoot || velocity.y < 0.0)) {
					onGround = false;
					angle = 0.0;
					collideMode = GROUND;
				}
				else {
					//Stick to ground by subtracting distance
					position.y += height1;
					if (topOnly[0]) {
						velocity.y = std::min(velocity.y, 0.0);
					}
				}
				break;
			case LEFT_WALL:
				height1 -= position.x - yRadius;
				height2 -= position.x - yRadius;
				height1 *= -1;
				height2 *= -1;
				if ((height1 < height2 && onGround1) || !onGround2) {
					angle = ang1;
				}
				else {
					angle = ang2;
					std::swap(height1, height2);
				}
				position.x -= height1;
				break;
			case CEILING:
				height1 -= position.y - yRadius;
				height2 -= position.y - yRadius;
				height1 *= -1;
				height2 *= -1;
				if ((height1 < height2 && onGround1) || !onGround2) {
					angle = ang1;
				}
				else {
					angle = ang2;
					std::swap(height1, height2);
				}
				position.y -= height1;
				break;
			case RIGHT_WALL:
				height1 -= position.x + yRadius;
				height2 -= position.x + yRadius;
				if ((height1 < height2 && onGround1) || !onGround2) {
					angle = ang1;
				}
				else {
					angle = ang2;
					std::swap(height1, height2);
				}
				position.x += height1;
				break;
			}
		}
		else {
			onGround = false;
			angle = 0.0;
			collideMode = GROUND;
		}
	}
	else {
		if (velocity.y >= 0.0) {
			height1 -= position.y + yRadius;
			height2 -= position.y + yRadius;
			if ((height1 < height2 && onGround1) || !onGround2) {
				angle = ang1;
			}
			else {
				angle = ang2;
				std::swap(height1, height2);
				std::swap(topOnly[0], topOnly[1]);
			}
			if (height1 <= 2 && (onGround1 || onGround2)) {
				if (!topOnly[0] || (height1 >= -topOnlyMaxOvershoot)) {
					onGround = true;
					jumping = false;
					position.y += height1;
				}
			}
		}
	}

	//Check for left ceiling
	h = checkSensor('C', tiles, ang3, &topOnly[2]);
	height3 = abs(h);
	onGround3 = (h >= 0);

	//Check for right ceiling
	h = checkSensor('D', tiles, ang4, &topOnly[3]);
	height4 = abs(h);
	onGround4 = (h >= 0);

	

	//Push the player out of ceilings
	if ((onGround3 && !topOnly[2]) || (onGround4 && !topOnly[3])) {
		int topDistance = ((state == State::ROLLING | state == State::ROLLJUMPING) ? 14 : 10);
		switch (collideMode) {
		case GROUND:
			height3 -= position.y - topDistance;
			height4 -= position.y - topDistance;
			if ((height4 > height3 && onGround3) || !onGround4) {
				std::swap(height3, height4);
			}
			ceilingBlocked = (height3 >= -5);
			if (height3 > -3) {
				position.y += height3 + 3;
				velocity.y = std::max(0.0, velocity.y);
			}
			break;
		case LEFT_WALL:
			height3 -= position.x + topDistance;
			height4 -= position.x + topDistance;
			height3 *= -1;
			height4 *= -1;
			if ((height4 > height3 && onGround3) || !onGround4) {
				std::swap(height3, height4);
			}
			ceilingBlocked = (height3 >= -5);
			if (height3 > -3) {
				position.x -= height3 + 3;
				velocity.x = std::min(0.0, velocity.x);
			}
			break;
		case CEILING:
			height3 -= position.y + topDistance;
			height4 -= position.y + topDistance;
			height3 *= -1;
			height4 *= -1;
			if ((height4 > height3 && onGround3) || !onGround4) {
				std::swap(height3, height4);
			}
			ceilingBlocked = (height3 >= -5);
			if (height3 > -3) {
				position.y -= height3 + 3;
				velocity.y = std::min(0.0, velocity.y);
			}
			break;
		case RIGHT_WALL:
			height3 -= position.x - topDistance;
			height4 -= position.x - topDistance;
			if ((height4 > height3 && onGround3) || !onGround4) {
				std::swap(height3, height4);
			}
			ceilingBlocked = (height3 >= -5);
			if (height3 > -3) {
				position.x += height3 + 3;
				velocity.x = std::max(0.0, velocity.x);
			}
			break;
		}
		
	}

	//Check for left wall
	h = checkSensor('E', tiles, sensorDir, &topOnly[4]);
	wallHeightLeft = abs(h);
	wallCollideLeft = (h >= 0);

	sensorDir = 1;

	//Check for right wall
	h = checkSensor('E', tiles, sensorDir, &topOnly[5]);
	wallHeightRight = abs(h);
	wallCollideRight = (h >= 0);

	if (collideMode == Mode::GROUND) {
		for (const SDL_Rect& entityCollision : walls) {
			if (position.y + yRadius < entityCollision.y || position.y + 4 > entityCollision.y + entityCollision.h) {
				break;
			}

			int distance = entityCollision.x - position.x;
			if (position.x < entityCollision.x + entityCollision.w / 2) {
				if (distance <= 10 && (!wallCollideRight || (wallHeightRight - position.x) > distance)) {
					wallCollideRight = true;
					wallHeightRight = entityCollision.x;
				}
			}
			else {
				distance += entityCollision.w;
				if (distance >= -10 && (!wallCollideLeft || (wallHeightLeft - position.x) < distance)) {
					wallCollideLeft = true;
					wallHeightLeft = entityCollision.x + entityCollision.w;
				}
			}
		}
	}

	output += "Walls: " + std::to_string(wallCollideLeft) + " " + std::to_string(wallCollideRight);

	sensorDir = -1;
	int temph1 = checkSensor('E', tiles, sensorDir, &topOnly[2]);
	sensorDir = 1;
	int temph2 = checkSensor('E', tiles, sensorDir, &topOnly[3]);


	//Push the player out of walls
	switch (collideMode) {
	case GROUND:
		wallHeightLeft -= position.x;
		wallHeightRight -= position.x;
		if (wallHeightRight <= 10 && wallCollideRight && !topOnly[5]) {
			position.x -= 10 - wallHeightRight;
			velocity.x = std::min(velocity.x, 0.0);
			gsp = std::min(gsp, 0.0);
		}
		if (wallHeightLeft >= -10 && wallCollideLeft && !topOnly[4]) {
			position.x += 10 + wallHeightLeft;
			velocity.x = std::max(velocity.x, 0.0);
			gsp = std::max(gsp, 0.0);
		}
		break;
	case LEFT_WALL:
		wallHeightLeft -= position.y;
		wallHeightRight -= position.y;
		if (wallHeightRight <= 10 && wallCollideRight) {
			position.y -= 10 - wallHeightRight;
			velocity.y = std::min(velocity.y, 0.0);
			gsp = std::min(gsp, 0.0);
		}
		if (wallHeightLeft >= -10 && wallCollideLeft) {
			position.y += 10 + wallHeightLeft;
		}
		break;
	case CEILING:
		wallHeightLeft -= position.x;
		wallHeightRight -= position.x;
		if (wallHeightRight >= -10 && wallCollideRight) {
			position.x += 10 + wallHeightRight;
			velocity.x = std::max(velocity.x, 0.0);
			gsp = std::min(gsp, 0.0);
		}
		if (wallHeightLeft <= 10 && wallCollideLeft) {
			position.x -= 10 - wallCollideLeft;
		}
		break;
	case RIGHT_WALL:
		wallHeightLeft -= position.y;
		wallHeightRight -= position.y;
		if (wallHeightRight >= -10 && wallCollideRight) {
			position.y += 10 + wallHeightRight;
			velocity.y = std::max(velocity.y, 0.0);
			gsp = std::min(gsp, 0.0);
		}
		if (wallHeightLeft <= 10 && wallCollideLeft) {
			position.y -= 10 - wallCollideLeft;
		}
		break;
	}

	return output;
}

const std::vector < double > angles = { 0.0, 128.0, 192.0, 64.0 };

int Player::checkSensor(char sensor, const std::vector < std::vector < Ground > >& tiles, double& ang, bool* isTopOnly) {
	int xEnd, xStart, yEnd, yStart;
	int c = position.x;
	int d = position.y;
	int xRadius = 9 - (2 * (state == State::ROLLING || state == State::ROLLJUMPING));
	enum class direction { UP, DOWN, LEFT, RIGHT };
	direction iterOp; //0 = y--, 1 = y++, 2 = x--, 3 = x++
	bool side = false;

	if (isTopOnly) {
		*isTopOnly = false;
	}

	switch (5 * collideMode + sensor - 'A') {
	case 0: //Ground A
	case 13: //Ceiling D
		xStart = xEnd = c - xRadius;
		yStart = d + 36;
		yEnd = d;
		iterOp = (direction)0; // y--
		break;
	case 1: //Ground B
	case 12: //Ceiling C
		xStart = xEnd = c + xRadius;
		yStart = d + 36;
		yEnd = d;
		iterOp = (direction)0; // y--
		break;
	case 2: //Ground C
	case 11: //Ceiling B
		xStart = xEnd = c - xRadius;
		yStart = d - 36;
		yEnd = d;
		iterOp = (direction)1; // y++
		break;
	case 3: //Ground D
	case 10: //Ceiling A
		xStart = xEnd = c + xRadius;
		yStart = d - 36;
		yEnd = d;
		iterOp = (direction)1; // y++
		break;
	case 5: //LW A
	case 18: //RW D
		xStart = c - 36;
		xEnd = c;
		yStart = yEnd = d - xRadius;
		iterOp = (direction)3; // x++
		break;
	case 6: //LW B
	case 17: //RW C
		xStart = c - 36;
		xEnd = c;
		yStart = yEnd = d + xRadius;
		iterOp = (direction)3; // x++
		break;
	case 7: //LW C
	case 16: //RW B
		xStart = c + 36;
		xEnd = c;
		yStart = yEnd = d - xRadius;
		iterOp = (direction)2; // x--
		break;
	case 8: //LW D
	case 15: //RW A
		xStart = c + 36;
		xEnd = c;
		yStart = yEnd = d + xRadius;
		iterOp = (direction)2; // x--
		break;
	case 4: //Ground E
		xStart = c + 16 * ang;
		xEnd = c;
		yStart = yEnd = d + 4;
		iterOp = (direction)static_cast< int >(2.5 - ang / 2.0);
		break;
	case 9: //LW E
		xStart = xEnd = c - 4;
		yStart = d + 16 * ang;
		yEnd = d;
		iterOp = (direction)static_cast < int >(0.5 - ang / 2.0);
		break;
	case 14: //Ceil E
		xStart = c - 16 * ang;
		xEnd = c;
		yStart = yEnd = d - 4;
		iterOp = (direction)static_cast < int >(2.5 + ang / 2.0);
		break;
	case 19: //RW E
		xStart = xEnd = c + 4;
		yStart = d - 16 * ang;
		yEnd = d;
		iterOp = (direction)static_cast <int>(0.5 + ang / 2.0);
		break;
	}

	ang = 0.0;

	if (xStart < 0 || yStart < 0) {
		return -1;
	}

	side = (int(iterOp) - 2) >= 0;
	int blockX = xStart / (TILE_WIDTH * GROUND_WIDTH);
	int blockY = yStart / (TILE_WIDTH * GROUND_WIDTH);
	if (blockX >= tiles.size()) {
		return -1;
	}
	else if (blockY >= tiles[0].size()) {
		return -1;
	}
	int count = 0;
	int tileX = (xStart - blockX * TILE_WIDTH * GROUND_WIDTH) / TILE_WIDTH;
	int tileY = (yStart - blockY * TILE_WIDTH * GROUND_WIDTH) / TILE_WIDTH;
	bool flip = false;
	
	int maxHeight = -1;
	
	const int startTileX = xStart / TILE_WIDTH;
	const int startTileY = yStart / TILE_WIDTH;

	const int endTileX = xEnd / TILE_WIDTH;
	const int endTileY = yEnd / TILE_WIDTH;


	while (true) {
		bool flip = false;

		int tileHeight = 0;
		
		bool outOfBounds = (blockX < 0 || blockX >= tiles.size()) || (blockY < 0 || blockY >= tiles[0].size());

		if (!outOfBounds) {
			const Ground& block = tiles[blockX][blockY];

			if (!block.empty()) {
				tileHeight = Player::getHeight(tiles, SDL_Point{ blockX, blockY }, SDL_Point{ tileX, tileY }, side, path, xStart, xEnd, yStart, yEnd, flip);

				if (block.getFlag(tileX, tileY, path) & static_cast<int>(Ground::Flags::TOP_SOLID)) {
					if (isTopOnly) {
						if (iterOp == direction::UP) {
							*isTopOnly = true;
						}
						else {
							tileHeight = 0;
						}
					}
				}

			}

			if (tileHeight != 0) {
				ang = block.getTileAngle(tileX, tileY, path);
			}
		}


		if (tileHeight != 0) {
			bool isFlippedAway = false;

			switch (iterOp) {
			case direction::UP:
			case direction::LEFT:
				isFlippedAway = flip;
				break;
			case direction::DOWN:
			case direction::RIGHT:
				isFlippedAway = !flip;
				break;
			}

			if (isFlippedAway) {
				tileHeight = TILE_WIDTH;
			}
			
			if (iterOp == direction::UP || iterOp == direction::LEFT) {
				maxHeight =  ((side ? tileX : tileY) + 1) * TILE_WIDTH + static_cast < int >(GROUND_PIXEL_WIDTH) * (side ? blockX : blockY) - tileHeight;
			}
			else {
				maxHeight = ((side ? tileX : tileY) + 0) * TILE_WIDTH + static_cast < int > (GROUND_PIXEL_WIDTH) * (side ? blockX : blockY) + tileHeight;
			}
		}

		count++;
		switch (iterOp) {
		case direction::UP:
			--tileY;
			break;
		case direction::DOWN:
			++tileY;
			break;
		case direction::LEFT:
			--tileX;
			break;
		case direction::RIGHT:
			++tileX;
			break;
		}

		if (tileX == GROUND_WIDTH) {
			tileX = 0;
			++blockX;
		}
		else if (tileX == -1) {
			tileX = GROUND_WIDTH - 1;
			--blockX;
		}

		if (tileY == GROUND_WIDTH) {
			tileY = 0;
			++blockY;
		}
		else if (tileY == -1) {
			tileY = GROUND_WIDTH - 1;
			--blockY;
		}


		if (signum(xStart - xEnd) * ((xStart % static_cast < int >(TILE_WIDTH)) + tileX * static_cast < int >(TILE_WIDTH) + blockX * static_cast < int >(GROUND_PIXEL_WIDTH)) < signum(xStart - xEnd) * xEnd) {
			return maxHeight;
		}

		if (signum(yStart - yEnd) * ((yStart % static_cast < int >(TILE_WIDTH)) + tileY * static_cast < int >(TILE_WIDTH) + blockY * static_cast < int >(GROUND_PIXEL_WIDTH)) < signum(yStart - yEnd) * yEnd) {
			return maxHeight;
		}
	}
}

int Player::getHeight(const std::vector < std::vector < Ground > >& ground, SDL_Point blockPosition, SDL_Point tilePosition, bool side, bool path, int xStart, int xEnd, int yStart, int yEnd, bool& flip) {
	flip = false;

	int h;
	const auto& block = ground[blockPosition.x][blockPosition.y];
	const auto& tile = block.getTile(tilePosition.x, tilePosition.y, path);
	const auto& tileFlags = block.getFlag(tilePosition.x, tilePosition.y, path);

	if (side) {
		if (tileFlags & SDL_FLIP_VERTICAL) {
			// Tile is flipped, so set the current height to the flipped heightMap
			h = tile.getHeight(TILE_WIDTH - 1 - ((yStart - blockPosition.y * TILE_WIDTH * GROUND_WIDTH) % TILE_WIDTH), side);
		}
		else {
			// Tile is not flipped, set the current height normally.
			h = tile.getHeight((yStart - blockPosition.y * TILE_WIDTH * GROUND_WIDTH) % TILE_WIDTH, side);
		}
		if (tileFlags & SDL_FLIP_HORIZONTAL) {
			flip = true;
		}
	}
	else {
		if (tileFlags & SDL_FLIP_HORIZONTAL) {
			// Tile is flipped, so set the current height to the flipped heightMap
			h = tile.getHeight(TILE_WIDTH - 1 - ((xStart - blockPosition.x * TILE_WIDTH * GROUND_WIDTH) % TILE_WIDTH), side);
		}
		else {
			// Tile is not flipped, set the current height normally.
			h = tile.getHeight((xStart - blockPosition.x * TILE_WIDTH * GROUND_WIDTH) % TILE_WIDTH, side);
		}
		if (tileFlags & SDL_FLIP_VERTICAL) {
			flip = true;
		}
	}

	return h;
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

void Player::render(SDL_Rect& cam, double screenRatio) {
	if (invis)
		return;
	angle = (angle == 255.0 || angle == 1.0) ? 0.0 : angle;
	SDL_Rect pos = getRelativePos(cam);
	double frames = Timer::getFrameTime().count() / (1000.0 / 60.0);
	displayAngle = angle;
	displayAngle += 256;
	displayAngle %= 256;
	int rot = ((state == State::ROLLING || state == State::ROLLJUMPING) ? 0 : 45 * (displayAngle / 32.0)); //For all rotations
	//const int numRotations = 10;
	//int rot = (360 / numRotations) * ((displayAngle + 128 / numRotations) / (256 / numRotations));
	SDL_RendererFlip flip = static_cast<SDL_RendererFlip>(SDL_FLIP_HORIZONTAL & horizFlip);
	int temp = currentAnim;

	using namespace animation_effects;


	do {
		AnimationEffectList effects;
		if (temp % animations.size() == 5) {
			SDL_Rect tailPos = pos;
			SDL_Point tailCenter = { 0, 0 };
			SDL_RendererFlip tailFlip;
			int tailRot = 360-90;
			if (temp == 5 + 4 * animations.size()) {
				if (velocity.x != 0.0 || velocity.y != 0.0) {
					tailRot += atan2(-velocity.y, -velocity.x) * 180.0 / M_PI;
				}
				tailFlip = static_cast < SDL_RendererFlip > (SDL_FLIP_HORIZONTAL & (velocity.x > 0.0));
			}
			else {
				//tailPos.x -= centerOffset.x + 5 + (horizFlip * 39);
				//tailPos.y += 24;
				tailRot = 90 * (horizFlip ? -1 : 1);
				tailFlip = static_cast < SDL_RendererFlip >(SDL_FLIP_HORIZONTAL & !horizFlip);
			}
			effects.emplace_back(Rotation{ { 0, 0 }, tailRot });
			animations[temp % animations.size()]->Render(getXY(tailPos), 0, nullptr, 1.0 / screenRatio, tailFlip, effects);
		}
		else {
			effects.emplace_back(Rotation { { 0, 0 }, rot });
			animations[temp % animations.size()]->Render(getXY(pos), 0, nullptr, 1.0 / screenRatio, flip, effects);
		}
		temp /= animations.size();
	} while (temp != 0);
}

SDL_Rect Player::getCollisionRect() const {
	switch (collideMode) {
	case GROUND:
		return SDL_Rect{ position.x + collisionRect.x, position.y + collisionRect.y, collisionRect.w, collisionRect.h };
	case LEFT_WALL:
		return SDL_Rect{ position.x - collisionRect.y - collisionRect.h, position.y + collisionRect.x, collisionRect.h, collisionRect.w };
	case CEILING:
		return SDL_Rect{ position.x + collisionRect.x, position.y - collisionRect.y - collisionRect.h, collisionRect.w, collisionRect.h };
	case RIGHT_WALL:
		return SDL_Rect{ position.x + collisionRect.y, position.y - collisionRect.x - collisionRect.w, collisionRect.h, collisionRect.w };
	default:
		return SDL_Rect{ -1, -1, -1, -1 };
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
	SDL_Point entityCenter = getXY(entity.getCollisionRect()) + (SDL_Point{ entity.getCollisionRect().w, entity.getCollisionRect().h } / 2);
	if (position.y > entityCenter.y || velocity.y < 0) {
		velocity.y -= signum(velocity.y);
	}
	else {
		if (globalObjects::input.GetKeyState(InputComponent::JUMP)) {
			velocity.y *= -1;
		}
		else {
			velocity.y = std::max(-1.0 * velocity.y, -1.0);
		}
	}
}

void Player::walkLeftAndRight(const InputComponent& input, double thisAccel, double thisDecel, double thisFrc) {
	if (std::abs(gsp) < 2.0 && collideMode != GROUND && !controlLock) {
		if (angle >= 64 && angle <= 192) {
			collideMode = GROUND;
			angle = 0.0;
			onGround = false;
		}
		controlLock = 400;
	}
	if (input.GetKeyState(InputComponent::LEFT) && !controlLock) {
		if (gsp <= 0.0) {
			gsp -= thisAccel;
		}
		else {
			if (std::abs(gsp - thisDecel) != gsp - thisDecel) {
				gsp = 0.0;
			}
			else {
				gsp -= thisDecel;
			}
		}
	}
	else if (input.GetKeyState(InputComponent::RIGHT) && !controlLock) {
		if (gsp >= 0.0) {
			gsp += thisAccel;
		}
		else {
			if (std::abs(gsp + thisDecel) != gsp + thisDecel) {
				gsp = 0.0;
			}
			else {
				gsp += thisDecel;
			}
		}
	}
	else {
		if (std::abs(gsp) < thisFrc) {
			gsp = 0.0;
		}
		else {
			gsp -= thisFrc * signum(gsp);
		}
	}
}

void Player::updateIfWalkOrIdle(const InputComponent& input, double thisAccel, double thisDecel, double thisFrc, double slp) {
	// Perform normal actions
	walkLeftAndRight(input, thisAccel, thisDecel, thisFrc);

	using namespace player_constants::physics;

	// Do slope calculations
	if (gsp != 0.0 || (angle >= 16 && angle <= 240)) {
		const double thisSlp = slp * Timer::getFrameTime().count() / (1000.0 / 60.0);
		const double speedDifference = -thisSlp * sin(hexToRad(angle));
		gsp += speedDifference;
		if (std::abs(gsp) >= DEFAULT_TOP_SPEED) {
			gsp = signum(gsp) * DEFAULT_TOP_SPEED;
		}
	}

	// Initiate a jump
	if (input.GetKeyPress(InputComponent::JUMP) && onGround) {
		onGround = false;
		jumping = true;
		state = State::JUMPING;
		collideMode = GROUND;
		jmp = 6.5;
		velocity.x -= jmp * sin(hexToRad(angle));
		velocity.y -= jmp * cos(hexToRad(angle));
		angle = 0.0;
	};
}

void Player::updateInAir(const InputComponent & input, double thisAccel, double thisDecel, double thisFrc) {
	if (input.GetKeyState(InputComponent::LEFT) && state != State::ROLLJUMPING) {
		velocity.x -= thisAccel;
	}
	if (input.GetKeyState(InputComponent::RIGHT) && state != State::ROLLJUMPING) {
		velocity.x += thisAccel;
	}
	if (velocity.y < 0 && velocity.y > -4 && std::abs(velocity.x) >= 0.125) {
		velocity.x *= thisFrc;
	}
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
