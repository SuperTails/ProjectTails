#include "Drawing.h"

namespace drawing {
	void drawPoint(SDL_Renderer* renderer, const Camera& camera, Point pos, Color color, double size) {
		SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, SDL_ALPHA_OPAQUE);

		pos -= camera.getPosition();
		pos.x *= camera.scale;
		pos.y *= camera.scale;

		pos.x -= (size - 1.0) / 2.0;
		pos.y -= (size - 1.0) / 2.0;

		SDL_Rect dest{ int(pos.x), int(pos.y), int(size), int(size) };

		SDL_RenderDrawRect(renderer, &dest);
	}

	void drawRect(SDL_Renderer* renderer, const Camera& camera, Rect rect, Color color, bool filled) {
		SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, SDL_ALPHA_OPAQUE);

		rect.x -= camera.getPosition().x;
		rect.y -= camera.getPosition().y;
		
		rect.x *= camera.scale;
		rect.y *= camera.scale;
		rect.w *= camera.scale;
		rect.h *= camera.scale;

		SDL_Rect dest{ int(rect.x), int(rect.y), int(rect.w), int(rect.h) };

		if (filled) {
			SDL_RenderFillRect(renderer, &dest);
		}
		else {
			SDL_RenderDrawRect(renderer, &dest);
		}
	}

	void drawLine(SDL_Renderer* renderer, const Camera& camera, Point begin, Point end, Color color) {
		SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, SDL_ALPHA_OPAQUE);

		begin -= camera.getPosition();
		end -= camera.getPosition();

		begin.x *= camera.scale;
		begin.y *= camera.scale;

		end.x *= camera.scale;
		end.y *= camera.scale;
		
		SDL_RenderDrawLine(renderer, begin.x, begin.y, end.x, end.y);
	}
};
