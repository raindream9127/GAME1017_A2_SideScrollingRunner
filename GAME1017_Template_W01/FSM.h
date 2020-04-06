#pragma once
#include <vector>
#include <SDL_image.h>
#include "TextElement.h"
#include "Button.h"
#include "Box.h"
using namespace std;

class State // This is the abstract base class for all specific states.
{
protected:
	State() {}
public:
	virtual void Enter() = 0;
	virtual void HandleEvents(SDL_Event& event);
	virtual void Update() = 0; 
	virtual void Render() = 0;
	virtual void Exit() = 0;
	virtual void Resume(); 
};

class PauseState : public State
{
private:
	vector<Button*> m_vButtons;
public:
	PauseState();
	void Enter();
	void HandleEvents(SDL_Event& event);
	void Update();
	void Render();
	void Exit(); 
};

class GameState : public State
{
private:
	Player* m_pPlayer;
	Mix_Music* m_pMusic;
	vector<Mix_Chunk*> m_vSounds;
	vector<Sprite*> m_pPlatforms;
	vector<Sprite*> m_pObstacles;
	Background* m_pBackground[2];
	MidBackground* m_pMidBackground[5];
	TextElement* t, * t2;
	Timer* ti;
	SDL_Texture* m_pBackgroundText;
	SDL_Texture* m_pObsText;
	SDL_Texture* m_pPlayerText;
	unsigned int m_distance;
	int m_bestRecord;
	bool m_bSpaceOk;
public:
	GameState();
	void Enter();
	void HandleEvents(SDL_Event& event);
	void Update();
	void Render();
	void Exit();
	void Resume();
	void CheckCollision();
};

class LoseState : public State
{
private:
	vector<Button*> m_vButtons;
	SDL_Texture* m_pBGTexture;
public:
	LoseState();
	void Enter();
	void HandleEvents(SDL_Event& event);
	void Update();
	void Render();
	void Exit();
};

class TitleState : public State
{
private:
	vector<Button*> m_vButtons;
	SDL_Texture* m_pBGTexture;
public:
	TitleState();
	void Enter();
	void HandleEvents(SDL_Event& event);
	void Update();
	void Render();
	void Exit();
};

class FSM
{
private:
	vector<State*> m_vStates;
public:
	void Update();
	void HandleEvents();
	void Render();
	void ChangeState(State* pState);
	void PushState(State* pState);
	void PopState();
	void Clean();
	vector<State*>& GetStates();
};