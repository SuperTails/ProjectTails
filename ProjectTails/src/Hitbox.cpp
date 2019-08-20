#include "Hitbox.h"
#include "Miscellaneous.h"
#include "Camera.h"

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

Rect HitboxForm::getBox() const {
	return box;
}

void HitboxForm::render(const Camera& camera, Point center) const {
	Rect temp = box;
	temp.x += center.x;
	temp.y += center.y;
	temp.x -= camera.getPosition().x;
	temp.y -= camera.getPosition().y;
	SDL_Rect collision = static_cast< SDL_Rect >(temp) * camera.scale;
	SDL_SetRenderDrawColor(globalObjects::renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
	SDL_RenderDrawRect(globalObjects::renderer, &collision);
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
