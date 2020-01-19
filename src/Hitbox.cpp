#include "Hitbox.h"
#include "Miscellaneous.h"
#include "Camera.h"
#include "Drawing.h"

HitboxForm::HitboxForm(Rect rect) :
	boxes({ rect })
{

}

HitboxForm::HitboxForm(const std::vector< Rect >& rects) :
	boxes(rects)
{
	
}

bool rectIntersection(Rect a, Rect b) {
	return a.x < b.x + b.w &&
	       a.x + a.w > b.x &&
	       a.y < b.y + b.h &&
	       a.y + a.h > b.y;
}

bool HitboxForm::intersects(const HitboxForm& other, Point otherCenter) {
	for (auto boxA : boxes) {
		for (auto boxB : other.boxes) {
			Rect otherBox{ boxB.x + otherCenter.x, boxB.y + otherCenter.y, boxB.w, boxB.h };
			if (rectIntersection(boxA, otherBox)) {
				return true;
			}
		}
	}

	return false;
}

std::optional< Rect > HitboxForm::getAABoundingBox() const {
	if (boxes.empty()) {
		return {};
	}

	auto minXRect = std::min_element(boxes.begin(), boxes.end(), [](auto l, auto r) { return l.x < r.x; });
	auto minYRect = std::min_element(boxes.begin(), boxes.end(), [](auto l, auto r) { return l.y < r.y; });
	auto maxXRect = std::max_element(boxes.begin(), boxes.end(), [](auto l, auto r) { return l.x + l.w < r.x + r.w; });
	auto maxYRect = std::max_element(boxes.begin(), boxes.end(), [](auto l, auto r) { return l.y + l.h < r.y + r.h; });

	const Point topLeft{ minXRect->x, minYRect->y };
	const Point bottomRight{ maxXRect->x + maxXRect->w, maxYRect->y + maxYRect->h };
	const Vector2 size = bottomRight - topLeft;

	return Rect{ topLeft.x, topLeft.y, size.x, size.y };
}

void HitboxForm::render(const Camera& camera, Point center) const {
	for (Rect temp : boxes) {
		temp.x += center.x;
		temp.y += center.y;
		drawing::drawRect(globalObjects::renderer, camera, temp, drawing::Color{ 255, 255, 255 }, false);
	}
}

bool intersects(const PhysicsEntity& a, const PhysicsEntity& b) {
	return a.getHitbox().intersects(b.getHitbox(), b.getPosition() - a.getPosition());
}

AbsoluteHitbox::AbsoluteHitbox(HitboxForm rawForm, Point center) :
	hitbox(rawForm)
{
	for (auto& box : hitbox.boxes) {
		box.x += center.x;
		box.y += center.y;
	}
}
