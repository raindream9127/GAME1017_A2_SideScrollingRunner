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

class Player : public Sprite
{
private:
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
	void Update();
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

// Platform doesn't have much in it right now. 
class Platform : public Sprite
{
public:
	Platform(const SDL_Rect d) :Sprite({ 0,0,0,0 }, d) {}
};