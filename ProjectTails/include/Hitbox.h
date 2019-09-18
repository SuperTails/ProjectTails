#pragma once
#include "Shapes.h"

class Camera;
class PhysicsEntity;

class HitboxForm {
public:
	HitboxForm() = default;

	HitboxForm(Rect rect);

	// otherCenter is the relative position of the other bounding box compared to this one
	bool intersects(HitboxForm other, Point otherCenter);

	void render(const Camera& camera, Point center) const;

	Rect getAABoundingBox() const;

	friend class AbsoluteHitbox;

private:
	Rect box{ 0, 0, 0, 0 };
};

struct AbsoluteHitbox {
	AbsoluteHitbox(HitboxForm rawForm, Point center);

	HitboxForm box;
};

bool intersects(const PhysicsEntity& a, const PhysicsEntity& b);
