#include "Tests.h"
#include "Player.h"
#include "CollisionTile.h"
#include "Ground.h"
#include <algorithm>

bool Tests::doTests = false;

bool Tests::runTests() {
	Player player;

	std::array< int, CollisionTile::heightMapSize > heights;
	heights.fill(0);
	CollisionTile emptyTile(heights, 0.0);
	heights.fill(16);
	CollisionTile fullTile(heights, 0.0);
	std::vector < CollisionTile > collisionList{ emptyTile, fullTile };
	Ground::setCollisionList(collisionList);

	{
		Ground::Layer collisionLayer;
		std::fill(collisionLayer.begin(), collisionLayer.begin() + GROUND_SIZE / 2, Ground::Tile{ 0, 0 });
		std::fill(collisionLayer.begin() + GROUND_SIZE / 2, collisionLayer.end(), Ground::Tile{ 1, 0 });
		Ground::Layer graphicsLayer;
		std::fill(graphicsLayer.begin(), graphicsLayer.end(), Ground::Tile{ 0, 0 });
		Ground::addTile({ { graphicsLayer }, { collisionLayer } });
	}
	
	std::vector< std::vector< Ground > > tiles(1, std::vector< Ground >(1, Ground{ 0, { 0, 0 } }));
	player.setAnimation(1);
	player.position = { 64, 44 };
	std::cout << player.getPosition().y << "\n";
	assertTest(Mode::GROUND == player.getMode(), "Player default mode should be 0, was " + Player::modeToString(player.getMode()));
	const auto range1 = player.getRange(Player::Sensor::A);
	std::cout << std::get<0>(range1).first << ", " << std::get<0>(range1).second << " " << std::get<1>(range1).first << ", " << std::get<1>(range1).second << " " << std::get<2>(range1) << "\n";
	const auto floor1 = player.checkSensor(Player::Sensor::A, tiles);
	if (floor1.has_value()) {
		//std::cout << "Floor1: " << std::get<0>(*floor1) << " " << std::get<1>(*floor1) << " " << std::get<2>(*floor1) << "\n";
	}
	else {
		std::cout << "Floor1 has no value\n";
	}
	Ground::clearTiles();
}

void Tests::assertTest(bool result, const std::string& errorMessage) {
	if (!result) {
		std::cerr << errorMessage;
	}
}
