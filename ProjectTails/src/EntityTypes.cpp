#include <array>
#include <algorithm>
#include <stdexcept>
#include <cassert>
#include "EntityTypes.h"
#include "PhysicsEntity.h"
#include "Player.h"
#include "Functions.h"

Ring::Ring(const FlagList& list) :
	Ring() {
	assert(list.size() == requiredFlags);
}

Ring::Ring() :
	timeUntilDespawn(256.0 * 1000.0 / 60.0), 
	pickedUp(false) {

}

void Ring::update(PhysicsEntity& parent, EntityManager& manager, const Player& player) {
	if (pickedUp) {
		parent.destroy();
	}
	else if (parent.velocity.x != 0.0 || parent.velocity.y != 0.0) {
		bool willDespawn = timeUntilDespawn.update();
		int newTime = 233.0 * (timeUntilDespawn.timeRemaining().count() / 4267.0) + 33.0;
		parent.getAnim(0)->setDelay(Timer::DurationType{ newTime });
		if (willDespawn) {
			parent.destroy();
		}
	}
}

Spring::Spring(const FlagList& list) :
	Spring() {
	if (list.front() == 'u') {
		direction = Direction::UP;
	}
	else if (list.front() == 'd') {
		direction = Direction::DOWN;
	}
	else if (list.front() == 'l') {
		direction = Direction::LEFT;
	}
	else if (list.front() == 'r') {
		direction = Direction::RIGHT;
	}
	assert(list.size() == requiredFlags);
}

Spring::Spring() :
	extendedTime(100.0 * 1000.0 / 60.0) {
	extendedTime.stop();
}

void Spring::update(PhysicsEntity& parent, EntityManager& manager, const Player& player) {
	bool stopExtending = extendedTime.update();
	if (extendedTime.isTiming()) {
		parent.setAnim(1);
	}
	else {
		parent.setAnim(0);
	}
	if (stopExtending) {
		extendedTime.stop();
	}
}

void Spring::bounceEntity(const PhysicsEntity& parent, PhysicsEntity& entity) {
	extendedTime.start();
	doublePoint vel = entity.getVelocity();
	if (static_cast<int>(direction) & static_cast<int>(Direction::LEFT)) {
		vel.x = -10.0;
	}
	else if (static_cast<int>(direction) & static_cast<int>(Direction::RIGHT)) {
		vel.x = 10.0;
	}
	if (static_cast<int>(direction) & static_cast<int>(Direction::UP)) {
		vel.y = -10.0;
	}
	else if (static_cast<int>(direction) & static_cast<int>(Direction::DOWN)) {
		vel.y = 10.0;
	}
	entity.setVelocity(vel);
}

BeeBadnik::BeeBadnik(const FlagList& list) :
	BeeBadnik() {
	assert(list.size() == requiredFlags);
}

BeeBadnik::BeeBadnik() :
	timeUntilMove(120.0 * 1000.0 / 60.0),
	hasFired(false) {

}

void BeeBadnik::update(PhysicsEntity& parent, EntityManager& manager, const Player& player) {
	bool startMoving = timeUntilMove.update();
	if (startMoving) {
		timeUntilMove.stop();
	}
	if (parent.currentAnimIndex() == 0 || !timeUntilMove.isTiming()) {
		const SDL_Rect playerPosition = player.getPosition();
		parent.velocity.x = -2.0;
		parent.setAnim(0);
		if (playerPosition.x != parent.getPosition().x) {
			const double slope = std::abs(double(parent.getPosition().y - playerPosition.y) / (playerPosition.x - parent.getPosition().x));
			if (slope >= 0.9 && slope <= 1.1) {
				parent.setAnim(1);
				timeUntilMove.start();
				parent.velocity.x = 0.0;
				hasFired = false;
			}
		}
	}
	else {
		if (timeUntilMove.timeRemaining().count() < 60.0 && !hasFired) {
			hasFired = true;

			SDL_Rect newPos{ parent.getPosition() };
			newPos.x += 30;
			newPos.y += 25;

			PhysStruct temp{ "BEEPROJECTILE", {}, newPos, true };
			auto entity = std::make_unique< PhysicsEntity >(temp);
			entity->setVelocity({-2.0, 2.0});
			manager.AddEntity(std::move(entity));
		}
		parent.velocity.x = 0;
	}
}

CrabBadnik::CrabBadnik(const FlagList& list) :
	CrabBadnik() {
	assert(list.size() == requiredFlags);
}

