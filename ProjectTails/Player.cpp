#include "stdafx.h"
#include "Player.h"



Player::Player() :
	accel(0.046875),
	decel(0.5),
	frc(accel),
	top(6.0),
	slp(0.125),
	horizFlip(false),
	path(0),
	controlLock(0),
	spindash(-1),
	looking(0),
	rolling(false),
	flying(-1),
	corkscrew(false),
	ceilingBlocked(false),
	displayAngle(0.0),
	platform(nullptr),
	wall(nullptr),
	actCleared(false)
{
	loaded = true;
	centerOffset = { -25, -11 };
}

void Player::SetActType(int aType) {
	actType = aType;
	if (actType == 1) {
		currentAnim = 0;
		gravity = 0;
	}
	else if (actType == 2) {
		collisionRect = { 0, 0, 36, 31 };
		currentAnim = 1;
		gravity = 0.21875;
		accel = 0.046875;
		decel = 0.5;
		frc = accel;
		top = 6;
	}
}

std::vector<std::unique_ptr<PhysicsEntity>>::iterator Player::Damage(std::vector < std::unique_ptr < PhysicsEntity > >& entities, std::vector < PhysStruct >& phys_paths, std::vector < PhysProp >& props, std::vector<std::unique_ptr<PhysicsEntity>>::iterator& iter, int enemyX, SDL_Window* window) {
	double toRad = M_PI / 180;
	double angle = 101.25;
	bool n = false;
	int speed = 4;
	__int64 dist = std::distance(entities.begin(), iter);
	if (damageCountdown == 0) {
		for (int i = 0; (i < rings) && (i < 32); i++) {
			PhysStruct p = { {position.x, position.y, 256, 256}, props[1], (int)(phys_paths.size() - 1), std::vector < char >() };
			entities.emplace_back(new PhysicsEntity(p));
			doublePoint vel = { -1 * sin(angle * toRad) * speed, cos(angle * toRad) * speed };
			entities.back()->setVelocity(vel);
			entities.back()->setGravity(.09375);
			entities.back()->setCustom(0, 4267.0);
			entities.back()->num = -1;
			if (n) {
				entities.back()->setVelocity({ vel.x * -1, vel.y });
				angle += 22.5;
			}
			n = !n;
			if (i + 1 == 16) {
				speed = 2;
				angle = 101.25;
			}
		}
		int tempRings = rings;
		rings = 0;
		damageCountdown = 2000;
		velocity.y = -4;
		velocity.x = 2 * signum(position.x - enemyX);
		velocity.x = (velocity.x == 0) ? 1.0 : velocity.x;
		currentAnim = 12;
		return entities.begin() + dist;
	}
	return iter;
}

