#pragma once
#include "Shapes.h"
#include <vector>
#include <optional>

class Camera;
class PhysicsEntity;

class HitboxForm {
public:
	HitboxForm() = default;

	explicit HitboxForm(Rect rect);

	HitboxForm(const std::vector< Rect >& rects);

	// otherCenter is the relative position of the other bounding box compared to this one
	bool intersects(const HitboxForm& other, Point otherCenter);

	void render(const Camera& camera, Point center) const;

	std::optional< Rect > getAABoundingBox() const;

	friend class AbsoluteHitbox;

private:
	std::vector< Rect > boxes{};
};

struct AbsoluteHitbox {
	AbsoluteHitbox(HitboxForm rawForm, Point center);

	HitboxForm hitbox;
};

bool intersects(const PhysicsEntity& a, const PhysicsEntity& b);
