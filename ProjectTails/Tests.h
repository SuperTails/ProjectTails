#pragma once
#include <string>

class Tests {
public:
	static bool runTests();

	static bool doTests;
private:
	static void assertTest(bool result, const std::string& errorMessage);
};
