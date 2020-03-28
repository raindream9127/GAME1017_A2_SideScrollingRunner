#pragma once
#include "SDL.h"

class Sprite
{
private:
	SDL_Rect m_src, m_dst;
	SDL_Texture* m_pTexture;
public:
	Sprite(SDL_Rect s, SDL_Rect d, SDL_Texture* texture);
	void Render();
	friend class Box;
};

class Box
{
private:
	int m_x;
	Sprite* m_sprite;
public:
	Box(SDL_Rect s, SDL_Rect d, SDL_Texture* texture, bool hasSprite = false);
	~Box();
	void Update();
	void Render();
	int GetX();
};