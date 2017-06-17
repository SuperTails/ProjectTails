#include "stdafx.h"
#include "Ground.h"

SDL_Surface* Ground::map;
std::vector < CollisionTile > Ground::tileList;
std::string Ground::mPath;

void Ground::setMap(std::string mapPath) {
	SDL_FreeSurface(map);
	mPath = mapPath;
	SDL_Surface* temp = IMG_Load(mapPath.c_str());
	map = SDL_ConvertSurface(temp, SDL_GetWindowSurface(globalObjects::window)->format, SDL_RLEACCEL);
	SDL_FreeSurface(temp);
}

void Ground::setList(std::vector < CollisionTile > list) {
	tileList = list;
}

Ground::Ground(const Ground& other) :
	multiPath(other.multiPath),
	tileIndices(nullptr),
	tileFlags(nullptr),
	flip(other.flip),
	PhysicsEntity(other)
{
	if (other.tileIndices != nullptr) {
		tileIndices = new (std::nothrow) int[GROUND_SIZE * (other.multiPath + 1)];
		tileFlags = new (std::nothrow) int[GROUND_SIZE * (other.multiPath + 1)];
		if (tileIndices == nullptr || tileFlags == nullptr) {
			throw "Could not allocate memory for ground array.";
		}
		std::memmove(tileIndices, other.tileIndices, GROUND_SIZE * sizeof(int) * (multiPath + 1));
		std::memmove(tileFlags, other.tileFlags, GROUND_SIZE * sizeof(int) * (multiPath + 1));
	}
};

Ground::Ground(Ground&& other) noexcept :
	Ground()
{
	swap(*this, other);
#if _DEBUG
	std::cout << "Ground move constructor was called.\n";
#endif
};

Ground::Ground() :
	multiPath(false),
	tileIndices(nullptr),
	tileFlags(nullptr),
	flip(false)
{
	position = { -1, -1, -1, -1, -1 };
}

