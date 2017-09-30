#pragma once
#include <variant>
#include <chrono>
#include <memory>
#include <string>
#include <vector>
#include <SDL.h>
#include <unordered_map>
#include <functional>
#include "Timer.h"

class Player;
class PhysicsEntity;
class EntityManager;
struct AnimStruct;
struct doublePoint;

typedef std::vector < char > FlagList;

struct Ring {
	static const std::size_t requiredFlags = 0;
	Timer timeUntilDespawn;
	bool pickedUp;

	Ring(const FlagList& list);

	Ring();
	Ring(const Ring&) noexcept = default;
	Ring(Ring&&) noexcept = default;

	Ring& operator= (const Ring& rhs) = default;

	void update(PhysicsEntity& parent, EntityManager& manager, const Player& player);
};

struct Spring {
	static const std::size_t requiredFlags = 1;
	enum class Direction { RIGHT = 1, DOWN = 2, LEFT = 4, UP = 8, RIGHTDOWN = RIGHT | DOWN, LEFTDOWN = LEFT | DOWN, LEFTUP = LEFT | UP, RIGHTUP = RIGHT | UP };
	Direction direction;
	Timer extendedTime;

	Spring(const FlagList& list);

	Spring();
	Spring(const Spring&) = default;
	Spring(Spring&&) = default;

	Spring& operator= (const Spring& rhs) = default;

	void update(PhysicsEntity& parent, EntityManager& manager, const Player& player);

	void bounceEntity(const PhysicsEntity& parent, PhysicsEntity& entity);
};

struct BeeBadnik {
	static const std::size_t requiredFlags = 0;
	Timer timeUntilMove;
	bool hasFired;

	BeeBadnik(const FlagList& list);

	BeeBadnik();
	BeeBadnik(const BeeBadnik&) = default;
	BeeBadnik(BeeBadnik&&) = default;

	BeeBadnik& operator= (const BeeBadnik& rhs) = default;

	void update(PhysicsEntity& parent, EntityManager& manager, const Player& player);
};

struct CrabBadnik {
	static const std::size_t requiredFlags = 0;
	Timer timeUntilWalk;
	Timer timeUntilStop;
	int currentDirection;

	CrabBadnik(const FlagList& list);

	CrabBadnik();
	CrabBadnik(const CrabBadnik&) = default;
	CrabBadnik(CrabBadnik&&) = default;

	CrabBadnik& operator= (const CrabBadnik& rhs) = default;

	void update(PhysicsEntity& parent, EntityManager& manager, const Player& player);
};

struct Bridge {
	static const std::size_t requiredFlags = 1;
	std::size_t bridgeWidth;
	int playerPosition;
	double transition;
	std::vector < double > segmentOffsets;

	Bridge(const FlagList& list);

	Bridge();
	Bridge(const Bridge&) = default;
	Bridge(Bridge&&) = default;

	Bridge& operator= (const Bridge& rhs) = default;

	void update(PhysicsEntity& parent, EntityManager& manager, const Player& player);

	void render(const PhysicsEntity& parent, const SDL_Rect& cameraPosition) const;
};

struct Goalpost {
	static const std::size_t requiredFlags = 0;
	Timer timeUntilStop;
	bool finishedSpinning;

	Goalpost(const FlagList& list);

	Goalpost();
	Goalpost(const Goalpost&) = default;
	Goalpost(Goalpost&&) = default;

	Goalpost& operator= (const Goalpost& rhs) = default;

	void update(PhysicsEntity& parent, EntityManager& manager, const Player& player);
};

struct Pathswitch {
	static const std::size_t requiredFlags = 1;
	int debounce;
	enum class Mode { UNSET, SET, INVERT };
	Mode mode;

	Pathswitch(const FlagList& list);

	Pathswitch();
	Pathswitch(const Pathswitch&) = default;
	Pathswitch(Pathswitch&&) = default;

	Pathswitch& operator= (const Pathswitch& rhs) = default;

	void update(PhysicsEntity& parent, EntityManager& manager, const Player& player);

	void setPath(bool& i);

	void triggerDebounce();
};

struct NoCustomData {
	NoCustomData(const FlagList& list);

	NoCustomData() = default;
	NoCustomData(const NoCustomData&) = default;
	NoCustomData(NoCustomData&&) = default;

	NoCustomData& operator= (const NoCustomData& rhs) = default;

	static const std::size_t requiredFlags = 0;
	void update(PhysicsEntity& parent, EntityManager& manager, const Player& player) {}
};

template < typename T >
struct IsValidImpl {
private:
	template < typename Param > constexpr auto isValid(int)
	-> decltype(std::declval<T>()(std::declval<Param>()), std::true_type()) {
		return std::true_type{};
	}

	template < typename Param > constexpr std::false_type isValid(...) {
		return std::false_type{};
	}
public:

	template < typename Param > constexpr auto operator()(const Param& p) {
		return isValid<Param>(int{});
	}
};

template < typename T > constexpr auto isValid(const T& t) {
	return IsValidImpl<T>{};
}

namespace entity_property_data {
	enum class Keys { RING, SPRING, BEE_BADNIK, CRAB_BADNIK, BRIDGE, PATHSWITCH, GOALPOST, NO_CUSTOM };
	extern std::array < std::string, 8 > keyStrings;

	const std::string& keyToString(const Keys& key);
	Keys stringToKey(const std::string& keyString);

	auto hasRender = isValid([](const auto& x) constexpr -> decltype(x.render(std::declval<PhysicsEntity>(), std::declval<SDL_Rect>())){});

	auto isConstAnimationFunction = isValid([](const auto& x) constexpr -> decltype(x(std::declval<const std::unique_ptr<Animation>&>())){});

	typedef std::variant < Ring, Spring, BeeBadnik, CrabBadnik, Bridge, Pathswitch, Goalpost, NoCustomData > CustomData;

	struct EntityType {
		double defaultGravity = 0.0;
		std::pair< double, double > defaultVelocity;
		bool isHazard;
		std::vector < AnimStruct > animationTypes;
		SDL_Rect collisionRect;
		Keys behaviorKey;
	};

	typedef std::string EntityTypeId;

	extern std::unordered_map < std::string, EntityType > entityTypes;

	const EntityType& getEntityTypeData(const EntityTypeId& id); 

	bool isEnemy(const EntityTypeId& key);

	bool isHazard(const EntityTypeId& id);

	namespace helpers {
		template < typename F, std::size_t... I >
		void forAllCustomImpl(F&& f, std::index_sequence<I...>) {
			int ignored[] = { 0, (std::invoke(f, std::variant_alternative_t<I, CustomData>{}, I), 0)... };
			(void)ignored;
		}

		template < typename F >
		void forAllCustom(F&& f) {
			forAllCustomImpl(f, std::make_index_sequence<std::variant_size_v<CustomData>>());
		}
		
		template < typename R, typename F >
		R getOne(F&& f, std::size_t i) {
			R r;
			bool found = false;
			forAllCustom([&](auto p, std::size_t j) {
				if (i == j) {
					r = f(p);
					found = true;
				}
			});
			if (!found) {
				throw std::invalid_argument("Invalid custom index!");
			}
			return r;
		}
	}

	CustomData createCustomFromKey(Keys key, const FlagList& flags);

	std::size_t requiredFlagCount(Keys behaviorKey);

	std::size_t requiredFlagCount(const std::string& behaviorKey);
}
