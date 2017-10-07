#pragma once
#include "Player.h"
#include <string>

class PlayerTest {
public:
	bool runTests();
private:
	void reportTest(bool result, const std::string& errorMessage);
};
