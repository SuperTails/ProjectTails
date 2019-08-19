#include "Hitbox.h"

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
