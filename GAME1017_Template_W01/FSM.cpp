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
	m_pObsText = IMG_LoadTexture(Engine::Instance().GetRenderer(), "Img/Obstacles.png");
	m_vec.reserve(8);
	// Create the vector now.
	for (int i = 0; i < 9; i++)
	{
		Box* temp = new Box({ 128, 128, 128, 128 }, { 128 * i, 384, 128, 128 }, m_pObsText);  // 9 empty boxes
		m_vec.push_back(temp);
	}	
	m_spawnCtr = 2;
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
	case SDL_KEYUP:
		if (event.key.keysym.sym == SDLK_a || event.key.keysym.sym == SDLK_d)
			m_pPlayer->SetAccelX(0.0);
		if (event.key.keysym.sym == SDLK_SPACE)
			m_bSpaceOk = true;
		break;
	}
}

void GameState::Update()
{
	// Check for out of bounds.
	if (m_vec[0]->GetX() <= -128) // Fully off-screen.
	{
		// Clean the first element in the vector.
		delete m_vec[0];
		m_vec[0] = nullptr;
		m_vec.erase(m_vec.begin()); // Pop the front element.
		// Create a new box at the end.
		Box* temp;

		if (++m_spawnCtr == 3)
		{
			m_spawnCtr = 0;
			// Random choice of obstacle


			// Circular saw
			temp = new Box({ 128, 128, 128, 128 }, { 128 * 8, 384, 128, 128 }, m_pObsText, true);
		}
		else
		{
			temp = new Box({ 128, 128, 128, 128 }, { 128 * 8, 384, 128, 128 }, m_pObsText);
		}
		m_vec.push_back(temp);
	}
	// Scroll the boxes.
	for (int col = 0; col < 9; col++)
	{
		m_vec[col]->Update();
	}
	// Updates the player
	switch (m_pPlayer->GetAnimState())
	{
	case RUNNING:
		if (Engine::Instance().KeyDown(SDL_SCANCODE_S))
		{
			m_pPlayer->SetRolling();
		}
		else if (Engine::Instance().KeyDown(SDL_SCANCODE_SPACE) && m_bSpaceOk && m_pPlayer->IsGrounded())
		{
			m_bSpaceOk = false; // This just prevents repeated jumps when holding spacebar.
			m_pPlayer->SetAccelY(-JUMPFORCE); // Sets the jump force.
			m_pPlayer->SetGrounded(false);
			m_pPlayer->SetJumping();
		}
		break;
	case ROLLING:
		if (!Engine::Instance().KeyDown(SDL_SCANCODE_S))
		{
			m_pPlayer->SetRunning();
		}
		break;
	}
	if (Engine::Instance().KeyDown(SDL_SCANCODE_A))
	{
		m_pPlayer->SetDir(-1);
		m_pPlayer->MoveX();
	}
	else if (Engine::Instance().KeyDown(SDL_SCANCODE_D))
	{
		m_pPlayer->SetDir(1);
		m_pPlayer->MoveX();
	}

	m_pPlayer->Update();
	m_pPlayer->SetAccelY(0.0); // After jump, reset vertical acceleration.
	CheckCollision(); // Easy to comment out a function call if needed.
}

void GameState::Render()
{
	SDL_SetRenderDrawColor(Engine::Instance().GetRenderer(), 0, 0, 0, 255);
	SDL_RenderClear(Engine::Instance().GetRenderer()); // Clear the screen with the draw color.
	// Render stuff.
	for (int col = 0; col < 9; col++)
	{
		m_vec[col]->Render();
	}
	SDL_SetRenderDrawColor(Engine::Instance().GetRenderer(), 0, 0, 0, 255);
	SDL_RenderClear(Engine::Instance().GetRenderer()); // Clear the screen with the draw color.
	// Draw the player box.
	SDL_RenderCopy(Engine::Instance().GetRenderer(), m_pPlayerText, m_pPlayer->GetSrcP(), m_pPlayer->GetDstP());
	// Draw the platforms.
	SDL_SetRenderDrawColor(Engine::Instance().GetRenderer(), 192, 64, 0, 255);
	for (int i = 0; i < 4; i++)
		SDL_RenderFillRect(Engine::Instance().GetRenderer(), m_pPlatforms[i]->GetDstP());
	// Draw anew.
	SDL_RenderPresent(Engine::Instance().GetRenderer());
}

void GameState::Exit()
{ 
	for (int col = 0; col < 9; col++)
	{
		delete m_vec[col]; // vector.erase() won't deallocate memory through pointer.
		m_vec[col] = nullptr; // Optional again, but good practice.
	}
	m_vec.clear();
	m_vec.shrink_to_fit();
	delete m_pPlayer;
	for (int i = 0; i < 4; i++)
		delete m_pPlatforms[i];
	SDL_DestroyTexture(m_pPlayerText);
	SDL_DestroyTexture(m_pObsText);
}

void GameState::Resume() { cout << "Resuming Game..." << endl; }

void GameState::CheckCollision()
{
	for (int i = 0; i < 4; i++)
	{
		if (SDL_HasIntersection(m_pPlayer->GetDstP(), m_pPlatforms[i]->GetDstP()))
		{
			if ((m_pPlayer->GetDstP()->y + m_pPlayer->GetDstP()->h) - m_pPlayer->GetVelY() <= m_pPlatforms[i]->GetDstP()->y)
			{ // Collision from top.
				m_pPlayer->SetGrounded(true);
				if (m_pPlayer->GetAnimState() == JUMPING)
				{
					m_pPlayer->SetRunning();
				}
				m_pPlayer->SetVelY(0.0); // Stop the player from moving vertically. We aren't modifying gravity.
				m_pPlayer->SetY(m_pPlatforms[i]->GetDstP()->y - m_pPlayer->GetDstP()->h - 1);
			}
			else if (m_pPlayer->GetDstP()->y - m_pPlayer->GetVelY() >= m_pPlatforms[i]->GetDstP()->y + m_pPlatforms[i]->GetDstP()->h)
			{ // Collision from bottom.
				m_pPlayer->SetVelY(0.0); // Stop the player from moving vertically. We aren't modifying gravity.
				m_pPlayer->SetY(m_pPlatforms[i]->GetDstP()->y + m_pPlatforms[i]->GetDstP()->h + 1);
			}
			else if ((m_pPlayer->GetDstP()->x + m_pPlayer->GetDstP()->w) - m_pPlayer->GetVelX() <= m_pPlatforms[i]->GetDstP()->x)
			{ // Collision from left.
				m_pPlayer->SetVelX(0.0); // Stop the player from moving horizontally.
				m_pPlayer->SetX(m_pPlatforms[i]->GetDstP()->x - m_pPlayer->GetDstP()->w - 1);
			}
			else if (m_pPlayer->GetDstP()->x - m_pPlayer->GetVelX() >= m_pPlatforms[i]->GetDstP()->x + m_pPlatforms[i]->GetDstP()->w)
			{ // Collision from right.
				m_pPlayer->SetVelX(0.0); // Stop the player from moving horizontally.
				m_pPlayer->SetX(m_pPlatforms[i]->GetDstP()->x + m_pPlatforms[i]->GetDstP()->w + 1);
			}
			break;
		}
	}
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
