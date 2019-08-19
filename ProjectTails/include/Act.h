#pragma once
#include <memory>
#include <unordered_map>
#include <vector>
#include <string>
#include <iosfwd>
#include <list>

struct Version;
class Camera;
class Player;
class PhysicsEntity;
struct PhysStruct;
class Ground;
class Animation;

struct SDL_Rect;
struct SDL_Point;

class Act
{
public:
	enum class ActType : unsigned char;

	Act() = default;

	Act(const std::string& path);

	Act(const int& num, const std::string& name1, const std::vector < PhysicsEntity >& ent, const SDL_Rect& w, const ActType& a, double screenRatio, const SDL_Point& levelSize, const std::vector < std::vector < Animation > >& backgnd, std::vector < Ground >& ground);

	Act(const Act& act);

	Act(Act&& other) = default;

	void renderObjects(Player& player, Camera& cam);

	void updateCollisions(Player& player, std::vector<bool>& toDestroy, std::vector< std::unique_ptr< PhysicsEntity > >& toAdd);
	void updateEntities(Player& player, Camera& cam);

	int numEntities() const { return entities.size(); };

	std::string getName() const { return name; };
	int getNumber() const { return number; };

	std::vector< std::vector< Ground > >& getGround() { return solidTiles; };

	const std::vector< std::vector< Ground > >& getGround() const { return solidTiles; };

	std::vector< std::unique_ptr < PhysicsEntity > >& getEntities() { return entities; };

	const std::vector< std::unique_ptr < PhysicsEntity > >& getEntities() const { return entities; };

	void setEntities(std::vector< std::unique_ptr< PhysicsEntity > >&& newList) {
		entities = std::move(newList);
	};

	ActType getType() { return aType; };

	void loadVersion(std::ifstream& file, Version version);

	const std::string& getBlockPrefix() const { return blockPrefix; };

	const std::string& getBackgroundFolder() const { return backgroundFolder; };

	void save(std::ostream& stream) const;

private:
	typedef std::vector< std::unique_ptr< PhysicsEntity > > entityList;
	typedef std::vector < PhysStruct > entityLoadList;
	typedef std::list < Ground* > solidRenderList;

	typedef entityList::iterator entityListIterator;
	typedef entityLoadList::iterator entityLoadIterator;
	typedef solidRenderList::iterator solidRenderListIterator;

	int number = 0; //Which zone it is, title screen = 0.
	std::string name{}; //The zone's name
	entityList entities{}; //Array for the generic physics objects
	std::vector< std::vector < Ground > > solidTiles{}; //Array for ground
	std::vector< std::vector < Animation > > background{};

	std::string blockPrefix{};
	std::string backgroundFolder{};

	ActType aType;

	void renderBlockLayer(const Camera& cam, int layer) const;
};

std::istream& operator>> (std::istream& stream, Act::ActType& type);
std::ostream& operator<< (std::ostream& stream, Act::ActType  type);
