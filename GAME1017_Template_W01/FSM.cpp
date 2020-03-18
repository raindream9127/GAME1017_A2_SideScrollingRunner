#include <iostream>
#include <SDL.h>
#include <SDL_mixer.h>
#include "FSM.h"
#include "Engine.h"
#include "Utilities.h"
using namespace std;

void State::HandleEvents(SDL_Event& event)
{
	switch (event.type)
	{
	case SDL_QUIT: // User pressed window's 'x' button.
		Engine::Instance().QuitGame();
		break;
	case SDL_KEYDOWN: // Try SDL_KEYUP instead.
		if (event.key.keysym.sym == SDLK_ESCAPE)
			Engine::Instance().QuitGame();
		break;
	case SDL_MOUSEBUTTONDOWN:
		if (event.button.button >= 1 && event.button.button <= 3)
			Engine::Instance().SetMouseState(event.button.button - 1, true);
		break;
	case SDL_MOUSEBUTTONUP:
		if (event.button.button >= 1 && event.button.button <= 3)
			Engine::Instance().SetMouseState(event.button.button - 1, false);
		break;
	case SDL_MOUSEMOTION:
		SDL_GetMouseState(&Engine::Instance().GetMousePos().x, &Engine::Instance().GetMousePos().y);
		break;
	default:
		break;
	}
}

// Begin State. CTRL+M+H and CTRL+M+U to turn on/off collapsed code.
void State::Render()
{
	SDL_RenderPresent(Engine::Instance().GetRenderer());
}
void State::Resume() {}
// End State.

#pragma region Begin PauseState.
PauseState::PauseState() {}

void PauseState::Enter()
{
	cout << "Entering Pause..." << endl;
	m_vButtons.push_back(new ResumeButton("Img/button_resume.png", { 0,0,94,38 }, { 462,300,100,40 }));
	// This exit button has a different size but SAME function as the one in title.
	m_vButtons.push_back(new ExitButton("Img/button_exit.png", { 0,0,94,38 }, { 462,400,100,40 }));
	Mix_HaltChannel(-1);
}

void PauseState::HandleEvents(SDL_Event& event)
{
	switch (event.type)
	{
	case SDL_KEYDOWN: // Try SDL_KEYUP instead.
		if (event.key.keysym.sym == SDLK_r)
		{
			Engine::Instance().GetFSM().PopState();
		}
		break;
	default:
		State::HandleEvents(event);
		break;
	}	
}

void PauseState::Update()
{
	for (int i = 0; i < (int)m_vButtons.size(); i++)
		m_vButtons[i]->Update();
}

