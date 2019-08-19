#include "stdafx.h"
#include "InputComponent.h"
#include <iostream>


InputComponent::InputComponent() :
	KeyStates(0),
	KeyPress(0),
	mouseWheel(0)
{
}

//Updates the KeyStates array values
void InputComponent::UpdateKeys() {
	KeyPress = 0;
	bool mouseWheelEvent = false;
	while (SDL_PollEvent(&KeyEvent)) {
		switch (KeyEvent.type) {

		#define KEY(key) \
			KeyPress[key] = !KeyStates[key]; \
			KeyStates[key] = true; \
			break;

		case SDL_KEYDOWN:
			switch (KeyEvent.key.keysym.sym) {
			case SDLK_w: KEY(W)
			case SDLK_a: KEY(A)
			case SDLK_s: KEY(S)
			case SDLK_d: KEY(D)
			case SDLK_j: KEY(J)
			case SDLK_m: KEY(M)
			case SDLK_UP: KEY(UP)
			case SDLK_LEFT: KEY(LEFT)
			case SDLK_RIGHT: KEY(RIGHT)
			case SDLK_DOWN: KEY(DOWN)
			case SDLK_n: KEY(N)
			case SDLK_x: KEY(X)
			case SDLK_LEFTBRACKET: KEY(LBRACKET)
			case SDLK_RIGHTBRACKET: KEY(RBRACKET)
			case SDLK_f: KEY(F)
			case SDLK_r: KEY(R)
			}
			break;
			
		#undef KEY

		case SDL_KEYUP:
			switch (KeyEvent.key.keysym.sym) {
			case SDLK_w:
				KeyStates[W] = false;
				break;
			case SDLK_a:
				KeyStates[A] = false;
				break;
			case SDLK_s:
				KeyStates[S] = false;
				break;
			case SDLK_d:
				KeyStates[D] = false;
				break;
			case SDLK_j:
				KeyStates[J] = false;
				break;
			case SDLK_m:
				KeyStates[M] = false;
				break;
			case SDLK_UP:
				KeyStates[UARROW] = false;
				break;
			case SDLK_LEFT:
				KeyStates[LARROW] = false;
				break;
			case SDLK_RIGHT:
				std::cout << "r up\n";
				KeyStates[RARROW] = false;
				break;
			case SDLK_DOWN:
				KeyStates[DARROW] = false;
				break;
			case SDLK_n:
				KeyStates[N] = false;
				break;
			case SDLK_x:
				KeyStates[X] = false;
				break;
			case SDLK_LEFTBRACKET:
				KeyStates[LBRACKET] = false;
				break;
			case SDLK_RIGHTBRACKET:
				KeyStates[RBRACKET] = false;
				break;
			case SDLK_f:
				KeyStates[F] = false;
				break;
			case SDLK_r:
				KeyStates[R] = false;
			default:
				break;
			}
			break;

		case SDL_MOUSEWHEEL:
			mouseWheelEvent = true;
			mouseWheel = KeyEvent.wheel.y;
			break;
			
		default:
			break;
		}
	}
	if (!mouseWheelEvent) {
		mouseWheel = 0;
	}
}

InputComponent::~InputComponent()
{
}
