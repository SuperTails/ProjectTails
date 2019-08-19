#pragma once
#include "Shapes.h"

class HitboxForm {
public:
	HitboxForm(Rect rect);

	// otherCenter is the relative position of the other bounding box compared to this one
	bool intersects(HitboxForm other, Point otherCenter);

private:
	Rect box;
};
