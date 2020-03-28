#include "Box.h"
#include "Engine.h"
#include "SDL.h"
#define SCROLLSPD 2

Sprite::Sprite(SDL_Rect s, SDL_Rect d, SDL_Texture* texture) : m_src(s), m_dst(d) , m_pTexture(texture) {}

void Sprite::Render()
{
	SDL_RenderCopy(Engine::Instance().GetRenderer(), m_pTexture, &m_src, &m_dst);
}

Box::Box(SDL_Rect s, SDL_Rect d, SDL_Texture* texture, bool hasSprite) : m_x(d.x), m_sprite(nullptr)
{
	if (hasSprite)
	{
		m_sprite = new Sprite(s, d, texture);
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
		SDL_Rect temp = { m_x, 384, 128, 128 };
		SDL_RenderDrawRect(Engine::Instance().GetRenderer(), &temp);
	}
}

int Box::GetX()
{
	return m_x;
}
