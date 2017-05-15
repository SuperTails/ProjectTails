#include "stdafx.h"
#include "Ground.h"

SDL_Window* Ground::window;
SDL_Surface* Ground::map;
std::vector < CollisionTile > Ground::tileList;
std::string Ground::mPath;

void Ground::setMap(std::string mapPath) {
	SDL_FreeSurface(map);
	mPath = mapPath;
	SDL_Surface* temp = IMG_Load(mapPath.c_str());
	map = SDL_ConvertSurface(temp, SDL_GetWindowSurface(window)->format, SDL_RLEACCEL);
	SDL_FreeSurface(temp);
}

void Ground::setList(std::vector < CollisionTile > list) {
	tileList = list;
}

Ground::Ground(Ground&& other) :
	multiPath(other.multiPath),
	tileIndices(nullptr),
	tileFlags(nullptr),
	PhysicsEntity(std::move(other)),
	flip(std::move(other.flip))
{
	tileIndices = other.tileIndices;
	tileFlags = other.tileFlags;
	other.tileIndices = nullptr;
	other.tileFlags = nullptr;
	std::cout << "Ground move constructor was called.";
};

Ground::Ground(const Ground& other) :
	multiPath(other.multiPath),
	tileIndices(nullptr),
	tileFlags(nullptr),
	flip(other.flip),
	PhysicsEntity(other)
{
	if (other.tileIndices != nullptr) {
		tileIndices = new int[256 * (other.multiPath + 1)];
		tileFlags = new int[256 * (other.multiPath + 1)];
		std::memmove(tileIndices, other.tileIndices, sizeof(int) * 256 * (multiPath + 1));
		std::memmove(tileFlags, other.tileFlags, sizeof(int) * 256 * (multiPath + 1));
	}
};

std::vector < CollisionTile >& Ground::getList() {
	return tileList;
}

Ground::Ground() :
	multiPath(false),
	tileIndices(nullptr),
	tileFlags(nullptr),
	flip(false)
{
	position = { -1, -1, -1, -1, -1 };
}

Ground::Ground(SDL_Point p, const std::vector < int >& indices, const std::vector < int >& flags, const std::vector < int >& collideIndices, const std::vector < int >& collideFlags, bool pFlip) :
	tileIndices(new int[collideIndices.size()]),
	tileFlags(new int[collideFlags.size()]),
	multiPath(collideIndices.size() / 256 - 1),
	flip(pFlip),
	PhysicsEntity({ p.x * 256, p.y * 256, 256, 256 }, window, std::max(collideIndices.size() / 256 - 1, indices.size() / 256 - 1))
{
	SDL_Rect current = { 0, 0, 16, 16 };
	SDL_Rect dest = { 0, 0, 16, 16 };
	int ind = 0;
	int numFrames = 16;

	SDL_Surface* flipHoriz = Animation::FlipSurface(map, SDL_FLIP_HORIZONTAL);
	SDL_Surface* flipVertical = Animation::FlipSurface(map, SDL_FLIP_VERTICAL);
	SDL_Surface* flipBoth = Animation::FlipSurface(flipHoriz, SDL_FLIP_VERTICAL);

	for (int i = 0; i < indices.size(); i++) {
		ind = indices[i];
		current.x = 16 * (ind % (map->w / 16));
		current.y = 16 * floor(double(ind) / (map->w / 16));
		SDL_RendererFlip currentFlip = SDL_RendererFlip(flags[i] ^ flip);
		if (currentFlip & SDL_FLIP_HORIZONTAL) {
			current.x = map->w - current.x - 16;
		}
		if (currentFlip & SDL_FLIP_VERTICAL) {
			current.y = map->h - current.y - 16;
		}
		dest.x = (i & 0xF) << 4;
		dest.y = i & (0xFF & (~0xF));
		if(flip)
			dest.x = 256 - 16 - dest.x;
		SDL_Surface* surfaceFlipped = nullptr;
		switch (currentFlip) {
		case SDL_FLIP_NONE:
			surfaceFlipped = map;
			break;
		case SDL_FLIP_HORIZONTAL:
			surfaceFlipped = flipHoriz;
			break;
		case SDL_FLIP_VERTICAL:
			surfaceFlipped = flipVertical;
			break;
		case SDL_FLIP_HORIZONTAL | SDL_FLIP_VERTICAL:
			surfaceFlipped = flipBoth;
			break;
		}
		//0 is rendered in front of the player
		//1 is rendered behind the player
		if (multiPath && indices.size() != 512) {
			int tilePathFront = collideIndices[i];
			int tilePathBack = collideIndices[i + 256];
			if (tilePathBack && !tilePathFront) {
				SDL_BlitSurface(surfaceFlipped, &current, animations[1]->GetSpriteSheet(), &dest);
				current.x = 0;
				current.y = 0;
				SDL_BlitSurface(map, &current, animations[0]->GetSpriteSheet(), &dest);
			}
			else {
				SDL_BlitSurface(surfaceFlipped, &current, animations[0]->GetSpriteSheet(), &dest); //src, srcrect, dst, dstrect
				current.x = 0;
				current.y = 0;
				SDL_BlitSurface(map, &current, animations[1]->GetSpriteSheet(), &dest);
			}
		}
		else if (indices.size() == 512) {
			SDL_BlitSurface(surfaceFlipped, &current, animations[i / 256]->GetSpriteSheet(), &dest);
		}
		else {
			SDL_BlitSurface(surfaceFlipped, &current, animations[0]->GetSpriteSheet(), &dest); //src, srcrect, dst, dstrect
		}
	}

	SDL_FreeSurface(flipHoriz);
	SDL_FreeSurface(flipVertical);
	SDL_FreeSurface(flipBoth);

	for (std::unique_ptr < Animation >& i : animations) {
		i->updateTexture();
	}

	std::memmove(tileIndices, &collideIndices[0], std::size_t(sizeof(int) * 256 * (multiPath + 1)));
	std::memmove(tileFlags, &collideFlags[0], std::size_t(sizeof(int) * 256 * (multiPath + 1)));
}