CrabBadnik::CrabBadnik() :
	timeUntilWalk(240.0 * 1000.0 / 60.0),
	timeUntilStop(30.0 * 1000.0 / 60.0),
	currentDirection(1) {
	timeUntilWalk.stop();
}

void CrabBadnik::update(PhysicsEntity& parent, EntityManager& manager, const Player& player) {
	bool doneWalking = timeUntilStop.update();
	bool doneStanding = timeUntilWalk.update();
	if (doneWalking) {
		timeUntilStop.stop();
		timeUntilWalk.start();
		parent.velocity.x = 0;
		parent.setAnim(0);
		if (doneStanding) {
			currentDirection *= -1;
			timeUntilWalk.stop();
			timeUntilStop.start();
		}
	}
	else {
		timeUntilWalk.stop();
		timeUntilStop.start();
		parent.setAnim(1);
		parent.velocity.x = currentDirection;
	}
}

Bridge::Bridge(const FlagList& list) :
	bridgeWidth(list.front()),
	playerPosition(0),
	transition(0.0),
	segmentOffsets(bridgeWidth, 0.0) {
	assert(list.size() == requiredFlags);
}

Bridge::Bridge() :
	bridgeWidth(16),
	playerPosition(0),
	transition(0.0) {

}

void Bridge::update(PhysicsEntity& parent, EntityManager& manager, const Player& player) {
	bool playerIsOnPlatform = player.getPosition().y - parent.getPosition().y <= 21;
	parent.setCollisionRect({ 0, 0, int(bridgeWidth * 16), 16 });
	if (playerIsOnPlatform) {
		//Smooth on/off timer
		transition = std::min(1.0, transition + (Timer::getFrameTime().count() / (1000.0 / 60.0)) / 30);

		//Player index
		playerPosition = (player.getPosition().x - parent.getPosition().x) / 16.0;
		SDL_Rect newCollisionRect;
		newCollisionRect.x = std::min(std::max(playerPosition * 16, 0), int(bridgeWidth) - 1 * 16);
		newCollisionRect.y = 0;
		newCollisionRect.w = 16;
		newCollisionRect.h = 16;
		parent.setCollisionRect(newCollisionRect);

		auto getMaxDepression = [size = static_cast<std::size_t>(bridgeWidth)](std::size_t index) {
			if (index < size / 2) {
				return 2 * (index + 1);
			}
			else if (index > size / 2) {
				return 2 * (size - index);
			}
			else {
				return size;
			}
		};
		std::size_t playerIndex = std::min<std::size_t>(playerPosition, bridgeWidth - 1.0);
		int thisMax = getMaxDepression(playerIndex);

		for (int i = 0; i < std::min<std::size_t>(playerPosition + 1, bridgeWidth - 1); ++i) {
			segmentOffsets[i] = transition * thisMax * sin(M_PI / 2 * (1 + i) / (1 + playerPosition));
		}
		for (int i = playerPosition + 1; i < bridgeWidth; ++i) {
			segmentOffsets[i] = transition * thisMax * sin(M_PI / 2 * (bridgeWidth - i) / (bridgeWidth - playerPosition));
		}
		if (playerPosition < segmentOffsets.size()) {
			SDL_Rect oldCollision = parent.getCollisionRect();
			oldCollision.y = segmentOffsets[playerPosition];
			parent.setCollisionRect(oldCollision);
		}
	}
	else {
		transition = std::max(0.0, transition - (Timer::getFrameTime().count() / (1000.0 / 60.0)) / 120);
		playerPosition = -1;
		for (auto& offset : segmentOffsets) {
			offset *= sqrt(transition);
		}
	}
}

void Bridge::render(const PhysicsEntity& parent, const SDL_Rect& cameraPosition) const {
	SDL_Rect relativePos = PhysicsEntity::getRelativePos(parent.getPosition(), cameraPosition);
	int xStart = relativePos.x;
	int yStart = relativePos.y;
	for (std::size_t segment = 0; segment < bridgeWidth; ++segment) {
		relativePos.x = xStart + 16 * segment;
		relativePos.y = yStart + segmentOffsets[segment];
		parent.getAnim()->Render(getXY(relativePos), 0, NULL, 1.0 / globalObjects::ratio);
	}
}

Goalpost::Goalpost(const FlagList& list) :
	Goalpost() {
	assert(list.size() == requiredFlags);
}

Goalpost::Goalpost() :
	timeUntilStop(240.0 * 1000.0 / 60.0) {
	
}