Ground::Ground(SDL_Point p, const groundArrayData& arrayData, bool pFlip) :
	tileIndices(new int[arrayData.collideIndices.size()]),
	tileFlags(new int[arrayData.collideFlags.size()]),
	multiPath(arrayData.collideIndices.size() / GROUND_SIZE - 1),
	flip(pFlip),
	PhysicsEntity({ int(p.x * GROUND_PIXEL_WIDTH), int(p.y * GROUND_PIXEL_WIDTH), int(GROUND_PIXEL_WIDTH), int(GROUND_PIXEL_WIDTH) }, std::max(arrayData.collideIndices.size() / GROUND_SIZE - 1, arrayData.graphicsIndices.size() / GROUND_SIZE - 1))
{
	SDL_Rect current = { 0, 0, TILE_WIDTH, TILE_WIDTH };
	SDL_Rect dest = { 0, 0, TILE_WIDTH, TILE_WIDTH };
	int ind = 0;

	SDL_Surface* flipHoriz = Animation::FlipSurface(map, SDL_FLIP_HORIZONTAL);
	SDL_Surface* flipVertical = Animation::FlipSurface(map, SDL_FLIP_VERTICAL);
	SDL_Surface* flipBoth = Animation::FlipSurface(flipHoriz, SDL_FLIP_VERTICAL);

	for (int i = 0; i < arrayData.graphicsIndices.size(); i++) {
		ind = arrayData.graphicsIndices[i];
		current.x = TILE_WIDTH * (ind % (map->w / TILE_WIDTH));
		current.y = TILE_WIDTH * floor(double(ind) / (map->w / TILE_WIDTH));
		SDL_RendererFlip currentFlip = SDL_RendererFlip(arrayData.graphicsFlags[i] ^ flip);
		if (currentFlip & SDL_FLIP_HORIZONTAL) {
			current.x = map->w - current.x - TILE_WIDTH;
		}
		if (currentFlip & SDL_FLIP_VERTICAL) {
			current.y = map->h - current.y - TILE_WIDTH;
		}
		dest.x = (i % GROUND_WIDTH) * TILE_WIDTH;
		dest.y = TILE_WIDTH * ((i % GROUND_SIZE) / GROUND_WIDTH);
		if (flip)
			dest.x = (GROUND_WIDTH * TILE_WIDTH) - TILE_WIDTH - dest.x;
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
		if (multiPath && arrayData.graphicsIndices.size() != GROUND_SIZE * 2) {
			int tilePathFront = arrayData.collideIndices[i];
			int tilePathBack = arrayData.collideIndices[i + GROUND_SIZE];
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
		else if (arrayData.graphicsIndices.size() == GROUND_SIZE * 2) {
			SDL_BlitSurface(surfaceFlipped, &current, animations[i / GROUND_SIZE]->GetSpriteSheet(), &dest);
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

	std::memmove(tileIndices, &arrayData.collideIndices[0], GROUND_SIZE * std::size_t(sizeof(int) * (multiPath + 1)));
	std::memmove(tileFlags, &arrayData.collideFlags[0], GROUND_SIZE * std::size_t(sizeof(int) * (multiPath + 1)));
}

Ground::~Ground()
{
	delete[] tileIndices;
	delete[] tileFlags;
}

void Ground::Render(const SDL_Rect& camPos, const double& ratio, const SDL_Rect* position, int layer, bool flip) const {
	SDL_Rect pos = (position != nullptr) ? *position : GetRelativePos(camPos);
	if (layer < animations.size()) {
		animations[layer]->Render(&pos, 0, NULL, 1.0 / ratio, SDL_RendererFlip(flip));
	}
}

CollisionTile& Ground::getTile(int tileX, int tileY, bool path) const {
	path &= multiPath;

	if(!flip)
		return tileList[tileIndices[tileX + tileY * GROUND_WIDTH + path * GROUND_SIZE]];
	
	return tileList[tileIndices[tileY * GROUND_WIDTH + (GROUND_WIDTH - 1 - tileX) + GROUND_SIZE * path]];
}

int Ground::getFlag(int tileX, int tileY, bool path) const {
	path &= multiPath;

	if (!flip)
		return tileFlags[tileX + tileY * GROUND_WIDTH + path * GROUND_SIZE];

	return tileFlags[tileY * GROUND_WIDTH + (GROUND_WIDTH - 1 - tileX) + GROUND_SIZE * path] ^ SDL_FLIP_HORIZONTAL;
}

Ground& Ground::operator= (Ground arg) {
	using std::swap;

	swap(*this, arg);

	return *this;
}

double Ground::getTileAngle(int tileX, int tileY, bool path) const {
	CollisionTile& tile = getTile(tileX, tileY, path);
	int flag = getFlag(tileX, tileY, path) & (SDL_FLIP_NONE | SDL_FLIP_HORIZONTAL | SDL_FLIP_VERTICAL);
	switch (flag) {
	case SDL_FLIP_NONE:
		return tile.getAngle();
	case SDL_FLIP_HORIZONTAL:
		return 0x100 - tile.getAngle();
	case SDL_FLIP_VERTICAL:
		return (0x180 - tile.getAngle()) % 0x100;
	case (SDL_FLIP_HORIZONTAL | SDL_FLIP_VERTICAL):
		return (0x080 + tile.getAngle()) % 0x100;
	default:
		std::cerr << "Invalid flag: " << flag << "\n";
		throw "Invalid flag";
		return -1;
	}
};

void swap(Ground& lhs, Ground& rhs) noexcept {
	using std::swap;

	swap(static_cast<PhysicsEntity&>(lhs), static_cast<PhysicsEntity&>(rhs));

	swap(lhs.tileIndices, rhs.tileIndices);
	swap(lhs.tileFlags, rhs.tileFlags);
	swap(lhs.multiPath, rhs.multiPath);
	swap(lhs.flip, rhs.flip);
}