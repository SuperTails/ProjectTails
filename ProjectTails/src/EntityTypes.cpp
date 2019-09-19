#include <array>
#include <algorithm>
#include <stdexcept>
#include <cassert>
#include <sstream>
#include "Camera.h"
#include "EntityTypes.h"
#include "PhysicsEntity.h"
#include "Player.h"
#include "Functions.h"

Ring::Ring(const FlagList& list) :
	Ring() {
	assert(list.size() == requiredFlags);
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

void Ring::onPlayerTouch(PhysicsEntity& parent, EntityManager& manager, Player& player) {
	if (player.addRing()) {
		pickedUp = true;
	}
}

Spring::Spring(const FlagList& list) :
	Spring() {
	assert(list.size() == requiredFlags);
#define DIRECTION(flag, value) case flag: direction = Direction::value; break
	switch (list.front()) {
	DIRECTION('u', UP);
	DIRECTION('d', DOWN);
	DIRECTION('l', LEFT);
	DIRECTION('r', RIGHT);
	default:
		throw std::invalid_argument(std::string{ "Invalid direction for spring " } + list.front());
	}
}

Spring::Spring() :
	extendedTime(100.0 * 1000.0 / 60.0) {
	extendedTime.stop();
}

void Spring::update(PhysicsEntity& parent, EntityManager& manager, const Player& player) {
	bool stopExtending = extendedTime.update();
	if (extendedTime.isTiming()) {
		parent.setAnim({ 1 });
	}
	else {
		parent.setAnim({ 0 });
	}
	if (stopExtending) {
		extendedTime.stop();
	}
}

void Spring::onPlayerTouch(PhysicsEntity& parent, EntityManager& manager, Player& player) {
	if (intersects(parent, player)) {
		player.setOnGround(false);
		player.setCorkscrew(true);
		bounceEntity(parent, static_cast< PhysicsEntity& >(player));
	}
}

void Spring::bounceEntity(const PhysicsEntity& parent, PhysicsEntity& entity) {
	extendedTime.start();
	Vector2 vel = entity.getVelocity();
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
	timeUntilMove(60.0 * 1000.0 / 60.0),
	hasFired(false) {

}

void BeeBadnik::update(PhysicsEntity& parent, EntityManager& manager, const Player& player) {
	if (timeUntilMove.update()) {
		// Start moving
		timeUntilMove.stop();
		parent.setAnim({ 0 });
	}

	if (!timeUntilMove.isTiming()) {
		// Flying animation
		const Point playerPosition = player.getPosition();
		parent.velocity.x = -2.0;
		parent.setAnim({ 0 });
		if (playerPosition.x != parent.getPosition().x) {
			const double slope = std::abs(double(parent.getPosition().y - playerPosition.y) / (playerPosition.x - parent.getPosition().x));
			if (0.9 <= slope && slope <= 1.1) {
				// Player found
				parent.setAnim({ 1 });
				timeUntilMove.start();
				parent.velocity.x = 0.0;
				hasFired = false;
			}
		}
	}
	else {
		if (timeUntilMove.timeRemaining().count() < 30.0 && !hasFired) {
			hasFired = true;

			Point newPos = parent.getPosition();
			newPos.x += 30.0;
			newPos.y += 25.0;

			auto entity = std::make_unique< PhysicsEntity >("BEEPROJECTILE", std::vector< char >{}, newPos);
			entity->setVelocity({ -2.0, 2.0 });
			manager.AddEntity(std::move(entity));
		}
		parent.velocity.x = 0;
	}
}

void BeeBadnik::onPlayerTouch(PhysicsEntity& parent, EntityManager& manager, Player& player) {
	player.hitEnemy(manager, parent);
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
		parent.setAnim({ 0 });
		if (doneStanding) {
			currentDirection *= -1;
			timeUntilWalk.stop();
			timeUntilStop.start();
		}
	}
	else {
		timeUntilWalk.stop();
		timeUntilStop.start();
		parent.setAnim({ 1 });
		parent.velocity.x = currentDirection;
	}
}

void CrabBadnik::onPlayerTouch(PhysicsEntity &parent, EntityManager &manager, Player &player) {
	player.hitEnemy(manager, parent);
}

Bridge::Bridge(const FlagList& list) :
	bridgeWidth(list.front()),
	transition(0.0),
	segmentOffsets(bridgeWidth, 0.0) {
	assert(list.size() == requiredFlags);
}

Bridge::Bridge() :
	bridgeWidth(16),
	transition(0.0) {

}

void Bridge::onPlayerTouch(PhysicsEntity &parent, EntityManager &manager, Player &player) {
	
}

void Bridge::update(PhysicsEntity& parent, EntityManager& manager, const Player& player) {
	bool playerIsOnPlatform = player.getPosition().y - parent.getPosition().y <= 21;
	parent.setHitbox(HitboxForm(Rect{ 0.0, 0.0, bridgeWidth * 16.0, 16.0 }));
	if (playerIsOnPlatform) {
		//Smooth on/off timer
		transition = std::min(1.0, transition + (Timer::getFrameTime().count() / (1000.0 / 60.0)) / 30);

		//Player index
		int playerPosition = (player.getPosition().x - parent.getPosition().x) / 16.0;

		parent.setHitbox(HitboxForm{Rect{ std::min(std::max(playerPosition * 16.0, 0.0), (bridgeWidth - 1.0) * 16.0), 0, 16, 16 }});

		const auto playerIndex = std::min< std::size_t>(playerPosition, bridgeWidth - 1.0);

		int maxDepression = [&,this]() {
			const auto size = static_cast< std::size_t >(bridgeWidth);
			if (playerIndex < size / 2) {
				return 2 * (playerIndex + 1);
			}
			else if (playerIndex > size / 2) {
				return 2 * (size - playerIndex);
			}
			else {
				return size;
			}
		}();

		for (int i = 0; i < std::min< std::size_t >(playerPosition + 1, bridgeWidth - 1); ++i) {
			segmentOffsets[i] = transition * maxDepression * sin(M_PI / 2 * (1 + i) / (1 + playerPosition));
		}
		for (int i = playerPosition + 1; i < bridgeWidth; ++i) {
			segmentOffsets[i] = transition * maxDepression * sin(M_PI / 2 * (bridgeWidth - i) / (bridgeWidth - playerPosition));
		}
		// TODO: Figure out what this did
		/*if (playerPosition < segmentOffsets.size()) {
			SDL_Rect oldCollision = parent.getCollisionRect();
			oldCollision.y = segmentOffsets[playerPosition];
			parent.setCollisionRect(oldCollision);
		}*/
	}
	else {
		transition = std::max(0.0, transition - (Timer::getFrameTime().count() / (1000.0 / 60.0)) / 120);
		for (auto& offset : segmentOffsets) {
			offset *= sqrt(transition);
		}
	}
}

void Bridge::render(const PhysicsEntity& parent, const Camera& camera) const {
	Point relativePos = parent.getPosition() - camera.getPosition();
	int xStart = relativePos.x;
	int yStart = relativePos.y;
	for (std::size_t segment = 0; segment < bridgeWidth; ++segment) {
		relativePos.x = xStart + 16 * segment;
		relativePos.y = yStart + segmentOffsets[segment];
		for (auto index : parent.currentAnim) {
			parent.getAnim(index)->Render(SDL_Point{ int(relativePos.x), int(relativePos.y) }, 0, NULL, camera.scale);
		}
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

void Goalpost::onPlayerTouch(PhysicsEntity &parent, EntityManager &manager, Player &player) {
	player.setActCleared(true);
}

Pathswitch::Pathswitch(const FlagList& list) :
	debounce(0) {
	assert(list.size() == requiredFlags);
	static const std::unordered_map< char, Mode > flagMap{ { 'u', Mode::UNSET }, { 's', Mode::SET }, { 'i', Mode::INVERT } };
	mode = flagMap.at(list[0]);
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

void Pathswitch::onPlayerTouch(PhysicsEntity &parent, EntityManager &manager, Player &player) {
	bool path = player.getPath();
	setPath(path);
	player.setPath(path);
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
	static constexpr const std::array< const char*, 2 > enemyList{ "BEEBADNIK", "CRABBADNIK" };

	return std::find(enemyList.begin(), enemyList.end(), key) != enemyList.end();
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

static std::array < std::string, 8 > keyStrings{
	"RING", "SPRING", "BEEBADNIK", "CRABBADNIK", "BRIDGE", "PATHSWITCH", "GOALPOST", "NOCUSTOM"
};

static_assert(keyStrings.size() == static_cast<std::size_t>(entity_property_data::Key::NO_CUSTOM) + 1, "");

std::istream& entity_property_data::operator>> (std::istream& stream, Key& key) {
	std::string in;
	stream >> in;
	if (const auto foundKey = std::find(keyStrings.begin(), keyStrings.end(), in); foundKey == keyStrings.end()) {
		stream.setstate(std::istream::failbit);
	}
	else {
		key = static_cast< Key >(std::distance(keyStrings.begin(), foundKey));
	}
	return stream;
}

std::ostream& entity_property_data::operator<< (std::ostream& stream, Key key) {
	stream << keyStrings[static_cast< std::size_t >(key)];
	return stream;
}

std::string entity_property_data::to_string(entity_property_data::Key key) {
	return keyStrings[static_cast< std::size_t >(key)];
}

entity_property_data::Key entity_property_data::stringToKey(const std::string& keyString) {
	std::istringstream str{ keyString };
	Key key;
	str >> key;
	if (str.fail()) {
		throw std::invalid_argument("Invalid key string " + keyString);
	}
	return key;
}

entity_property_data::Key entity_property_data::behaviorKeyFromId(const std::string& entityId) {
	std::istringstream str{ entityId };
	Key key;
	str >> key;
	return (str.fail() ? Key::NO_CUSTOM : key);
}

bool entity_property_data::isHazard(const entity_property_data::EntityTypeId& id) {
	static const std::array < EntityTypeId, 3 > hazardList { "SPIKES", "BEEPROJECTILE" };

	return isEnemy(id) || std::find(hazardList.begin(), hazardList.end(), id) != hazardList.end();
}

entity_property_data::CustomData entity_property_data::createCustomFromKey(entity_property_data::Key key, const FlagList& flags) {
	using namespace entity_property_data;
	const std::size_t keyValue = static_cast< std::size_t > (key);
	return helpers::getOne< CustomData >([&](auto& custom) { return CustomData{ std::decay_t<decltype(custom)>{ flags } }; }, keyValue);
}

std::size_t entity_property_data::requiredFlagCount(entity_property_data::Key behaviorKey) {
	const std::size_t keyValue = static_cast< std::size_t > (behaviorKey);
	return helpers::getOne< std::size_t >([](auto& custom) { return custom.requiredFlags; }, keyValue);

}

std::size_t entity_property_data::requiredFlagCount(const std::string& behaviorKey) {
	using namespace entity_property_data;
	return requiredFlagCount(stringToKey(behaviorKey));
}