void PauseState::Render()
{
	cout << "Rendering Pause..." << endl;
	Engine::Instance().GetFSM().GetStates().front()->Render();
	SDL_SetRenderDrawBlendMode(Engine::Instance().GetRenderer(), SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(Engine::Instance().GetRenderer(), 0, 0, 255, 128);
	SDL_Rect rect = { 256, 128, 512, 512 };
	SDL_RenderFillRect(Engine::Instance().GetRenderer(), &rect);
	for (int i = 0; i < (int)m_vButtons.size(); i++)
		m_vButtons[i]->Render();
	State::Render();
}

void PauseState::Exit()
{
	cout << "Exiting Pause..." << endl;
	for (int i = 0; i < (int)m_vButtons.size(); i++)
	{
		delete m_vButtons[i];
		m_vButtons[i] = nullptr;
	}
	m_vButtons.clear();
	m_vButtons.shrink_to_fit();
}
#pragma endregion

#pragma region Begin GameState.
GameState::GameState() {}

void GameState::Enter()
{ 
	m_vec.reserve(8);
	// Create the vector now.
	for (int i = 0; i < 9; i++)
	{
		vector<Box*> temp;
		temp.push_back(new Box(128 * i, 384)); // Create empty box.
		m_vec.push_back(temp);
	}
}

void GameState::HandleEvents(SDL_Event& event)
{
	switch (event.type)
	{
	case SDL_QUIT: // User pressed window's 'x' button.
		Engine::Instance().QuitGame();
		break;
	case SDL_KEYDOWN: // Try SDL_KEYUP instead.
		if (event.key.keysym.sym == SDLK_ESCAPE)
			Engine::Instance().QuitGame();
		break;
	}
}

void GameState::Update()
{
	// Check for out of bounds.
	if ((m_vec[0])[0]->GetX() <= -128) // Fully off-screen.
	{
		// Clean the column vector.
		for (int i = 0; i < (int)m_vec[0].size(); i++)
		{
			delete m_vec[0][i];
			m_vec[0][i] = nullptr;
		}
		m_vec.erase(m_vec.begin()); // Pop the front element.
		// Create a new column vector.
		vector<Box*> temp;
		int numrows = 1 + rand() % 4; // 1-4 Boxes per column!
		for (int row = 0; row < numrows; row++)
			temp.push_back(new Box(128 * 8, 384 - 128 * row, rand() % 2)); // Create empty box.
		m_vec.push_back(temp);
	}
	// Scroll the boxes.
	for (int col = 0; col < 9; col++)
	{
		for (int row = 0; row < (int)m_vec[col].size(); row++)
		{
			(m_vec[col])[row]->Update(); // If the parenthesis make it easier to understand.
		}
	}
}

void GameState::Render()
{
	SDL_SetRenderDrawColor(Engine::Instance().GetRenderer(), 0, 0, 0, 255);
	SDL_RenderClear(Engine::Instance().GetRenderer()); // Clear the screen with the draw color.
	// Render stuff.
	for (int col = 0; col < 9; col++)
	{
		for (int row = 0; row < (int)m_vec[col].size(); row++)
		{
			(m_vec[col])[row]->Render();
		}
	}
	// Draw anew.
	SDL_RenderPresent(Engine::Instance().GetRenderer());
}

void GameState::Exit()
{ 
	for (int col = 0; col < 9; col++)
	{
		for (int row = 0; row < (int)m_vec[col].size(); row++)
		{
			delete m_vec[col][row]; // vector.erase() won't deallocate memory through pointer.
			m_vec[col][row] = nullptr; // Optional again, but good practice.
		}
		m_vec[col].clear();
	}
	m_vec.clear();
	m_vec.shrink_to_fit();
}

void GameState::Resume() { cout << "Resuming Game..." << endl; }

void GameState::CheckCollision()
{

}
#pragma endregion

#pragma region Begin TitleState.
TitleState::TitleState() {}

void TitleState::Enter()
{ 
	cout << "Entering Title..." << endl;
	m_pBGTexture = IMG_LoadTexture(Engine::Instance().GetRenderer(), "Img/title_bg.jpg");
	m_vButtons.push_back(new PlayButton("Img/button_play.png", { 0,0,94,38 }, { 462,400,100,40 }));
	// For the bind: what function, what instance, any parameters.
	m_vButtons.push_back(new ExitButton("Img/button_exit.png", { 0,0,94,38 }, { 462,500,100,40 }));
}

void TitleState::HandleEvents(SDL_Event& event)
{
	State::HandleEvents(event); // Don't need any events specific to TitleState.
}

void TitleState::Update()
{
	for (int i = 0; i < (int)m_vButtons.size(); i++)
		m_vButtons[i]->Update();
}

void TitleState::Render()
{
	cout << "Rendering Title..." << endl;
	SDL_SetRenderDrawColor(Engine::Instance().GetRenderer(), 255, 0, 0, 255);
	SDL_RenderClear(Engine::Instance().GetRenderer());
	SDL_Rect src = { 0,0,1024,768 }, dst = src;
	SDL_RenderCopy(Engine::Instance().GetRenderer(), m_pBGTexture, &src, &dst);
	for (int i = 0; i < (int)m_vButtons.size(); i++)
		m_vButtons[i]->Render();
	State::Render();
}

void TitleState::Exit()
{ 
	cout << "Exiting Title..." << endl;
	SDL_DestroyTexture(m_pBGTexture);
	for (int i = 0; i < (int)m_vButtons.size(); i++)
	{
		delete m_vButtons[i];
		m_vButtons[i] = nullptr;
	}
	m_vButtons.clear();
	m_vButtons.shrink_to_fit();
}
#pragma endregion

#pragma region Begin LoseState
LoseState::LoseState() {}

void LoseState::Enter()
{
	cout << "Entering Lose..." << endl;
	m_pBGTexture = IMG_LoadTexture(Engine::Instance().GetRenderer(), "Img/lose_bg.png");
	m_vButtons.push_back(new TitleButton("Img/button_title.png", { 0,0,94,38 }, { 462,400,100,40 }));
	// For the bind: what function, what instance, any parameters.
	m_vButtons.push_back(new ExitButton("Img/button_exit.png", { 0,0,94,38 }, { 462,500,100,40 }));
}

void LoseState::HandleEvents(SDL_Event& event)
{
	State::HandleEvents(event); // Don't need any events specific to TitleState.
}

void LoseState::Update()
{
	for (int i = 0; i < (int)m_vButtons.size(); i++)
		m_vButtons[i]->Update();
}

void LoseState::Render()
{
	cout << "Rendering Lose..." << endl;
	SDL_SetRenderDrawColor(Engine::Instance().GetRenderer(), 255, 0, 0, 255);
	SDL_RenderClear(Engine::Instance().GetRenderer());
	SDL_Rect src = { 0,0,1024,768 }, dst = src;
	SDL_RenderCopy(Engine::Instance().GetRenderer(), m_pBGTexture, &src, &dst);
	for (int i = 0; i < (int)m_vButtons.size(); i++)
		m_vButtons[i]->Render();
	State::Render();
}

void LoseState::Exit()
{
	cout << "Exiting Lose..." << endl;
	SDL_DestroyTexture(m_pBGTexture);
	for (int i = 0; i < (int)m_vButtons.size(); i++)
	{
		delete m_vButtons[i];
		m_vButtons[i] = nullptr;
	}
	m_vButtons.clear();
	m_vButtons.shrink_to_fit();
}
#pragma endregion

#pragma region Begin StateMachine.
void FSM::Update()
{
	if (!m_vStates.empty()) // empty() and back() are methods of the vector type.
		m_vStates.back()->Update();
}

void FSM::HandleEvents()
{
	if (!m_vStates.empty())
	{
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			m_vStates.back()->HandleEvents(event);
		}
	}		
}

