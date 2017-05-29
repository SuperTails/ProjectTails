#pragma once
#include "SDL.h"
#include <vector>
class InputComponent
{
public:
	enum Key { W,A,S,D,J,M,LARROW,UARROW,RARROW,DARROW,N,X,LBRACKET,RBRACKET,F,R,KEYS_SIZE };

	enum KeyMap { LEFT = A, RIGHT = D, UP = W, DOWN = S, JUMP = J };
	

	void UpdateKeys(); 

	// Returns true if the given key is currently held
	bool GetKeyState(int k) const { return KeyStates[k];  };

	// Returns true if the given key was just pressed on this frame
	bool GetKeyPress(int k) const { return KeyPress[k]; };

	int GetWheel() const { return mouseWheel; };

	SDL_Event KeyEvent;

	InputComponent();
	~InputComponent();

private:
	std::vector < bool > KeyStates;
	std::vector < bool > KeyPress;
	int mouseWheel;
};

