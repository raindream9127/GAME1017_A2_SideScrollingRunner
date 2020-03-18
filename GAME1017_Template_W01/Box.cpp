#include "Box.h"
#include "Engine.h"
#include "SDL.h"
#define SCROLLSPD 2

Sprite::Sprite(int x, int y) : m_dst({ x, y, 128, 128 })
{
	m_col = { (Uint8)(rand()%256), (Uint8)(rand()%256), (Uint8)(rand()%256), (Uint8)255 };
}

void Sprite::Render()
{
	SDL_SetRenderDrawColor(Engine::Instance().GetRenderer(), m_col.r, m_col.g, m_col.b, m_col.a);
	SDL_RenderFillRect(Engine::Instance().GetRenderer(), &m_dst);
}

Box::Box(int x, int y, bool hasSprite) : m_x(x), m_y(y), m_sprite(nullptr)
{
	if (hasSprite)
	{
		m_sprite = new Sprite(x, y);
	}
}

Box::~Box()
{
	if (m_sprite != nullptr)
	{
		delete m_sprite;
		m_sprite = nullptr; // Optional.
	}
}

void Box::Update()
{
	m_x -= SCROLLSPD;
	if (m_sprite != nullptr)
	{
		m_sprite->m_dst.x = m_x;
	}
}

void Box::Render()
{
	if (m_sprite != nullptr)
		m_sprite->Render();
	// Comment the below code out if you don't want to see a box for an empty sprite.
	else
	{
		SDL_SetRenderDrawColor(Engine::Instance().GetRenderer(), 255, 0, 255, 255);
		SDL_Rect temp = { m_x, m_y, 128, 128 };
		SDL_RenderDrawRect(Engine::Instance().GetRenderer(), &temp);
	}
}

int Box::GetX()
{
	return m_x;
}