void Goalpost::update(PhysicsEntity& parent, EntityManager& manager, const Player& player) {
	using namespace std::chrono_literals;
	if (timeUntilStop.update()) {
		parent.getAnim(0)->setDelay(-1ms);
		if (finishedSpinning && parent.getAnim(0)->getFrame() != 4) {
			parent.getAnim(0)->setDelay(240ms);
		}
	}
	else {
		int newTime = 240.0 - timeUntilStop.timeRemaining().count() / 1.5;
		parent.getAnim(0)->setDelay(std::chrono::milliseconds{ newTime });
		finishedSpinning = true;
	}
}

Pathswitch::Pathswitch(const FlagList& list) :
	debounce(0) {
	assert(list.size() == requiredFlags);
	switch(list.front()) {
	case 'u':
		mode = Mode::UNSET;
		break;
	case 's':
		mode = Mode::SET;
		break;
	case 'i':
		mode = Mode::INVERT;
		break;
	default:
		throw std::invalid_argument("Invalid pathswitch flag: " + std::string(list.front(), 1));
	}
}

Pathswitch::Pathswitch() :
	debounce(0),
	mode( Mode::UNSET ) {

}

void Pathswitch::update(PhysicsEntity& parent, EntityManager& manager, const Player& player) {
	if (debounce != 0) {
		--debounce;
	}
}

void Pathswitch::setPath(bool& i) {
	if (debounce == 0) {
		switch (mode) {
		case Mode::UNSET:
			i = 0;
			break;
		case Mode::SET:
			i = 1;
			break;
		case Mode::INVERT:
			i = !i;
			break;
		}
	}
	triggerDebounce();
}

void Pathswitch::triggerDebounce() {
	debounce = 2;
}

NoCustomData::NoCustomData(const FlagList& list) {
	assert(list.size() == requiredFlags);
}

bool entity_property_data::isEnemy(const entity_property_data::EntityTypeId& key) {
	static const std::array < const char*, 2 > enemyList{ "BEEBADNIK", "CRABBADNIK" };

	return std::find(enemyList.begin(), enemyList.end(), key) != enemyList.end();
}

std::array < std::string, 8 > entity_property_data::keyStrings{
	"RING", "SPRING", "BEEBADNIK", "CRABBADNIK", "BRIDGE", "PATHSWITCH", "GOALPOST", "NOCUSTOM"
};

static_assert(entity_property_data::keyStrings.size() == static_cast<std::size_t>(entity_property_data::Keys::NO_CUSTOM) + 1, "");

const std::string& keyToString(const entity_property_data::Keys& key) {
	return entity_property_data::keyStrings[static_cast<int>(key)];
}

entity_property_data::Keys entity_property_data::stringToKey(const std::string& keyString) {
	const auto key = std::find(keyStrings.begin(), keyStrings.end(), keyString);
	if (key == keyStrings.end()) {
		return Keys::NO_CUSTOM;
	}
	else {
		return static_cast<Keys>(std::distance(keyStrings.begin(), key));
	}
}

std::unordered_map < std::string, entity_property_data::EntityType > entity_property_data::entityTypes;

const entity_property_data::EntityType& entity_property_data::getEntityTypeData(const entity_property_data::EntityTypeId& id) {
	const auto typeData = entity_property_data::entityTypes.find(id);
	if (typeData == entity_property_data::entityTypes.end()) {
		throw std::logic_error("Invalid entity type data ID: " + id);
	}
	else {
		return typeData->second;
	}
}

bool entity_property_data::isHazard(const entity_property_data::EntityTypeId& id) {
	static const std::array < EntityTypeId, 3 > hazardList { "SPIKES", "BEEPROJECTILE" };

	return isEnemy(id) || std::find(hazardList.begin(), hazardList.end(), id) != hazardList.end();
}

entity_property_data::CustomData entity_property_data::createCustomFromKey(entity_property_data::Keys key, const FlagList& flags) {
	using namespace entity_property_data;
	const std::size_t keyValue = static_cast< std::size_t > (key);
	return helpers::getOne<CustomData>([&](auto& custom) { return CustomData{ std::decay_t<decltype(custom)>{ flags } }; }, keyValue);
}

std::size_t entity_property_data::requiredFlagCount(entity_property_data::Keys behaviorKey) {
	const std::size_t keyValue = static_cast< std::size_t > (behaviorKey);
	return helpers::getOne<std::size_t>([](auto& custom) { return custom.requiredFlags; }, keyValue);

}

std::size_t entity_property_data::requiredFlagCount(const std::string& behaviorKey) {
	using namespace entity_property_data;
	return requiredFlagCount(stringToKey(behaviorKey));
}
