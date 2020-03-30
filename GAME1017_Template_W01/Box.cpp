#include "Box.h"
#include "Engine.h"
#include "SDL.h"
#define SCROLLSPD 2

Sprite::Sprite(const SDL_Rect s, const SDL_Rect d) : m_rSrc(s), m_rDst(d) 
{}

SDL_Rect* Sprite::GetSrcP()
{
	return &m_rSrc;
}

SDL_Rect* Sprite::GetDstP()
{
	return &m_rDst;
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
		m_sprite->m_rDst.x = m_x;
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

void Player::SetAnimationState(STATE st, int y, int fmax, int smin, int smax)
{
	m_state = st;
	m_rSrc.y = y;
	m_iFrame = 0;
	m_iFrameMax = fmax;
	m_iSpriteMin = smin;
	m_iSpriteMax = smax;
	m_iSprite = m_iSpriteMin;
}

Player::Player(const SDL_Rect s, const SDL_Rect d) :Sprite(s, d)
{
	m_bGrounded = false;
	m_dAccelX = m_dAccelY = m_dVelX = m_dVelY = 0.0;
	m_dGrav = GRAV;
	m_dMaxAccelX = 3.0;
	m_dMaxVelX = 8.0;
	m_dMaxVelY = m_dGrav * 2;
	m_dDrag = 0.9;
	m_dXForce = 0.1;
	m_iDir = 1;
	m_iAnimY = m_rSrc.y;
	SetRunning();
}

void Player::Update()
{
	// X axis. Clamp acceleration first.
	m_dAccelX = min(max(m_dAccelX, -(m_dMaxAccelX)), (m_dMaxAccelX));
	// Set and clamp X velocity.
	m_dVelX = (m_dVelX + m_dAccelX) * m_dDrag;
	m_dVelX = min(max(m_dVelX, -(m_dMaxVelX)), (m_dMaxVelX));
	m_rDst.x += (int)m_dVelX;
	// Now do Y axis.
	m_dVelY += m_dAccelY + (m_dGrav / 5); // Adjust gravity to get slower jump.
	m_dVelY = min(max(m_dVelY, -(m_dMaxVelY * 10)), (m_dMaxVelY));
	m_rDst.y += (int)m_dVelY;
	if (fabs(m_dVelY) > m_dGrav / 4)
	{
		SetJumping();
	}
	this->Animate();
}

void Player::Animate()
{
	if (m_iFrame++ == m_iFrameMax)
	{
		m_iFrame = 0;
		m_iSprite++;
		if (m_iSprite == m_iSpriteMax)
		{
			m_iSprite = m_iSpriteMin;
		}
	}
	m_rSrc.x = m_rSrc.w * m_iSprite;
}

void Player::MoveX() { m_dAccelX += m_dXForce * m_iDir; } // Using a x force variable * direction.

void Player::Stop() // If you want a dead stop both axes.
{
	m_dVelY = 0.0;
	m_dVelX = 0.0;
}

void Player::SetDir(int dir) { m_iDir = dir; } // Will be used to set direction of sprite for your Assignment 2.
void Player::SetAccelX(double a) { m_dAccelX = a; }
void Player::SetAccelY(double a) { m_dAccelY = a; }
bool Player::IsGrounded() { return m_bGrounded && (m_dVelY == 0); }
void Player::SetGrounded(bool g) { m_bGrounded = g; }
double Player::GetVelX() { return m_dVelX; }
double Player::GetVelY() { return m_dVelY; }
void Player::SetVelX(double v) { m_dVelX = v; }
void Player::SetVelY(double v) { m_dVelY = v; }
void Player::SetX(int y) { m_rDst.x = y; }
void Player::SetY(int y) { m_rDst.y = y; }

const STATE Player::GetAnimState()
{
	return m_state;
}

void Player::SetJumping()
{
	this->SetAnimationState(JUMPING, m_iAnimY, 1, 8, 9);
}

void Player::SetRunning()
{
	this->SetAnimationState(RUNNING, m_iAnimY, 6, 0, 8);
}

void Player::SetRolling()
{
	this->SetAnimationState(ROLLING, m_iAnimY + 128, 6, 0, 4);
}

void Player::SetDeath()
{
}
