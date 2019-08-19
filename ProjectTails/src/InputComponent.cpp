#include "stdafx.h"
#include "InputComponent.h"


InputComponent::InputComponent()
{
	KeyStates.resize(KEYS_SIZE);
	KeyPress.resize(KEYS_SIZE);
	for (int i = 0; i < KeyStates.size(); i++) {
		KeyStates[i] = false;
		KeyPress[i] = false;
	}
	mouseWheel = 0;
}

//Updates the KeyStates array values
void InputComponent::UpdateKeys() {
	for (int i = 0; i < KeyPress.size(); i++) {
		KeyPress[i] = false;
	}
	bool mouseWheelEvent = false;
	while (SDL_PollEvent(&KeyEvent)) {
		switch (KeyEvent.type) {

		case SDL_KEYDOWN:
			switch (KeyEvent.key.keysym.sym) {
			case SDLK_w:
				KeyStates[W] = true;
				KeyPress[W] = true;
				break;
			case SDLK_a:
				KeyStates[A] = true;
				KeyPress[A] = true;
				break;
			case SDLK_s:
				KeyStates[S] = true;
				KeyPress[S] = true;
				break;
			case SDLK_d:
				KeyStates[D] = true;
				KeyPress[D] = true;
				break;
			case SDLK_j:
				KeyStates[J] = true;
				KeyPress[J] = true;
				break;
			case SDLK_m:
				KeyStates[M] = true;
				KeyPress[M] = true;
				break;
			case SDLK_UP:
				KeyStates[UARROW] = true;
				KeyPress[UARROW] = true;
				break;
			case SDLK_LEFT:
				KeyStates[LARROW] = true;
				KeyPress[LARROW] = true;
				break;
			case SDLK_RIGHT:
				KeyStates[RARROW] = true;
				KeyPress[RARROW] = true;
				break;
			case SDLK_DOWN:
				KeyStates[DARROW] = true;
				KeyPress[DARROW] = true;
				break;
			case SDLK_n:
				KeyStates[N] = true;
				KeyPress[N] = true;
				break;
			case SDLK_x:
				KeyStates[X] = true;
				KeyPress[X] = true;
				break;
			case SDLK_LEFTBRACKET:
				KeyStates[LBRACKET] = true;
				KeyPress[LBRACKET] = true;
				break;
			case SDLK_RIGHTBRACKET:
				KeyStates[RBRACKET] = true;
				KeyPress[RBRACKET] = true;
				break;
			case SDLK_f:
				KeyStates[F] = true;
				KeyPress[F] = true;
				break;
			case SDLK_r:
				KeyStates[R] = true;
				KeyPress[R] = true;
			default:
				break;
			}
			break;
			
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
