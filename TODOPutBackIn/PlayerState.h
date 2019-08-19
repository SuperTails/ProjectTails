#pragma once
#include "InputComponent.h"
#include "Ground.h"
#include "PhysicsEntity.h"
#include <unordered_set>
#include <memory>
#include <variant>
#include <vector>

class Player;

struct Event {
	struct Input {
		InputComponent::Key key;
	};
	struct Level {
		
	};
	std::variant< Input, Level > data;
};

class PlayerState {
public:
	virtual ~PlayerState() = 0;

	virtual std::unique_ptr< PlayerState > update(Player& player, std::vector< std::vector< Ground > >& tiles, EntityManager& manager) = 0;

protected:
	PlayerState() {}; 
};

class Idle : public PlayerState {
public:
	Idle(Player& player);
	~Idle();

	std::unique_ptr< PlayerState > update(Player& player, std::vector< std::vector< Ground > >& tiles, EntityManager& manager) override;
};

class Walking : public PlayerState {
public:
	Walking(Player& player, double gsp_);
	~Walking();

	std::unique_ptr< PlayerState > update(Player& player, std::vector< std::vector< Ground > >& tiles, EntityManager& manager) override;

private:
	double gsp;
};

class Jumping : public PlayerState {
public:
	Jumping(Player& player, double jmp_, bool corkscrew_);
	~Jumping();

	std::unique_ptr< PlayerState > update(Player& player, std::vector< std::vector< Ground > >& tiles, EntityManager& manager) override;

private:
	double jmp;
	bool corkscrew;
};

class Flying : public PlayerState {
public:
	Flying(Player& player_, int flightTime_);
	~Flying();

	std::unique_ptr< PlayerState > update(Player& player, std::vector< std::vector< Ground > >& tiles, EntityManager& manager) override;

private:
	Player& player;
	Timer flightTime;
};

class Crouching : public PlayerState {
public:
	Crouching(Player& player);
	~Crouching();

	std::unique_ptr< PlayerState > update(Player& player, std::vector< std::vector< Ground > >& tiles, EntityManager& manager) override;

private:
};

class LookingUp : public PlayerState {
public:
	LookingUp(Player& player);
	~LookingUp();

	std::unique_ptr< PlayerState > update(Player& player, std::vector< std::vector< Ground > >& tiles, EntityManager& manager) override;

private:
};

class Spindash : public PlayerState {
public:
	Spindash(Player& player, double speed_);
	~Spindash();

	std::unique_ptr< PlayerState > update(Player& player, std::vector< std::vector< Ground > >& tiles, EntityManager& manager) override;

private:
	double speed;
};

class Rolling : public PlayerState {
public:
	Rolling(Player& player, double gsp_);
	~Rolling();

	std::unique_ptr< PlayerState > update(Player& player, std::vector< std::vector< Ground > >& tiles, EntityManager& manager) override;

private:
	double gsp;
};

class RollJumping : public PlayerState {
public:
	RollJumping(Player& player, double xVel_, double yVel_);
	~RollJumping();

	std::unique_ptr< PlayerState > update(Player& player, std::vector< std::vector< Ground > >& tiles, EntityManager& manager) override;

private:
	double xVel;
	double yVel;
};
