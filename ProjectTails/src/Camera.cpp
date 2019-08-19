#include "Camera.h"
#include "Timer.h"
#include "Player.h"


Camera::Camera(const SDL_Rect& collision) :
	scale{ 2.0 },
	offset{ 0, 0 },
	frameCount{ 0 },
	currentLookOffset{ 0 }
{
	collisionRect = collision;
}

Camera::~Camera()
{
}

// Player position + offset -> camera position
void Camera::updatePos(const Player& player) {
	doublePoint velocity = player.getVelocity();
	const int left = -8 + static_cast< int >(velocity.x);
	const int right = left + 16;
	const int top = -16;
	const int bottom = top + 48;
	const int lookDirection = player.lookDirection();
	const SDL_Point error = getXY(player.getPosition()) + SDL_Point{ offset.x, offset.y + player.getYRadius() } - position;
	if (error.x < left) {
		position.x += std::max(error.x - left, -16);
	}
	if (error.x > right) {
		position.x += std::min(error.x - right, 16);
	}
	if (error.y < top) {
		position.y += std::max(error.y - top, -16);
	}
	if (error.y > bottom) {
		position.y += std::min(error.y - bottom, 16);
	}

	// Prevent going past the left edge of the screen
	position.x = std::max(int(WINDOW_HORIZONTAL_SIZE / (2.0 * scale)), position.x - offset.x) + offset.x;

	const double thisFrames = (Timer::getFrameTime().count() / (1000.0 / 60.0));
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

SDL_Point Camera::getPosition() const {
	return { position.x, int(position.y + currentLookOffset) };
}

SDL_Rect Camera::getCollisionRect() const {
	return SDL_Rect{ position.x + collisionRect.x, int(position.y + collisionRect.y + currentLookOffset), collisionRect.w, collisionRect.h };
}
