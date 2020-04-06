#include "Box.h"
#include "Engine.h"
#include "SDL.h"
#include <algorithm>
#define SCROLLSPD 3
using namespace std;

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

SDL_Rect* Sprite::GetCollisionRect()
{
	return &m_rDst;
}

void Sprite::Render(SDL_Texture* tex)
{
	SDL_RenderCopy(Engine::Instance().GetRenderer(), tex, &m_rSrc, &m_rDst);
}

void Sprite::Update()
{
	m_rDst.x -= SCROLLSPD;
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

Player::Player(const SDL_Rect s, const SDL_Rect d) : Sprite(s, d),
	m_collisionRect({ d.x + d.w / 4, d.y, d.w / 2, d.h})
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

SDL_Rect* Player::GetCollisionRect()
{
	return &m_collisionRect;
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
	// Sets up the collision rectangle
	if (GetAnimState() != ROLLING)
	{
		m_collisionRect = { m_rDst.x + 32, m_rDst.y, 64, 128 };
	}
	else
	{
		m_collisionRect = { m_rDst.x + 32, m_rDst.y + 73, 55, 55 };
	}	
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
			if (GetAnimState() != DYING)
			{
				m_iSprite = m_iSpriteMin;
			}
			else
			{
				SDL_Delay(2000);
				Engine::Instance().GetFSM().ChangeState(new LoseState);
				return;
			}
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
	this->SetAnimationState(DYING, m_iAnimY + 128, 6, 4, 9);
}

Platform::Platform(const SDL_Rect d) : Sprite({ 1024, 512, 512, 256 }, d)
{}

MidBackground::MidBackground(const SDL_Rect d) : Sprite({ 1024, 0, 256, 512 }, d)
{}

void MidBackground::Update()
{
	m_rDst.x -= 2;
}

Background::Background(const SDL_Rect d) : Sprite({ 0, 0, 1024, 768 }, d)
{}

void Background::Update()
{
	m_rDst.x -= 1;
}

Spikes::Spikes() : Sprite({ 128, 64, 128, 64 }, {1024, 448, 128, 64})
{}

Spikewall::Spikewall() : Sprite({ 0, 0, 128, 448 }, {1024, 0, 128, 448})
{}

Circularsaw::Circularsaw() : Sprite({ 128, 128, 128, 128}, {1024, 448, 128, 128}), m_angle(0)
{}

void Circularsaw::Render(SDL_Texture * tex)
{
	SDL_RenderCopyEx(Engine::Instance().GetRenderer(), tex, &m_rSrc, &m_rDst, m_angle, NULL, SDL_FLIP_NONE);
}

void Circularsaw::Update()
{
	m_rDst.x -= SCROLLSPD;
	m_angle += 3;
	if (m_angle == 360)
	{
		m_angle = 0;
	}
}

Flyingplatform::Flyingplatform() : Sprite({ 128, 0, 128, 40}, { 1024, 256, 128, 40})
{}