std::vector<std::unique_ptr<PhysicsEntity>>::iterator Player::hitEnemy(std::vector < std::unique_ptr < PhysicsEntity > >& entities, std::vector<std::unique_ptr<PhysicsEntity>>::iterator currentEntity, std::vector < PhysStruct >& phys_paths, std::vector < PhysProp >& props, SDL_Window* window, SDL_Point enemyCenter) {
	if (!canDamage()) {
		velocity.y = -4;
		velocity.x = 2 * signum(position.x - enemyCenter.x);
		flying = -1;
		gravity = 0.21875;
		onGround = false;
		jumping = false;
		velocity.x = velocity.x ? velocity.x : 2;
		currentAnim = 12;
		return Damage(entities, phys_paths, props, currentEntity, enemyCenter.x, window);
	}
	else {
		if (position.y > enemyCenter.y || velocity.y < 0) {
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
		return currentEntity;
	}
}

void Player::UpdateP(Camera& cam) {
	InputComponent& input = globalObjects::input;
	last_time = time;
	time = SDL_GetTicks();
	if (!(time - last_time)) {
		return;
	}
	if (onGround || platform) {
		if ((!onGroundPrev && onGround) || (!prevOnPlatform && platform)) {
			if (int(angle) < 0x0F || int(angle) > 0xF0) {
				gsp = velocity.x;
			}
		}
		velocity.x = gsp * cos(hexToDeg(angle) * M_PI / 180.0);
		velocity.y = -1 * gsp * sin(hexToDeg(angle) * M_PI / 180.0);
		if (angle == 255.0) {
			angle = 0.0;
		}
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
		else if (abs(velocity.x - 1.5) <= 0.05) {
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
		else if (abs(velocity.y) <= 0.5) {
			velocity.y = 0;
			break;
		}
		velocity.y = 0.9 * (velocity.y);
		break;
	//Normal act
	case 2:
		if (position.x <= 10) {
			position.x = 10;
			gsp = std::max(0.0, gsp);
			velocity.x = std::max(0.0, velocity.x);
		}
		if (damageCountdown > 0) {
			gsp = 0.0;
			damageCountdown = std::max(0, damageCountdown - int(time - last_time));
		}
		else if (onGround || platform) {
			//On ground
			if (platform)
				velocity.y = std::min(0.0, velocity.y);
			flying = -1;
			accel = 0.046875;
			decel = rolling ? 0.125 : 0.5;
			frc = accel;
			top = rolling ? 16 : 6;
			jmp = 0;
			gravity = 0.21875;
			corkscrew = false;
			controlLock = std::max(0, controlLock - int(time - last_time));
			double thisAccel(((time - last_time) / (1000.0 / 60.0)) * accel);
			double thisDecel(((time - last_time) / (1000.0 / 60.0)) * decel);
			double thisFrc(((time - last_time) / (1000.0 / 60.0)) * frc);
			if (rolling) {
				if (abs(gsp) < 2.0 && collideMode != GROUND) {
					if (collideMode == CEILING || (angle >= 0x40 && angle <= 0xC0)) {
						collideMode = GROUND;
						angle = 0.0;
						onGround = false;
					}
					else {
						onGround = true;
					}
					controlLock = 400;
				}
				else if (input.GetKeyState(InputComponent::LEFT) && !controlLock) {
					if (gsp >= thisDecel) {
						gsp -= thisDecel;
					}
				}
				else if (input.GetKeyState(InputComponent::RIGHT) && !controlLock) {
					if (gsp <= thisDecel) {
						gsp += thisDecel;
					}
				}
				if (signum(gsp) == signum(sin(hexToDeg(angle) * M_PI / 180.0))) {
					//Uphill
					slp = 0.078125;
				}
				else {
					//Downhill
					slp = 0.3125;
				}

				double thisSlp(sin(hexToDeg(angle) * M_PI / 180.0) * slp * (time - last_time) / (1000.0 / 60.0));
				gsp -= thisSlp;
					
				if (abs(gsp) < thisFrc / 2) {
					gsp = 0.0;
					rolling = false;
					looking = 0;
				}
				else {
					gsp -= thisFrc * signum(gsp) / 2.0;
				}
				if (abs(gsp) <= 0.5) {
					rolling = false;
					looking = 0;
				}

				if (!input.GetKeyState(InputComponent::JUMP)) {
					spinDebounce = false;
				}

				if (input.GetKeyState(InputComponent::JUMP) && !controlLock && !spinDebounce && !ceilingBlocked) {
					onGround = false;
					platform = false;
					jumping = true;
					spinDebounce = true;
					collideMode = GROUND;
					jmp = 6.5;
					velocity.x -= jmp * sin(hexToDeg(angle) * M_PI / 180.0);
					velocity.y -= jmp * cos(hexToDeg(angle) * M_PI / 180.0);
					angle = 0.0;
				}
			}
			else if (looking == 0) {
				//Perform normal actions
				slp = 0.125;
				spinDebounce = false;
				if (abs(gsp) < 2.0 && collideMode != GROUND && !controlLock) {
					if (collideMode == CEILING) {
						collideMode = GROUND;
						angle = 0.0;
						onGround = false;
					}
					controlLock = 400;
				}
				if (input.GetKeyState(InputComponent::LEFT) && !controlLock) {
					if (gsp == 0) {
						//velocity.x -= thisAccel;
						gsp -= thisAccel;
					}
					else if (gsp < 0) {
						//velocity.x -= thisAccel;
						gsp -= thisAccel;
					}
					else {
						if (abs(gsp - thisDecel) != gsp - thisDecel) {
							gsp = 0;
						}
						else {
							gsp -= thisDecel;
						}
					}
				}
				else if (input.GetKeyState(InputComponent::RIGHT) && !controlLock) {
					if (gsp == 0) {
						gsp += thisAccel;
					}
					else if (gsp > 0) {
						gsp += thisAccel;
					}
					else {
						if (abs(gsp + thisDecel) != gsp + thisDecel) {
							gsp = 0;
						}
						else {
							gsp += thisDecel;
						}
					}
				}
				else {
					if (abs(gsp) < thisFrc) {
						gsp = 0;
					}
					else {
						gsp -= thisFrc * signum(gsp);
					}
				}
				if (gsp != 0 || collideMode != GROUND) {
					double thisSlp((time - last_time) / (1000.0 / 60.0) * slp);
					gsp -= thisSlp * sin(hexToDeg(angle) * M_PI / 180.0);
				}

				if (input.GetKeyState(InputComponent::JUMP) && (onGround || platform)) {
					onGround = false;
					platform = false;
					jumping = true;
					spinDebounce = true;
					collideMode = GROUND;
					jmp = 6.5;
					velocity.x -= jmp * sin(hexToDeg(angle) * M_PI / 180.0);
					velocity.y -= jmp * cos(hexToDeg(angle) * M_PI / 180.0);
					angle = 0.0;
				};

				//If pressing down...
				if (input.GetKeyState(InputComponent::DOWN)) {
					//..and not moving, then crouch.
					if (gsp == 0.0) {
						//Crouch
						looking = -1;
					}
					else if(abs(gsp) > 0.5) {
						rolling = true;
					}
				}
				if (input.GetKeyState(InputComponent::UP) && gsp == 0.0) {
					looking = 1;
				}
			}
			else if(looking == -1){
				double thisDecay(256.0 / ((time - last_time) / (1000.0 / 60.0)));
				double thisAdd(((time - last_time) / (1000.0 / 60.0)) * 2.0);
				if (!input.GetKeyState(InputComponent::DOWN)) {
					if (spindash != -1) {
						gsp = 8.0 + floor(spindash) / 2;
						gsp *= (horizFlip * -2 + 1);
						spindash = -1;
						rolling = true;
						top = 16;
					}
					looking = 0;
				}
				//Down is still being held
				else {
					if (input.GetKeyState(InputComponent::JUMP)) {
						if (spindash == -1)
							spindash = 0;
						if (!spinDebounce) {
							spindash += thisAdd;
							spinDebounce = true;
						}
					}
					else {
						spinDebounce = false;
					}
					if (spindash > 0.0) {
						spindash -= double((floor(spindash) * 8) / thisDecay);
					}
				}
			}
			else if (looking == 1) {
				if (!input.GetKeyState(InputComponent::UP)) {
					looking = 0;
				}
			}
		}
		//Flying
		else if (flying != -1) {
			accel = 0.09375;
			if (velocity.y < -1) {
				gravity = 0.03125;
			}
			frc = 0.96875;
			double thisAccel = ((time - last_time) / (1000.0 / 60.0)) * accel;
			double thisDecel = ((time - last_time) / (1000.0 / 60.0)) * decel;
			double thisFrc = pow(frc, (time - last_time) / (1000.0 / 60.0));
			if (input.GetKeyState(InputComponent::LEFT)) {
				velocity.x -= thisAccel;
			}
			if (input.GetKeyState(InputComponent::RIGHT)) {
				velocity.x += thisAccel;
			}
			if (flying > 0.0) {
				if (input.GetKeyState(InputComponent::JUMP)) {
					if (!spinDebounce && velocity.y >= -1) {
						gravity = -0.125;
						spinDebounce = true;
					}
				}
				else {
					spinDebounce = false;
				}
				flying = std::max(0.0, flying - (time - last_time));
			}
			horizFlip = velocity.x < 0;
			if (velocity.y < 0 && velocity.y > -4 && abs(velocity.x) >= 0.125) {
				velocity.x *= thisFrc;
			}
		}
		//Jumping/Falling
		else {
			accel = 0.09375;
			decel = accel;
			frc = 0.96875;
			gravity = 0.21875;
			top = 6;
			double thisAccel = ((time - last_time) / (1000.0 / 60.0)) * accel;
			double thisDecel = ((time - last_time) / (1000.0 / 60.0)) * decel;
			double thisFrc = pow(frc, (time - last_time) / (1000.0 / 60.0));
			if (input.GetKeyState(InputComponent::LEFT) && !rolling) {
				velocity.x -= thisAccel;
			}
			if (input.GetKeyState(InputComponent::RIGHT) && !rolling) {
				velocity.x += thisAccel;
			}
			if (velocity.y < 0 && velocity.y > -4 && abs(velocity.x) >= 0.125) {
				velocity.x *= thisFrc;
			}
			if (!input.GetKeyState(InputComponent::JUMP) && jumping && velocity.y < -4.0 && !corkscrew) {
				velocity.y = -4.0;
			}
			if (input.GetKeyState(InputComponent::JUMP) && !rolling && !corkscrew) {
				if (!spinDebounce && jumping) {
					flying = 8000.0;
					gravity = 0.03125;
					spinDebounce = true;
				}
			}
			else {
				spinDebounce = false;
			}
		}

		if (onGround || platform) {
			jmp = 0;
		}
			
			
		//Animations
		double anim_steps = std::max(8 - abs(gsp), 1.0);
		//Change animation based on speed
		if (actCleared) {
			currentAnim = 13;
			gsp = 0.0;
			velocity.x = 0.0;
		}
		else if (damageCountdown) {
			currentAnim = 12;
		}
		else if (flying > 0.0) {
			currentAnim = 8;
		}
		else if (flying == 0.0) {
			currentAnim = 9;
		}
		else if (spindash != -1) {
			currentAnim = 5 + 7 * animations.size();
			animations[5]->SetDelay(60);
		}
		else if (looking == -1) {
			currentAnim = 6;
		}
		else if (looking == 1) {
			currentAnim = 10;
		}
		else if (corkscrew) {
			currentAnim = 11;
		}
		else if (!(onGround || platform) && jumping || rolling) {
			currentAnim = 5 + 4 * animations.size();
			animations[5]->SetDelay(128);
		}
		else if (gsp == 0) {
			currentAnim = 1;
		}
		else if (abs(gsp) < top * 0.9) {
			currentAnim = 2;
			animations[currentAnim]->SetDelay(anim_steps * 1000.0 / 60.0);
		}
		else {
			currentAnim = 3;
			animations[currentAnim]->SetDelay(anim_steps * 1000.0 / 60.0);
		}
		
		if (flying >= 0.0) {
			centerOffset.y = -12;
		}
		if (spindash != -1) {
			centerOffset = { -47, -4 };
		}
		else if (looking == -1) {
			centerOffset = { -26, -2 };
			if (horizFlip) {
				centerOffset.x = -10;
			}
		}
		else if (!(onGround || platform) && jumping || rolling) {
			centerOffset = { -14, -14 };
		}
		else if (gsp == 0 || looking == 1) {
			if (horizFlip) {
				centerOffset = { -9, -11 };
			}
			else {
				centerOffset = { -27, -11 };
			}
		} else if (gsp < 0) {
			centerOffset = { -20, -11 };
			horizFlip = true;
		}
		else if (gsp > 0) {
			centerOffset = { -29, -11 };
			horizFlip = false;
		}

		if (currentAnim == 13 && (animations[currentAnim]->GetLooped() || animations[currentAnim]->getFrame() == 2)) {
			animations[13]->SetFrame(2);
			animations[13]->SetDelay(-1);
		}
			
		//End normal act
	}
	

	if (abs(gsp) > top) {
		gsp = top * signum(gsp);
	}

	if (abs(velocity.y) > 16.0) {
		velocity.y = 16.0 * signum(velocity.y);
	}

	//2 seconds
	if (damageCountdown > 0) {
		if ((damageCountdown % 132) > 66) {
			invis = true;
		}
		else {
			invis = false;
		}
		damageCountdown = (int)std::max((double)(damageCountdown - (int)(time - last_time)), 0.0);
	}
	else {
		invis = false;
	}

	this->Update(false);
}

bool Player::AddRing(int num) {
	if (damageCountdown < 100) {
		rings += num;
		return true;
	}
	return false;
}

//Returns height of A, height of B, and angle
std::string Player::CollideGround(std::vector < std::vector < Ground > >& tiles) {
	onGroundPrev = onGround || platform;
	int cx(position.x);
	int cy(position.y);
	int blockX((cx - 9) / (TILE_WIDTH * GROUND_WIDTH)), blockY((cy + 36) / (TILE_WIDTH * GROUND_WIDTH));
	int height1(-1), height2(-1), height3(-1), height4(-1), wallHeightLeft(-1), wallHeightRight(-1);
	double ang1, ang2, ang3, ang4;
	int yMin(cy);
	int yMax(cy + 36);
	double sensorDir(-1);
	int h;
	int y_radius(20 - 6 * rolling);
	int x_radius(9 - (rolling << 1));

	collisionRect = SDL_Rect{ -1 * x_radius, centerOffset.y, 2 * x_radius, 30 };

	bool onGround1, onGround2, onGround3, onGround4, wallCollideLeft, wallCollideRight;

	Mode last_mode = collideMode;
	//Calculate which mode to be in based on angle
	collideMode = Mode((static_cast<int>(angle + 0x20) & 0xFF) >> 6);

	if (globalObjects::input.GetKeyPress(InputComponent::X))
		std::cout << "Panic\n";

	//Check for left floor:
	h = checkSensor('A', tiles, ang1);
	height1 = abs(h);
	onGround1 = (h >= 0);

	//Check for right floor:
	h = checkSensor('B', tiles, ang2);
	height2 = abs(h);
	onGround2 = (h >= 0);

	std::string output(std::to_string(position.x));
	output += " ";
	output += std::to_string(position.y);

	if (platform) {
		prevOnPlatform = true;
		SDL_Rect objCollide = (*platform)->getCollisionRect();
		SDL_Point dir = calcRectDirection(objCollide);
		position.y -= dir.y + objCollide.h / 2;
		if (dir.x < -objCollide.w / 2 || dir.x > objCollide.w / 2)
			platform = nullptr;
		return output;
	}

	if (onGround) {
		if (onGround1 || onGround2) {
			switch (collideMode) {
			case GROUND:
				//Get dist of sensor
				height1 -= position.y + y_radius;
				height2 -= position.y + y_radius;
				//Make sure d1 is closer
				if (height1 < height2 || !onGround2) {
					angle = ang1;
				}
				else {
					angle = ang2;
					std::swap(height1, height2);
				}
				//Stick to ground by subtracting distance
				position.y += height1;
				break;
			case LEFT_WALL:
				height1 -= position.x - y_radius;
				height2 -= position.x - y_radius;
				height1 *= -1;
				height2 *= -1;
				if (height1 < height2 || !onGround2) {
					angle = ang1;
				}
				else {
					angle = ang2;
					std::swap(height1, height2);
				}
				position.x -= height1;
				break;
			case CEILING:
				height1 -= position.y - y_radius;
				height2 -= position.y - y_radius;
				height1 *= -1;
				height2 *= -1;
				if (height1 < height2 || !onGround2) {
					angle = ang1;
				}
				else {
					angle = ang2;
					std::swap(height1, height2);
				}
				position.y -= height1;
				break;
			case RIGHT_WALL:
				height1 -= position.x + y_radius;
				height2 -= position.x + y_radius;
				if (height1 < height2 || !onGround2) {
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
			angle = 0;
			collideMode = GROUND;
		}
	}
	else {
		if (velocity.y > 0) {
			if (std::max(height1, height2) - position.y - y_radius <= 0 && (onGround1 || onGround2)) {
				onGround = true;
				jumping = false;
			}
		}
	}

	//Check for left ceiling
	h = checkSensor('C', tiles, ang3);
	height3 = abs(h);
	onGround3 = (h >= 0);

	//Check for right ceiling
	h = checkSensor('D', tiles, ang4);
	height4 = abs(h);
	onGround4 = (h >= 0);

	//Push the player out of ceilings
	if (onGround3 || onGround4) {
		switch (collideMode) {
		case GROUND:
			height3 -= position.y - 10;
			height4 -= position.y - 10;
			if (height4 < height3 && onGround3) {
				std::swap(height3, height4);
			}
			ceilingBlocked = (height3 >= -5);
			if (height3 > -3) {
				position.y += height3 + 3;
			}
			break;
		case LEFT_WALL:
			height3 -= position.x + 10;
			height4 -= position.x + 10;
			height3 *= -1;
			height4 *= -1;
			if (height4 < height3 && onGround3) {
				std::swap(height3, height4);
			}
			ceilingBlocked = (height3 >= -5);
			if (height3 > -3) {
				position.x -= height3 + 3;
			}
			break;
		case CEILING:
			height3 -= position.y + 10;
			height4 -= position.y + 10;
			height3 *= -1;
			height4 *= -1;
			if (height4 < height3 && onGround3) {
				std::swap(height3, height4);
			}
			ceilingBlocked = (height3 >= -5);
			if (height3 > -3) {
				position.y -= height3 + 3;
			}
			break;
		case RIGHT_WALL:
			height3 -= position.x - 10;
			height4 -= position.x - 10;
			if (height4 < height3 && onGround3) {
				std::swap(height3, height4);
			}
			ceilingBlocked = (height3 >= -5);
			if (height3 > -3) {
				position.x += height3 + 3;
			}
			break;
		}
	}

	//Check for left wall
	h = checkSensor('E', tiles, sensorDir);
	wallHeightLeft = abs(h);
	wallCollideLeft = (h >= 0);

	sensorDir = 1;

	//Check for right wall
	h = checkSensor('E', tiles, sensorDir);
	wallHeightRight = abs(h);
	wallCollideRight = (h >= 0);

	//Push the player out of walls
	switch (collideMode) {
	case GROUND:
		wallHeightLeft -= position.x;
		wallHeightRight -= position.x;
		if (wallHeightRight < 10 && wallCollideRight) {
			position.x -= 10 - wallHeightRight;
			velocity.x = std::min(velocity.x, 0.0);
			gsp = std::min(gsp, 0.0);
		}
		if (wallHeightLeft > -10 && wallCollideLeft) {
			position.x += 10 + wallHeightLeft;
			velocity.x = std::max(velocity.x, 0.0);
			gsp = std::max(gsp, 0.0);
		}
		break;
	case LEFT_WALL:
		wallHeightLeft -= position.y;
		wallHeightRight -= position.y;
		if (wallHeightRight < 10 && wallCollideRight) {
			position.y -= 10 - wallHeightRight;
			velocity.y = std::min(velocity.y, 0.0);
			gsp = std::min(gsp, 0.0);
		}
		if (wallHeightLeft > -10 && wallCollideLeft) {
			position.y += 10 + wallHeightLeft;
		}
		break;
	case CEILING:
		wallHeightLeft -= position.x;
		wallHeightRight -= position.x;
		if (wallHeightRight > -10 && wallCollideRight) {
			position.x += 10 + wallHeightRight;
			velocity.x = std::max(velocity.x, 0.0);
			gsp = std::min(gsp, 0.0);
		}
		if (wallHeightLeft < 10 && wallCollideLeft) {
			position.x -= 10 - wallCollideLeft;
		}
		break;
	case RIGHT_WALL:
		wallHeightLeft -= position.y;
		wallHeightRight -= position.y;
		if (wallHeightRight > -10 && wallCollideRight) {
			position.y += 10 + wallHeightRight;
			velocity.y = std::max(velocity.y, 0.0);
			gsp = std::min(gsp, 0.0);
		}
		if (wallHeightLeft < 10 && wallCollideLeft) {
			position.y -= 10 - wallCollideLeft;
		}
		break;
	}

	if (wall) {
		SDL_Rect objCollide = (*wall)->getCollisionRect();
		SDL_Point dir = PhysicsEntity::calcRectDirection(objCollide);

		if ((dir.x > -1 * objCollide.w / 2 || dir.x < objCollide.w / 2) && !(dir.y < -1 * objCollide.h || dir.y > objCollide.y)) {
			if (dir.x < 0) {
				velocity.x = std::min(velocity.x, 0.0);
				gsp = std::min(gsp, 0.0);
				position.x += -1 * objCollide.w / 2 - dir.x;
			}
			else {
				velocity.x = std::max(velocity.x, 0.0);
				gsp = std::max(gsp, 0.0);
				position.x += objCollide.w / 2 - dir.x;
			}
		}
		else {
			wall = nullptr;
		}
	}
	
	return output;
}

const std::vector < double > angles = { 0.0, 128.0, 192.0, 64.0 };

int Player::checkSensor(char sensor, std::vector < std::vector < Ground > >& tiles, double& ang) {
	int xEnd, xStart, yEnd, yStart;
	int c = position.x;
	int d = position.y;
	int xRadius = 9 - (rolling << 1);
	int iterOp; //0 = y--, 1 = y++, 2 = x--, 3 = x++
	bool side = false;

	switch (5 * collideMode + sensor - 'A') {
	case 0: //Ground A
	case 13: //Ceiling D
		xStart = xEnd = c - xRadius;
		yStart = d + 36;
		yEnd = d;
		iterOp = 0; // y--
		break;
	case 1: //Ground B
	case 12: //Ceiling C
		xStart = xEnd = c + xRadius;
		yStart = d + 36;
		yEnd = d;
		iterOp = 0; // y--
		break;
	case 2: //Ground C
	case 11: //Ceiling B
		xStart = xEnd = c - xRadius;
		yStart = d - 36;
		yEnd = d;
		iterOp = 1; // y++
		break;
	case 3: //Ground D
	case 10: //Ceiling A
		xStart = xEnd = c + xRadius;
		yStart = d - 36;
		yEnd = d;
		iterOp = 1; // y++
		break;
	case 5: //LW A
	case 18: //RW D
		xStart = c - 36;
		xEnd = c;
		yStart = yEnd = d - xRadius;
		iterOp = 3; // x++
		break;
	case 6: //LW B
	case 17: //RW C
		xStart = c - 36;
		xEnd = c;
		yStart = yEnd = d + xRadius;
		iterOp = 3; // x++
		break;
	case 7: //LW C
	case 16: //RW B
		xStart = c + 36;
		xEnd = c;
		yStart = yEnd = d - xRadius;
		iterOp = 2; // x--
		break;
	case 8: //LW D
	case 15: //RW A
		xStart = c + 36;
		xEnd = c;
		yStart = yEnd = d + xRadius;
		iterOp = 2; // x--
		break;
	case 4: //Ground E
		xStart = c + 16 * ang;
		xEnd = c;
		yStart = yEnd = d + 4;
		iterOp = static_cast< int >(2.5 - ang / 2.0);
		break;
	case 9: //LW E
		xStart = xEnd = c - 4;
		yStart = d + 16 * ang;
		yEnd = d;
		iterOp = static_cast < int >(0.5 - ang / 2.0);
		break;
	case 14: //Ceil E
		xStart = c - 16 * ang;
		xEnd = c;
		yStart = yEnd = d - 4;
		iterOp = static_cast < int >(2.5 + ang / 2.0);
		break;
	case 19: //RW E
		xStart = xEnd = c + 4;
		yStart = d - 16 * ang;
		yEnd = d;
		iterOp = static_cast <int>(0.5 + ang / 2.0);
		break;
	}
	if (xStart < 0 || yStart < 0) {
		if (sensor == 'E')
			return -1 * xStart;
		return -1;
	}
	side = (iterOp - 2) >= 0;
	int blockX = xStart / (TILE_WIDTH * GROUND_WIDTH);
	int blockY = yStart / (TILE_WIDTH * GROUND_WIDTH);
	if (blockX >= tiles.size()) {
		if (sensor == 'E')
			return -1 * xStart;
		return -1 * (yStart + 20);
	}
	else if (blockY >= tiles[0].size()) {
		if (sensor == 'E')
			return -1 * xStart;
		return -1 * (yStart + 20);
	}
	int height = 0;
	int h = 0;
	int count = 0;
	int tileX = (xStart - blockX * TILE_WIDTH * GROUND_WIDTH) / TILE_WIDTH;
	int tileY = (yStart - blockY * TILE_WIDTH * GROUND_WIDTH) / TILE_WIDTH;
	bool check = false;
	bool flip = false;
	bool pathC = tiles[blockX][blockY].getMulti() & path; //Do not check the second path if there is no second path
	if (tiles[blockX][blockY].isEmpty()) {
		//There is no point to checking because we are in an empty tile.
		if (sensor != 'E') {
			height = yStart + 20;
			return -1 * height;
		}
		else {
			height = side ? xStart : yStart;
			return -1 * height;
		}
	}


	while (true) {
		if (!tiles[blockX][blockY].isEmpty()) {
			if (tiles[blockX][blockY].getTile(tileX + GROUND_SIZE * pathC, tileY).getCollide()) {
				//Tile does collide
				h = Player::getHeight(tiles, SDL_Point{ blockX, blockY }, SDL_Point{ tileX, tileY }, side, pathC, xStart, xEnd, yStart, yEnd, flip);
				
				bool rightDir;
				int currentFlags = tiles[blockX][blockY].getFlag(tileX + GROUND_SIZE * pathC + tileY * GROUND_WIDTH);
				switch (iterOp) {
				case 0: //y--
					rightDir = !(currentFlags & SDL_FLIP_VERTICAL);
					break;
				case 1: //y++
					rightDir = (currentFlags & SDL_FLIP_VERTICAL);
					break;
				case 2: //x--
					rightDir = !(currentFlags & SDL_FLIP_HORIZONTAL);
					break;
				case 3: //x++
					rightDir = (currentFlags & SDL_FLIP_HORIZONTAL);
					break;
				}

				if (!rightDir)
					h = 16;

				if (side) {
					if (h < TILE_WIDTH) {
						//If the height is not at the top, then we have our answer, UNLESS we are on the first tile and the height is zero, in which case no collision.
						if (sensor != 'E') {
							if (count == 0 && h == 0) {
								ang = tiles[blockX][blockY].getTileAngle(tileX + GROUND_SIZE * pathC, tileY);
								return -1 * (yStart + 20);
							}
							height = (tileX + 3 - iterOp) * TILE_WIDTH - h + blockX * TILE_WIDTH * GROUND_WIDTH;
							ang = tiles[blockX][blockY].getTileAngle(tileX + GROUND_SIZE * pathC, tileY);
							return height;
						}
						else {
							//Wall sensor is special
							if (count == 0 && h == 0) {
								return -1 * xStart;
							}
							if (iterOp == 3) {
								//Sensor is pointing right, add h to right side of tile
								height = blockX * TILE_WIDTH * GROUND_WIDTH + tileX * TILE_WIDTH + h;
							}
							else {
								//Sensor is pointing left, subtract h from left side of tile
								height = blockX * TILE_WIDTH * GROUND_WIDTH + (tileX + 1) * TILE_WIDTH - h;
							}
							return height;
						}
					}
				}
				else {
					if (h < TILE_WIDTH) {
						//If the height is not at the top, then we have our answer.
						if (sensor != 'E') {
							if (count == 0 && h == 0) {
								ang = tiles[blockX][blockY].getTileAngle(tileX + GROUND_SIZE * pathC, tileY);
								return yStart + 20;
							}
							height = (tileY + 1 - iterOp) * TILE_WIDTH - h + blockY * TILE_WIDTH * GROUND_WIDTH;
							ang = tiles[blockX][blockY].getTileAngle(tileX + GROUND_SIZE * pathC, tileY);
							return height;
						}
						else {
							if (count == 0 && h == 0) {
								return -1 * xStart;
							}
							if (iterOp == 1) {
								height = blockY * TILE_WIDTH * GROUND_WIDTH + tileY * TILE_WIDTH + h;
							}
							else {
								height = blockY * TILE_WIDTH * GROUND_WIDTH + (tileY + 1) * TILE_WIDTH - h;
							}
							return height;
							
						}
						
					}
				}
				check = false;
			} // <-- if(getCollide)
			else if (count == 0) {
				//If our first tile doesn't collide, then we have our answer.
				if (sensor == 'E') {
					return -1 * xStart;
				}
				else {
					ang = 0.0;
					height = yStart + 20;
				}
				return -1 * height;
			}
		}
		//If we have detected a tile with a height of 16 but there is no tile above it, then we are done.
		if (check) {
			if (side) {
				height = (tileX + 3 - iterOp) * TILE_WIDTH + blockX * TILE_WIDTH * GROUND_WIDTH;
				if (sensor != 'E')
					ang = angles[iterOp];
				return height;
			}
			else {
				height = (tileY + 1 - iterOp) * TILE_WIDTH + blockY * TILE_WIDTH * GROUND_WIDTH;
				if (sensor != 'E')
					ang = angles[iterOp];
				return height;
			}
		}

		//Increment tile search
		check = true;
		switch (iterOp) {
		case 0:
			tileY--;
			break;
		case 1:
			tileY++;
			break;
		case 2:
			tileX--;
			break;
		case 3:
			tileX++;
			break;
		}
		count++;
		
		//Endpoint checks
		if (side) {
			if (signum(xEnd - xStart) * ((tileX + 3 - iterOp) * TILE_WIDTH + blockX * TILE_WIDTH * GROUND_WIDTH) > signum(xEnd - xStart) * xEnd) {
				height = xEnd + 20;
				if (sensor != 'E')
					ang = 0;
				return -1 * height;
			}
		}
		else {
			if (signum(yEnd - yStart) * ((tileY + 1 - iterOp) * TILE_WIDTH + blockY * TILE_WIDTH * GROUND_WIDTH) > signum(yEnd - yStart) * yEnd) {
				//If we have gone past our endpoint, stop checking.
				height = yEnd + 20;
				if (sensor != 'E')
					ang = 0;
				return -1 * height;
			}
		}
		if (tileY == -1) {
			tileY = GROUND_WIDTH - 1;
			blockY--;
		}
		if (tileY == GROUND_WIDTH) {
			tileY = 0;
			blockY++;
		}
		if (tileX == -1) {
			tileX = GROUND_WIDTH - 1;
			blockX--;
		}
		if (tileX == GROUND_WIDTH) {
			tileX = 0;
			blockX++;
		}

		pathC = tiles[blockX][blockY].getMulti() & path;
	}
}

int Player::getHeight(std::vector < std::vector < Ground > >& ground, SDL_Point block, SDL_Point tile, bool side, bool pathC, int xStart, int xEnd, int yStart, int yEnd, bool& flip) {
	int h;
	if (side) {
		if (ground[block.x][block.y].getFlag(tile.x + tile.y * GROUND_WIDTH + GROUND_SIZE * pathC) & SDL_FLIP_VERTICAL) {
			//Tile is flipped, so set the current height to the flipped heightMap
			h = ground[block.x][block.y].getTile(tile.x + GROUND_SIZE * pathC, tile.y).getHeight(TILE_WIDTH - 1 - ((yStart - ground[block.x][block.y].getPosition().y) % TILE_WIDTH), side);
			flip = true;
		}
		else {
			//Tile is not flipped, set the current height normally.
			h = ground[block.x][block.y].getTile(tile.x + GROUND_SIZE * pathC, tile.y).getHeight((yStart - ground[block.x][block.y].getPosition().y) % TILE_WIDTH, side);
			flip = false;
		}
		if (ground[block.x][block.y].getFlag(tile.x + tile.y * GROUND_WIDTH + GROUND_SIZE * pathC) & SDL_FLIP_HORIZONTAL) {
			h = TILE_WIDTH * ((h + TILE_WIDTH - 1) / TILE_WIDTH) - (h % TILE_WIDTH);
		}
	}
	else {
		if (ground[block.x][block.y].getFlag(tile.x + GROUND_SIZE * pathC + tile.y * GROUND_WIDTH) & SDL_FLIP_HORIZONTAL) {
			//Tile is flipped, so set the current height to the flipped heightMap
			h = ground[block.x][block.y].getTile(tile.x + GROUND_SIZE * pathC, tile.y).getHeight(TILE_WIDTH - 1 - ((xStart - ground[block.x][block.y].getPosition().x) % TILE_WIDTH), side);
			flip = true;
		}
		else {
			//Tile is not flipped, set the current height normally.
			h = ground[block.x][block.y].getTile(tile.x + GROUND_SIZE * pathC, tile.y).getHeight((xStart - ground[block.x][block.y].getPosition().x) % TILE_WIDTH, side);
			flip = false;
		}
		if (ground[block.x][block.y].getFlag(tile.x + tile.y * GROUND_WIDTH + GROUND_SIZE * pathC) & SDL_FLIP_VERTICAL) {
			h = TILE_WIDTH * ((h + TILE_WIDTH - 1) / TILE_WIDTH) - (h % TILE_WIDTH);
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

void Player::Render(SDL_Rect& cam, double screenRatio) {
	if (invis)
		return;
	typedef Animation::effectData effectData;
	typedef Animation::effectType effectType;
	angle = (angle == 255.0 || angle == 1.0) ? 0.0 : angle;
	SDL_Rect pos = GetRelativePos(cam);
	pos.x += centerOffset.x;
	pos.y += centerOffset.y;
	//For only 45-degree rotations 
	double frames((time - last_time) / (1000.0 / 60.0));
	displayAngle = angle;
	displayAngle += 256;
	displayAngle %= 256;
	int rot = rolling ? 0 : 45 * (displayAngle / 32.0); //For all rotations
	//const int numRotations = 10;
	//int rot = (360 / numRotations) * ((displayAngle + 128 / numRotations) / (256 / numRotations));
	SDL_Point center{ -1 * centerOffset.x, -1 * centerOffset.y };
	SDL_RendererFlip flip = static_cast<SDL_RendererFlip>(SDL_FLIP_HORIZONTAL & horizFlip);
	int temp = currentAnim;

	effectData efxData;
	effectType efxType;

	do {
		if (temp % animations.size() == 5) {
			SDL_Rect tailPos = pos;
			SDL_Point tailCenter;
			SDL_RendererFlip tailFlip;
			int tailRot = -90;
			if (temp == 5 + 4 * animations.size()) {
				//Tails have width of 14, radius of 7
				int offset = 2;
				tailCenter = { 7, centerOffset.y + offset };
				tailPos.x -= centerOffset.x + 7;
				tailPos.y -= 2 * centerOffset.y + offset;
				if (velocity.x != 0.0 || velocity.y != 0.0) {
					tailRot += atan2(-velocity.y, -velocity.x) * 180.0 / M_PI;
				}
				tailFlip = static_cast < SDL_RendererFlip > (SDL_FLIP_HORIZONTAL & (velocity.x > 0.0));
				efxType = effectType::ROTATION;
				efxData.rot.degrees = tailRot;
				*efxData.rot.center = tailCenter;
			}
			else {
				tailPos.x -= centerOffset.x + 5 + (horizFlip * 39);
				tailPos.y += 24;
				tailCenter = { 7, -10 };
				tailRot = 90 * (horizFlip * -2 + 1);
				tailFlip = static_cast < SDL_RendererFlip >(SDL_FLIP_HORIZONTAL & !horizFlip);
				efxType = effectType::NONE;
			}
			animations[temp % animations.size()]->Render(&tailPos, tailRot, &tailCenter, 1.0 / screenRatio, tailFlip, efxType, &efxData);
		}
		else {
			efxType = effectType::ROTATION;
			efxData.rot.degrees = rot;
			*efxData.rot.center = center;
			animations[temp % animations.size()]->Render(&pos, rot, &center, 1.0 / screenRatio, flip, efxType, &efxData);
		}
		temp /= animations.size();
	} while (temp != 0);
}

SDL_Rect Player::getCollisionRect() {
	switch (collideMode) {
	case GROUND:
		return SDL_Rect{ position.x - (collisionRect.w / 2), position.y + centerOffset.y, collisionRect.w, collisionRect.h };
	case LEFT_WALL:
		return SDL_Rect{ position.x + centerOffset.y - collisionRect.h, position.y - (collisionRect.w / 2), collisionRect.h, collisionRect.w };
	case CEILING:
		return SDL_Rect{ position.x - (collisionRect.w / 2), position.y - centerOffset.y - collisionRect.h, collisionRect.w, collisionRect.h };
	case RIGHT_WALL:
		return SDL_Rect{ position.x + centerOffset.x, position.y + centerOffset.y, collisionRect.h, collisionRect.w };
	default:
		return SDL_Rect{ -1, -1, -1, -1 };
	}
}

void Player::hitPlatform(const Player::entityPtrType entity) {
	platform = entity;
}

void Player::hitWall(const Player::entityPtrType entity) {
	wall = entity;
}

void Player::setActCleared(bool b)
{
	actCleared = b;
	if (!b) {
		animations[13]->SetDelay(150);
		animations[13]->setLooped(false);
	}
}

bool Player::canDamage() {
	return (!corkscrew && jumping) || (rolling);
}