void FSM::Render()
{
	if (!m_vStates.empty())
		m_vStates.back()->Render();
}

void FSM::ChangeState(State* pState)
{
	if (!m_vStates.empty())
	{
		m_vStates.back()->Exit();
		delete m_vStates.back();	// De-allocating the state in the heap.
		m_vStates.back() = nullptr; // Nullifying pointer to the de-allocated state.
		m_vStates.pop_back();		// Removes the now-null pointer from the vector.
	}
	PushState(pState); // Invokes method below.
}

void FSM::PushState(State* pState)
{
	m_vStates.push_back(pState);
	m_vStates.back()->Enter();
}

void FSM::PopState()
{
	if (!m_vStates.empty())
	{
		m_vStates.back()->Exit();
		delete m_vStates.back();
		m_vStates.back() = nullptr;
		m_vStates.pop_back();
	}
	m_vStates.back()->Resume();
}

void FSM::Clean()
{
	while (!m_vStates.empty()) // Because we can exit the game in the pause state with the window's 'X'.
	{						   // Ensures that ALL states left in the vector are cleaned up.
		m_vStates.back()->Exit();
		delete m_vStates.back();
		m_vStates.back() = nullptr;
		m_vStates.pop_back();
	}
}

vector<State*>& FSM::GetStates() { return m_vStates; }
#pragma endregion
