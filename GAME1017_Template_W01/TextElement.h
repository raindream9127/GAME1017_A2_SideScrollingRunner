#pragma once
#include <string>
#include "SDL.h"
#include "SDL_ttf.h"
using namespace std;

class TextElement
{
private:
	TTF_Font* m_Font;
	SDL_Color m_TextColor;
	SDL_Rect m_rTextRect;
	SDL_Texture* m_pTexture;
	char m_String[256];
public:
	TextElement(const int x, const int y, const char* str, 
		const int sz, const SDL_Color col = { 255, 255, 255, 255 });
	~TextElement();
	void Render();
	void SetText(const char * str);
	void SetPos(const int x, const int y);
	void SetSize(const int sz);
	void SetColor(const SDL_Color& col);
};

class Timer
{
private:
	bool m_started, m_running, m_change;
	int m_elapsed, m_elapsedTotal;
	time_t m_startTime, m_lastTime, m_currTime;
public:
	Timer();
	void Start();
	void Pause();
	void Resume();
	void Update();
	string GetTime();
	bool HasChanged();
};

