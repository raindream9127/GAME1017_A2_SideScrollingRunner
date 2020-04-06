#include "Engine.h"
#include "TextElement.h"
#include <cstring>
#include <ctime>
using namespace std;

TextElement::TextElement(const int x, const int y, const char* str, const int sz, 
	const SDL_Color col): m_TextColor(col)
{
	strcpy_s(m_String, 64, str);
	this->SetPos(x, y);
	this->SetSize(sz);

}

TextElement::~TextElement()
{
	TTF_CloseFont(m_Font);
	SDL_DestroyTexture(m_pTexture);
}

void TextElement::Render()
{
	SDL_RenderCopy(Engine::Instance().GetRenderer(), m_pTexture, 0, &m_rTextRect);
}

void TextElement::SetText(const char * str)
{
	strcpy_s(m_String, 64, str);
	SDL_Surface* fontSurf = TTF_RenderText_Solid(m_Font, m_String, m_TextColor);
	SDL_DestroyTexture(m_pTexture);
	m_pTexture = SDL_CreateTextureFromSurface(Engine::Instance().GetRenderer(), fontSurf);
	m_rTextRect = { m_rTextRect.x, m_rTextRect.y, fontSurf->w, fontSurf->h };
	SDL_FreeSurface(fontSurf);
}

void TextElement::SetPos(const int x, const int y)
{
	m_rTextRect = { x, y, m_rTextRect.w, m_rTextRect.h };
}

void TextElement::SetSize(const int sz)
{
	if (m_Font != nullptr)
	{
		TTF_CloseFont(m_Font);
	}
	m_Font = TTF_OpenFont("Img/LTYPE.TTF", sz);
	this->SetText(m_String);
}

void TextElement::SetColor(const SDL_Color& col)
{
	m_TextColor = col;
	this->SetText(m_String);
}

Timer::Timer()
{
	m_started = m_running = m_change = false;
	m_elapsed = m_elapsedTotal = 0;
}

void Timer::Start()
{
	if (!m_started)
	{
		m_started = m_running = true;
		m_startTime = m_lastTime = m_currTime = time(NULL);
	}
	else if (m_started && !m_running)
	{
		this->Resume();
	}
}

void Timer::Pause()
{
	m_running = m_change = false;
	m_elapsedTotal += m_elapsed;
	m_elapsed = 0;
}

void Timer::Resume()
{
	m_startTime = time(NULL);
	m_running = true;
}

void Timer::Update()
{
	if (m_running)
	{
		m_lastTime = m_currTime;
		m_currTime = time(NULL);
		if (m_lastTime != m_currTime)
		{
			m_change = true;
		}
		else
		{
			m_change = false;
		}
		m_elapsed = (int)m_currTime - (int)m_startTime;
	}
}

string Timer::GetTime()
{
	string s = to_string(m_elapsedTotal + m_elapsed);
	return s;
}

bool Timer::HasChanged()
{
	return m_change;
}
