#pragma once
#include "SDL.h"

enum STATE
{
	RUNNING,
	JUMPING,
	ROLLING,
	DYING
};

class Sprite
{
protected:
	SDL_Rect m_rSrc, m_rDst;
public:
	Sprite(const SDL_Rect s, const SDL_Rect d);
	SDL_Rect* GetSrcP();
	SDL_Rect* GetDstP();
	virtual SDL_Rect* GetCollisionRect();
	virtual void Render(SDL_Texture* tex);
	virtual void Update();
};

class Spikes : public Sprite
{
public:
	Spikes();
};

class Spikewall : public Sprite
{
public:
	Spikewall();
};

class Circularsaw : public Sprite
{
private:
	int m_angle;
public:
	Circularsaw();
	void Render(SDL_Texture* tex) override;
	void Update() override;
};

class Flyingplatform : public Sprite
{
public:
	Flyingplatform();
};

class Background : public Sprite
{
public:
	Background(const SDL_Rect d);
	void Update();
};

class MidBackground : public Sprite
{
public:
	MidBackground(const SDL_Rect d);
	void Update();
};

class Platform : public Sprite
{
public:
	Platform(const SDL_Rect d);
};

class Player : public Sprite
{
private:
	SDL_Rect m_collisionRect;
	bool m_bGrounded;
	double m_dAccelX,
		m_dMaxAccelX,
		m_dAccelY,
		m_dVelX,
		m_dMaxVelX,
		m_dVelY,
		m_dMaxVelY,
		m_dDrag,
		m_dGrav,
		m_dXForce;
	STATE m_state;
	int m_iDir, // Direction. -1 or 1. Not being used in this example.
		m_iSprite,
		m_iSpriteMin,
		m_iSpriteMax,
		m_iFrame,
		m_iFrameMax,
		m_iAnimY;
	void SetAnimationState(STATE st, int y, int fmax, int smin, int smax);
public:
	Player(const SDL_Rect s, const SDL_Rect d);
	SDL_Rect* GetCollisionRect() override;
	void Update() override;
	void Animate();
	void MoveX();
	void Stop();
	void SetDir(int dir);
	void SetAccelX(double a);
	void SetAccelY(double a);
	bool IsGrounded();
	void SetGrounded(bool g);
	double GetVelX();
	double GetVelY();
	void SetVelX(double v);
	void SetVelY(double v);
	void SetX(int y);
	void SetY(int y);
	const STATE GetAnimState();
	void SetJumping();
	void SetRunning();
	void SetRolling();
	void SetDeath();
};
