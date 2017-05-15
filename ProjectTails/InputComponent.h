#pragma once
#include "SDL.h"
#include <vector>
class InputComponent
{
public:
	enum Key { W,A,S,D,J,M,LARROW,UARROW,RARROW,DARROW,N,X,LBRACKET,RBRACKET,F,R,KEYS_SIZE };

	enum KeyMap { LEFT = A, RIGHT = D, UP = W, DOWN = S, JUMP = J };
	

	void UpdateKeys(); 

	//Returns whether the given key is pressed or not
	bool GetKeyState(int k) { return KeyStates[k];  };

	bool GetKeyPress(int k) { return KeyPress[k]; };

	int GetWheel() { return mouseWheel; };

	SDL_Event KeyEvent;

	InputComponent();
	~InputComponent();

private:
	std::vector < bool > KeyStates;
	std::vector < bool > KeyPress;
	int mouseWheel;
};

