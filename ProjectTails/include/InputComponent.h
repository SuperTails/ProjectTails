#pragma once
#include <SDL2/SDL.h>
#include <bitset>

class InputComponent
{
public:
	enum class KeyMap { LEFT = 'a', RIGHT = 'd', UP = 'w', DOWN = 's', JUMP = ' ' };

	// Includes keycodes and scancodes
	static constexpr const std::size_t keyCount = 128 + 282;

	void updateKeys(); 

	// Returns true if the given key is currently held
	bool getKeyState(int k) const;
	bool getKeyState(KeyMap input) const;

	// Returns true if the given key was just pressed on this frame
	bool getKeyPress(int k) const;
	bool getKeyPress(KeyMap input) const;

	int getWheel() const { return mouseWheel; };

private:
	std::bitset< keyCount > keyStates = 0;
	std::bitset< keyCount > keyPress = 0;
	int mouseWheel = 0;
};

