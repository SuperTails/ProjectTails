#pragma once
#include <experimental/type_traits>
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
	Timer timeUntilDespawn{ static_cast< long >(256.0 * 1000.0 / 60.0) };
	bool pickedUp = false;

	Ring(const FlagList& list);

	Ring() noexcept = default;
	Ring(const Ring&) noexcept = default;
	Ring(Ring&&) noexcept = default;

	Ring& operator= (const Ring& rhs) noexcept = default;

	void update(PhysicsEntity& parent, EntityManager& manager, const Player& player);

	void onPlayerTouch(PhysicsEntity& parent, EntityManager& manager, Player& player);
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

	SDL_Rect getHitbox(const PhysicsEntity& parent) const;

	void onPlayerTouch(PhysicsEntity& parent, EntityManager& manager, Player& player);
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

	void onPlayerTouch(PhysicsEntity& parent, EntityManager& manager, Player& player);
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

	void onPlayerTouch(PhysicsEntity& parent, EntityManager& manager, Player& player);
};

struct Bridge {
	static const std::size_t requiredFlags = 1;
	std::size_t bridgeWidth;
	double transition;
	std::vector < double > segmentOffsets;

	Bridge(const FlagList& list);

	Bridge();
	Bridge(const Bridge&) = default;
	Bridge(Bridge&&) = default;

	Bridge& operator= (const Bridge& rhs) = default;

	void update(PhysicsEntity& parent, EntityManager& manager, const Player& player);

	void render(const PhysicsEntity& parent, const Camera& camera) const;

	void onPlayerTouch(PhysicsEntity& parent, EntityManager& manager, Player& player);
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

	void onPlayerTouch(PhysicsEntity& parent, EntityManager& manager, Player& player);
};

struct Pathswitch {
	static const std::size_t requiredFlags = 1;
	enum class Mode { UNSET, SET, INVERT };
	Mode mode;
	int debounce;

	Pathswitch(const FlagList& list);

	Pathswitch();
	Pathswitch(const Pathswitch&) = default;
	Pathswitch(Pathswitch&&) = default;

	Pathswitch& operator= (const Pathswitch& rhs) = default;

	void update(PhysicsEntity& parent, EntityManager& manager, const Player& player);

	void setPath(bool& i);

	void triggerDebounce();

	void onPlayerTouch(PhysicsEntity& parent, EntityManager& manager, Player& player);
};

struct NoCustomData {
	NoCustomData(const FlagList& list);

	NoCustomData() = default;
	NoCustomData(const NoCustomData&) = default;
	NoCustomData(NoCustomData&&) = default;

	NoCustomData& operator= (const NoCustomData& rhs) = default;

	static const std::size_t requiredFlags = 0;
	void update(PhysicsEntity& parent, EntityManager& manager, const Player& player) {}
	
	void onPlayerTouch(PhysicsEntity& parent, EntityManager& manager, Player& player) {}
};

namespace entity_property_data {
	enum class Key { RING, SPRING, BEE_BADNIK, CRAB_BADNIK, BRIDGE, PATHSWITCH, GOALPOST, NO_CUSTOM };

	std::istream& operator>> (std::istream& stream, Key& key);

	std::ostream& operator<< (std::ostream& stream, Key key);

	std::string to_string(Key key);

	Key stringToKey(const std::string& keyString);

	Key behaviorKeyFromId(const std::string& entityId);

	template < typename T >
	using hasRenderT = decltype(std::declval< T >().render(std::declval< PhysicsEntity >(), std::declval< SDL_Rect >()));

	template < typename T >
	using isConstAnimationFunctionT = decltype(std::declval< T >()(std::declval< const std::unique_ptr< Animation >& >()));

#ifdef __clang__
	template < typename T >
	constexpr const bool hasRender = std::experimental::is_detected< hasRenderT, T >();

	template < typename T >
	constexpr const bool isConstAnimationFunction = std::experimental::is_detected< isConstAnimationFunctionT, T >();
#else
	template < typename T >
	concept bool Renderable = std::experimental::is_detected< hasRenderT, T >::value;

	template < typename T >
	concept bool isConstAnimationFunction = std::experimental::is_detected< isConstAnimationFunctionT, T >();
#endif

	typedef std::variant< Ring, Spring, BeeBadnik, CrabBadnik, Bridge, Pathswitch, Goalpost, NoCustomData > CustomData;

	struct EntityType {
		double defaultGravity = 0.0;
		std::pair< double, double > defaultVelocity;
		bool isHazard;
		std::vector < AnimStruct > animationTypes;
		SDL_Rect collisionRect;
		Key behaviorKey;
	};

	typedef std::string EntityTypeId;

	extern std::unordered_map < std::string, EntityType > entityTypes;

	const EntityType& getEntityTypeData(const EntityTypeId& id); 

	bool isEnemy(const EntityTypeId& key);

	bool isHazard(const EntityTypeId& id);

	namespace helpers {
		template < typename F, std::size_t... I >
		void forAllCustomImpl(F&& f, std::index_sequence<I...>) {
			int ignored = (std::invoke(f, std::variant_alternative_t< I, CustomData >{}, I), ..., 0);
			(void)ignored;
		}

		template < typename F >
		void forAllCustom(F&& f) {
			forAllCustomImpl(f, std::make_index_sequence< std::variant_size_v< CustomData > >{});
		}

		template < typename R, typename F >
		R getOne(F&& f, std::size_t i) {
			if (i >= std::variant_size_v< CustomData >) {
				throw std::invalid_argument("Invalid custom index!");
			}

			R r;
			forAllCustom([&](auto p, std::size_t j) {
				if (i == j) {
					r = f(p);
				}
			});
			return r;
		}

		template < typename R, typename F >
		R getOne(F&& f, CustomData& data, std::size_t i) {
			if (i >= std::variant_size_v< CustomData >) {
				throw std::invalid_argument("Invalid custom index!");
			}
			
			R r;
			forAllCustom([&](auto p, std::size_t j) {
				if (i == j) {
					r = f(std::get< decltype(p) >(data));
				}
			});
			return r;
		}

		template < typename R, typename F >
		R getOne(F&& f, const CustomData& data, std::size_t i) {
			if (i >= std::variant_size_v< CustomData >) {
				throw std::invalid_argument("Invalid custom index!");
			}
			
			R r;
			forAllCustom([&](auto p, std::size_t j) {
				if (i == j) {
					r = f(std::get< decltype(p) >(data));
				}
			});
			return r;
		}
	}

	CustomData createCustomFromKey(Key key, const FlagList& flags);

	std::size_t requiredFlagCount(Key behaviorKey);

	std::size_t requiredFlagCount(const std::string& behaviorKey);
}
