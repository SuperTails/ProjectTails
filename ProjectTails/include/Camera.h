#pragma once
#include "PhysicsEntity.h"
#include "Constants.h"
#include "SDL.h"
#include "Miscellaneous.h"
#include "Shapes.h"

class Camera : public PhysicsEntity
{
public:
	Camera(const Rect& view);
	~Camera();

	void setOffset(Vector2 pos) { offset = pos; };
	Vector2 getOffset() const { return offset; };
	void updatePos(const Player& player);
	Point getPosition() const;
	
	Rect getViewArea() const;

	double scale;

private:
	Rect viewWindow;

	Vector2 offset; //The position of the camera relative to the player (usually negative)
	double frameCount;
	double currentLookOffset;
};

