#include "PlayerState.h"
#include "Player.h"
#include <algorithm>

using globalObjects::input;

Idle::Idle(Player& player) {
	player.setAnimation(1);
}

Idle::~Idle() {

}

static void updateNormalStates(Player& player, double accel, double decel, double frc, double& gsp) {
	if (std::abs(player.getGsp()) < 2.0 && player.getMode() != GROUND && !player.isLocked()) {
		if (64 <= player.getAngle() && player.getAngle() <= 192) {
			// TODO: Fall
			/*collideMode = GROUND;
			angle = 0.0;
			onGround = false;*/
		}
		player.startControlLock();
		return;
	}

	const double frameCount = Timer::getFrameTime().count() / (1000.0 / 60.0);

	const double thisAccel = frameCount * accel;
	const double thisDecel = frameCount * decel;
	const double thisFrc   = frameCount * frc;

	const int direction = input.GetKeyState(InputComponent::RIGHT) - input.GetKeyState(InputComponent::LEFT);
	if (direction == 0) {
		if (std::abs(gsp) < thisFrc) {
			gsp = 0.0;
		}
		else {
			gsp -= thisFrc * signum(gsp);
		}
		return;
	}

	// Same operations are performed when going left, but flipped
	if (direction < 0) {
		gsp *= -1.0;
	}

	if (gsp >= 0.0) {
		gsp += thisAccel;
	}
	else if (std::abs(gsp + thisDecel) != gsp + thisDecel) {
		gsp = 0.0;
	}
	else {
		gsp += thisDecel;
	}

	if (direction < 0) {
		gsp *= -1.0;
	}
}

std::unique_ptr< PlayerState > Idle::update(Player& player, std::vector< std::vector< Ground > >& tiles, EntityManager& manager) {
	const int dir = input.GetKeyState(InputComponent::RIGHT) - input.GetKeyState(InputComponent::LEFT);

	using namespace player_constants::physics;
	double gsp = 0.0;
	updateNormalStates(player, DEFAULT_ACCELERATION, DEFAULT_DECCELERATION, DEFAULT_FRICTION, gsp);

	if (gsp != 0.0) {
		return std::make_unique< Walking >(player, gsp);
	}

	if (input.GetKeyState(InputComponent::DOWN)) {
		return std::make_unique< Crouching >(player);
	}

	if (input.GetKeyState(InputComponent::UP)) {
		return std::make_unique< LookingUp >(player);
	}

	return {};
}

Walking::Walking(Player& player, double gsp_) : gsp(gsp_) {
	player.setAnimation(2);
}

Walking::~Walking() {

}

std::unique_ptr< PlayerState > Walking::update(Player& player, std::vector< std::vector< Ground > >& tiles, EntityManager& manager) {
	using namespace player_constants::physics;
	updateNormalStates(player, DEFAULT_ACCELERATION, DEFAULT_DECCELERATION, DEFAULT_FRICTION, gsp);

	if (globalObjects::input.GetKeyState(InputComponent::DOWN)) {
		if (std::abs(gsp) < 0.5) {
			return std::make_unique< Crouching >(player);
		}
		else {
			return std::make_unique< Rolling >(player, gsp);
		}
	}
	if (gsp == 0.0) {
		return std::make_unique< Idle >(player);
	}

	player.setAnimation((gsp >= 0.9 * player_constants::physics::DEFAULT_TOP_SPEED) ? 3 : 2);
	player.getAnim(player.currentAnim.front())->setDelay(Animation::DurationType(static_cast<int>(std::max(8.0 - std::abs(gsp), 1.0) * 1000.0 / 60.0)));

	if (gsp < 0.0) {
		player.setFlip(true);
	}
	else if (gsp > 0.0) {
		player.setFlip(false);
	}

	return {};
}

Jumping::Jumping(Player& player, double jmp_, bool corkscrew_) : jmp(jmp_), corkscrew(corkscrew_) {
	using namespace std::chrono_literals;
	player.setAnimation(5 + 4 * player.numAnims());
	player.getAnim(5)->setDelay(128ms);
}

std::unique_ptr< PlayerState > Jumping::update(Player& player, std::vector< std::vector< Ground > >& tiles, EntityManager& manager) {
	if (input.GetKeyPress(InputComponent::JUMP) && !corkscrew) {
		return std::make_unique< Flying >(player, 8000);
	}

	if (!input.GetKeyState(InputComponent::JUMP) && player.getVelocity().y < -4.0 && !corkscrew) {
		player.setVelocity({ player.getVelocity().x, -4.0 });
	}

	// TODO: Controls

	return {};
}

