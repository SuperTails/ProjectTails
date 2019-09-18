#include "Hitbox.h"
#include "Miscellaneous.h"
#include "Camera.h"
#include "Drawing.h"

HitboxForm::HitboxForm(Rect rect) :
	box(rect)
{

}

bool HitboxForm::intersects(HitboxForm other, Point otherCenter) {
	other.box.x += otherCenter.x;
	other.box.y += otherCenter.y;

	// Intersection between two rectangles
	return  box.x < other.box.x + other.box.w &&
		box.x + box.w > other.box.x &&
		box.y < other.box.y + other.box.h &&
		box.y + box.h > other.box.y;
}

Rect HitboxForm::getAABoundingBox() const {
	return box;
}

void HitboxForm::render(const Camera& camera, Point center) const {
	Rect temp = box;
	temp.x += center.x;
	temp.y += center.y;
	drawing::drawRect(globalObjects::renderer, camera, temp, drawing::Color{ 255, 255, 255 }, false);
}

bool intersects(const PhysicsEntity& a, const PhysicsEntity& b) {
	return a.getHitbox().intersects(b.getHitbox(), b.getPosition() - a.getPosition());
}

AbsoluteHitbox::AbsoluteHitbox(HitboxForm rawForm, Point center) :
	box(rawForm)
{
	box.box.x += center.x;
	box.box.y += center.y;
}
