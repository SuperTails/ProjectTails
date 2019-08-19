#pragma once
#include "PhysicsEntity.h"
#include "Constants.h"
#include "SDL.h"
#include "Miscellaneous.h"
class Camera : public PhysicsEntity
{
public:
	Camera(const SDL_Rect& collision);
	~Camera();

	void setOffset(SDL_Point pos) { offset = pos; };
	SDL_Point getOffset() const { return offset; };
	void updatePos(const Player& player);
	SDL_Point getPosition() const;
	SDL_Rect getCollisionRect() const;

	double scale;

private:

	SDL_Point offset; //The position of the camera relative to the player (usually negative)
	double frameCount;
	double currentLookOffset;
};

