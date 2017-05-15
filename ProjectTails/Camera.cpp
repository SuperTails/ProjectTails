#include "stdafx.h"
#include "Camera.h"


Camera::Camera(SDL_Rect* collision)
{
	position = { 0,0,0,0 };
	collisionRect = (*collision);
	currentLookOffset = 0;
	frameCount = 0;
}


Camera::~Camera()
{
}

void Camera::updatePos(SDL_Rect playerPos, int lookDirection) {
	int left = -8;
	int right = left + 16;
	int top = -16;
	int bottom = top + 48;
	playerPos.x += offset.x;
	playerPos.y += offset.y;
	if (playerPos.x - position.x < left) {
		position.x += std::max(playerPos.x - position.x - left, -16);
	}
	if (playerPos.x - position.x > right) {
		position.x += std::min(playerPos.x - position.x - right, 16);
	}
	if (playerPos.y - position.y < top) {
		position.y += std::max(playerPos.y - position.y - top, -16);
	}
	if (playerPos.y - position.y > bottom) {
		position.y += std::min(playerPos.y - position.y - bottom, 16);
	}
	position.x = std::max(int(WINDOW_HORIZONTAL_SIZE * globalObjects::ratio / 2), position.x - offset.x) + offset.x;
	double thisFrames = ((globalObjects::time - globalObjects::last_time) / (1000.0 / 60.0));
	if (lookDirection != 0 && (currentLookOffset == 0 || (lookDirection > 0) == (currentLookOffset < 0))) {
		frameCount = std::min(thisFrames + frameCount, 120.0);
		if (frameCount == 120.0) {
			if (lookDirection == 1) {
				//Scroll up
				currentLookOffset = std::max(-104.0, currentLookOffset - 2 * thisFrames);
			}
			else if (lookDirection == -1) {
				//Scroll down
				currentLookOffset = std::min(88.0, currentLookOffset + 2 * thisFrames);
			}
		}
	}
	else {
		frameCount = 0;
		if (currentLookOffset > 0.0) {
			//Camera is below player
			currentLookOffset = std::max(0.0, currentLookOffset - 2 * thisFrames);
		}
		else if (currentLookOffset < 0.0) {
			//Camera is above player
			currentLookOffset = std::min(0.0, currentLookOffset + 2 * thisFrames);
		}
	}
}

SDL_Rect Camera::getPosition() {
	return SDL_Rect{ position.x, int(position.y + currentLookOffset), position.w, position.h };
}

SDL_Rect Camera::getCollisionRect() {
	return SDL_Rect{ position.x + collisionRect.x, int(position.y + collisionRect.y + currentLookOffset), collisionRect.w, collisionRect.h };
}