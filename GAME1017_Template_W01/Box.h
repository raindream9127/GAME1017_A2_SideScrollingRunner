#pragma once
#include "SDL.h"

class Sprite
{
private:
	SDL_Rect m_dst;
	SDL_Color m_col;
public:
	Sprite(int x, int y);
	void Render();
	friend class Box;
};

class Box
{
private:
	int m_x, m_y;
	Sprite* m_sprite;
public:
	Box(int x, int y, bool hasSprite = false);
	~Box();
	void Update();
	void Render();
	int GetX();
};