Ground::~Ground()
{
	delete[] tileIndices;
	delete[] tileFlags;
}

void Ground::Render(const SDL_Rect& camPos, const double& ratio, const SDL_Rect* position, const int layer, const bool flip) const {
	SDL_Rect pos = position ? *position : GetRelativePos(camPos);
	if (layer < animations.size()) {
		animations[layer]->Render(&pos, 0, globalObjects::window, NULL, 1.0 / ratio, SDL_RendererFlip(flip));
	}
}

CollisionTile& Ground::getTile(int tileX, int tileY) {
	if(!flip)
		return tileList[tileIndices[tileY * 16 + tileX]];
	return tileList[tileIndices[tileY * 16 + 15 - (tileX & 0xFF) + (tileX & 0x100)]];
}

int Ground::getFlag(int ind) {
	if (!flip)
		return tileFlags[ind] ^ flip;
	return tileFlags[(ind & 0x1F0) + 0xF - (ind & 0xF)] ^ flip;
}

bool Ground::getHeight(int x, int yMax, int yMin, int& height) {
	int tileX = floor((x - position.x) / 16.0);
	int tileY = floor((yMax - position.y) / 16.0);
	int h;
	height = 0;
	bool incremented = false;
	if (tileIndices[0] == -1) {
		height = yMin + 20;
		return false;
	}
	while (true) {
		if (tileList[tileIndices[tileY * 16 + tileX]].getCollide()) {
			if (tileFlags[tileY * 16 + tileX] & SDL_FLIP_HORIZONTAL) {
				h = getTile(tileX, tileY).getHeight(15 - ((x - position.x) % 16));
			}
			else {
				h = getTile(tileX, tileY).getHeight((x - position.x) % 16);
			}
			if (h < 16) {
				height = (tileY + 1) * 16 - h + position.y;
				return true;
			}
		}
		tileY--;
		if ((tileY + 1) * 16 < yMin) {
			height = yMin + 20;
			return false;
		}
	}
}

Ground& Ground::operator = (const Ground& arg) {
	if (this == &arg)
		return *this;
	multiPath = arg.multiPath;
	delete[] tileIndices;
	delete[] tileFlags;
	tileIndices = new int[256 * (multiPath + 1)];
	tileFlags = new int[256 * (multiPath + 1)];
	std::memmove(tileIndices, arg.tileIndices, sizeof(int) * 256 * (multiPath + 1));
	std::memmove(tileFlags, arg.tileFlags, sizeof(int) * 256 * (multiPath + 1));
	invis = false;
	window = arg.window;
	for (int i = 0; i < arg.animations.size(); i++) {
		animations.emplace_back(new Animation(*arg.animations[i]));
	}
	position = arg.position;
	previousPosition = arg.previousPosition;
	loaded = arg.loaded;
	num = arg.num;
	time = SDL_GetTicks();
	last_time = time;
	currentAnim = arg.currentAnim;
	canCollide = true;
	gravity = arg.gravity;
	customVars = arg.customVars;
	flip = arg.flip;
}

Ground& Ground::operator = (Ground&& arg) {
	if (this == &arg)
		return *this;
	multiPath = std::move(arg.multiPath);
	delete[] tileIndices;
	delete[] tileFlags;
	tileIndices = arg.tileIndices;
	tileFlags = arg.tileFlags;
	arg.tileIndices = nullptr;
	arg.tileFlags = nullptr;
	position = std::move(arg.position);
	previousPosition = std::move(arg.previousPosition);
	loaded = std::move(arg.loaded);
	num = std::move(arg.num);
	time = SDL_GetTicks();
	last_time = time;
	currentAnim = std::move(arg.currentAnim);
	canCollide = true;
	gravity = std::move(arg.gravity);
	customVars = std::move(arg.customVars);
	flip = std::move(arg.flip);
	animations = std::move(arg.animations);
}

double Ground::getTileAngle(int tileX, int tileY) {
	CollisionTile& tile = getTile(tileX, tileY);
	switch (getFlag(tileY * 16 + tileX)) {
	case SDL_FLIP_NONE:
		return tile.getAngle();
	case SDL_FLIP_HORIZONTAL:
		return 0x100 - tile.getAngle();
	case SDL_FLIP_VERTICAL:
		return (0x180 - tile.getAngle()) % 0x100;
	case (SDL_FLIP_HORIZONTAL | SDL_FLIP_VERTICAL):
		return (0x080 + tile.getAngle()) % 0x100;
	default:
		throw "Invalid flag";
		return -1;
	}
};

void Ground::setIndices(std::vector < int > ind) {
	std::memmove(tileIndices, &ind[0], sizeof(int) * 256);
}