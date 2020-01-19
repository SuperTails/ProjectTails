#include "stdafx.h"
#include "InputComponent.h"
#include <iostream>

// Returns true if the given key is currently held
bool InputComponent::getKeyState(int k) const {
	if (k & (1 << 30)) {
		return keyStates[(k & ~(1 << 30)) + 128];
	}
	return keyStates[k];
}


bool InputComponent::getKeyState(KeyMap input) const { return getKeyState(static_cast< char >(input)); }

// Returns true if the given key was just pressed on this frame
bool InputComponent::getKeyPress(int k) const {
	if (k & (1 << 30)) {
		return keyPress[(k & ~(1 << 30)) + 128];
	}
	return keyPress[k];
}


bool InputComponent::getKeyPress(KeyMap input) const { return getKeyPress(static_cast< char >(input)); }

//Updates the KeyStates array values
void InputComponent::updateKeys() {
	keyPress = 0;
	bool mouseWheelEvent = false;
	SDL_Event keyEvent{};
	while (SDL_PollEvent(&keyEvent)) {
		int sym;
		switch (keyEvent.type) {
		case SDL_KEYDOWN:
			sym = keyEvent.key.keysym.sym;
			if (sym & (1 << 30)) {
				sym &= ~(1 << 30);
				sym += 128;
			}
			if (sym < keyCount) {
				keyPress[sym] = !keyStates[sym];
				keyStates[sym] = true;
			}
			break;
		case SDL_KEYUP:
			sym = keyEvent.key.keysym.sym & ~(1 << 30);
			if (sym & (1 << 30)) {
				sym &= ~(1 << 30);
				sym += 128;
			}
			if (sym < keyCount) {
				keyStates[sym] = false;
			}
			break;
		case SDL_MOUSEWHEEL:
			mouseWheelEvent = true;
			mouseWheel = keyEvent.wheel.y;
			break;
		default:
			break;
		}
	}
	if (!mouseWheelEvent) {
		mouseWheel = 0;
	}
}
