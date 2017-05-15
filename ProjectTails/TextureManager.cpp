#include "stdafx.h"
#include "TextureManager.h"

int sign(int val) {
	return (0 < val) - (val < 0);
}

FloatPoint GetRotatedPos(int x, int y, int r) {
	double sizeX, sizeY;

	double rot_rad = r * M_PI / 180;

	sizeX = (x)*cos(rot_rad) - (y)*sin(rot_rad);
	sizeY = (x)*sin(rot_rad) - (y)*sin(rot_rad);

	sizeX = sign(sizeX) * floor(abs(sizeX));
	sizeY = sign(sizeY) * floor(abs(sizeY));

	return{ sizeX, sizeY };
}

TextureManager::TextureManager(std::string path, SDL_Renderer* ren, SDL_Rect* pos, int rot)
{
	surface = IMG_Load(path.c_str());
	renderer = ren;
	position = pos;
	rotation = rot;
}

void TextureManager::Update() {
	SDL_Surface* temp;
	//SDL_BlitSurface(surface, NULL, temp, NULL);

	FloatPoint s = GetRotatedPos(surface->w, surface->h, rotation);
	
	if (rotation != 0) {
		for (int x = 0; x < s.x; x++) {
			for (int y = 0; y < s.y; y++) {
				int bytesperpixel = surface->format->BytesPerPixel;
				Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bytesperpixel;
				Uint32 color = *p;
			}
		}
	}
	
}

TextureManager::~TextureManager()
{
}