Flying::Flying(Player& player_, int flightTime_) : player(player_), flightTime(flightTime_) {
	player.setGravity(player_constants::physics::FLIGHT_GRAVITY);
	player.setAnimation(9);
}

std::unique_ptr< PlayerState > Flying::update(Player& player, std::vector< std::vector< Ground > >& tiles, EntityManager& manager) {
	const double frameCount = Timer::getFrameTime().count() / (1000.0 / 60.0);
	const double thisFrc = pow(player_constants::physics::AIR_FRICTION, frameCount); 

	if (player.getVelocity().y < -1) {
		player.setGravity(player_constants::physics::FLIGHT_GRAVITY);
	}

	if (flightTime.update()) {
		player.setAnimation(8);
		flightTime.stop();
	}

	if (flightTime.isTiming() && input.GetKeyPress(InputComponent::JUMP) && player.getVelocity().y >= -1.0) {
		player.setGravity(-0.125);
	}

	if (player.getVelocity().x < 0) {
		player.setFlip(true);
	}
	else if (player.getVelocity().x > 0) {
		player.setFlip(false);
	}

	return {};
}

Flying::~Flying() {
	player.setGravity(player_constants::physics::DEFAULT_GRAVITY);
}

Crouching::Crouching(Player& player) {
	player.setAnimation(6);
}

std::unique_ptr< PlayerState > Crouching::update(Player& player, std::vector< std::vector< Ground > >& tiles, EntityManager& manager) {
	if (!input.GetKeyState(InputComponent::DOWN)) {
		return std::make_unique< Idle >(player);
	}

	if (input.GetKeyPress(InputComponent::JUMP)) {
		return std::make_unique< Spindash >(player, 2.0);
	}

	return {};
}

LookingUp::LookingUp(Player& player) {
	player.setAnimation(10);
}

std::unique_ptr< PlayerState > LookingUp::update(Player& player, std::vector< std::vector< Ground > >& tiles, EntityManager& manager) {
	if (!input.GetKeyState(InputComponent::UP)) {
		return std::make_unique< Idle >(player);
	}

	return {};
}

Spindash::Spindash(Player& player, double speed_) : speed(speed_) {
	using namespace std::chrono_literals;
	player.setAnimation(5 + 7 * player.numAnims());
	player.getAnim(5)->setDelay(60ms);
}

std::unique_ptr< PlayerState > Spindash::update(Player& player, std::vector< std::vector< Ground > >& tiles, EntityManager& manager) {
	// If no longer pressing down, start rolling
	if (!input.GetKeyState(InputComponent::DOWN)) {
		double gsp = 8.0 + floor(speed / 2.0);
		gsp = (player.getFlip() ? -gsp : gsp);
		return std::make_unique< Rolling >(player, gsp);
	}
	// If jump is pressed, add speed
	else if (input.GetKeyPress(InputComponent::JUMP)) {
		speed += 2.0;
	}

	// Make sure spindash decays
	speed -= (floor(speed * 8.0) / 256.0) * (Timer::getFrameTime().count() / (1000.0 / 60.0));

	return {};
}

Rolling::Rolling(Player& player, double gsp_) : gsp(gsp_) {
	using namespace std::chrono_literals;
	player.setAnimation(5 + 4 * player.numAnims());
	player.getAnim(5)->setDelay(128ms);
}

std::unique_ptr< PlayerState > Rolling::update(Player& player, std::vector< std::vector< Ground > >& tiles, EntityManager& manager) {
	const double frameCount = (Timer::getFrameTime().count() / (1000.0 / 60.0));
	const double thisDecel = player_constants::physics::ROLLING_DECCELERATION * frameCount;
	const double thisFrc   = player_constants::physics::ROLLING_FRICTION * frameCount;

	//TODO: Controls

	double slp;

	//TODO: Slope

	// Calculate slope value
	if (signum(gsp) == signum(sin(hexToRad(player.getAngle())))) {
		// Uphill
		slp = 0.078125;
	}
	else {
		// Downhill
		slp = 0.3125;
	}

	gsp -= sin(hexToRad(player.getAngle())) * slp * frameCount;

	if (player.getOnGround()) {
		if (std::abs(gsp) < thisFrc) {
			return std::make_unique< Idle >(player);
		}
		else {
			gsp -= thisFrc * signum(gsp);
		}

		if (std::abs(gsp) < 0.5) {
			return std::make_unique< Walking >(player, gsp);
		}
	}

	// TODO: Rolljumps
}
