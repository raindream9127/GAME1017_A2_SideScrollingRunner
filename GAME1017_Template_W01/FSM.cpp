#include <iostream>
#include <SDL.h>
#include <SDL_mixer.h>
#include <cstring>
#include "FSM.h"
#include "Engine.h"
#include "Utilities.h"
#include "tinyxml2.h"
using namespace std;
using namespace tinyxml2;

// Begin State. CTRL+M+H and CTRL+M+U to turn on/off collapsed code.
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
GameState::GameState() : m_distance(0), m_bSpaceOk(true)
{}

void GameState::Enter()
{ 
	cout << "Entering Game..." << endl;

	// Sets up music and SFX
	m_pMusic = Mix_LoadMUS("Aud/game.mp3"); // Load the music track.
	// Load the chunks into the Mix_Chunk vector.
	m_vSounds.reserve(2); // Optional but good practice.
	m_vSounds.push_back(Mix_LoadWAV("Aud/jump.wav"));
	m_vSounds.push_back(Mix_LoadWAV("Aud/dying.wav"));
	Mix_VolumeChunk(m_vSounds[0], 24);

	// Sets up timer and text
	XMLDocument xmlDoc;
	const char* bestRecord;
	char recordText[20] = "Best Time: ";
	xmlDoc.LoadFile("best_record.xml");
	XMLElement* pElement = xmlDoc.FirstChildElement("bestrecord");
	pElement->QueryStringAttribute("time", &bestRecord);
	m_bestRecord = stoi(string(bestRecord));
	strcat_s(recordText, bestRecord);
	t = new TextElement(20, 20, " ", 14);
	t2 = new TextElement(20, 40, recordText, 14, { 255, 0, 0, 255 });
	ti = new Timer();
	ti->Start();
	string temp = "Time: 0";
	t->SetText(temp.c_str());

	// Loads textures
	m_pObsText = IMG_LoadTexture(Engine::Instance().GetRenderer(), "Img/Obstacles.png");
	m_pBackgroundText = IMG_LoadTexture(Engine::Instance().GetRenderer(), "Img/backgrounds.png");
	m_pPlayerText = IMG_LoadTexture(Engine::Instance().GetRenderer(), "Img/Player.png");

	// Creates sprite objects
	m_pPlayer = new Player({ 0, 512, 128, 128 }, { 128, 384, 128, 128 });
	m_pBackground[0] = new Background({ 0, 0, 1024, 768 });
	m_pBackground[1] = new Background({ 1024, 0, 1024, 768 });
	m_pMidBackground[0] = new MidBackground({ 0, 0, 256, 512 });
	m_pMidBackground[1] = new MidBackground({ 256, 0, 256, 512 });
	m_pMidBackground[2] = new MidBackground({ 512, 0, 256, 512 });
	m_pMidBackground[3] = new MidBackground({ 768, 0, 256, 512 });
	m_pMidBackground[4] = new MidBackground({ 1024, 0, 256, 512 });
	m_pPlatforms.push_back(new Platform({ 0, 512, 512, 256 }));
	m_pPlatforms.push_back(new Platform({ 512, 512, 512, 256 }));
	m_pPlatforms.push_back(new Platform({ 1024, 512, 512, 256 }));	

	// Plays music
	Mix_PlayMusic(m_pMusic, -1); // Play. -1 = looping.
	Mix_VolumeMusic(16); // 0-MIX_MAX_VOLUME (128).
}

void GameState::HandleEvents(SDL_Event& event)
{
	if (m_pPlayer->GetAnimState() != DYING)
	{
		switch (event.type)
		{
		case SDL_KEYDOWN: // Try SDL_KEYUP instead.
			if (event.key.keysym.sym == SDLK_p)
			{
				ti->Pause();
				Engine::Instance().GetFSM().PushState(new PauseState());
				return;
			}
			else if (event.key.keysym.sym == SDLK_x)
			{
				Engine::Instance().GetFSM().ChangeState(new TitleState());
				return;
			}
			else if (event.key.keysym.sym == SDLK_b)
			{
				t2->SetColor({ 0, 0, 255, 255 });
			}
			else if (event.key.keysym.sym == SDLK_x)
			{
				t2->SetColor({ (Uint8)(rand() % 255), (Uint8)(rand() % 255),
					(Uint8)(rand() % 255), 255 });
			}
			break;
		case SDL_KEYUP:
			if (event.key.keysym.sym == SDLK_a || event.key.keysym.sym == SDLK_d)
				m_pPlayer->SetAccelX(0.0);
			if (event.key.keysym.sym == SDLK_SPACE)
				m_bSpaceOk = true;
			break;
		default:
			State::HandleEvents(event);
			break;
		}
	}	
}

void GameState::Update()
{
	if (m_pPlayer->GetAnimState() != DYING)
	{
		// Updates repeatable sprites
		for (auto i : m_pBackground)
		{
			i->Update();
			if (i->GetDstP()->x < -1024)
			{
				i->GetDstP()->x += 2048;
			}
		}
		for (auto i : m_pMidBackground)
		{
			i->Update();
			if (i->GetDstP()->x < -256)
			{
				i->GetDstP()->x += 1280;
			}
		}
		for (int i = 0; i < 3; ++i)
		{
			m_pPlatforms[i]->Update();
			if (m_pPlatforms[i]->GetDstP()->x < -512)
			{
				m_pPlatforms[i]->GetDstP()->x += 1536;
			}
		}
		// Updates random sprites
		for (int i = 3; i < (int)m_pPlatforms.size(); ++i)  // Flying platforms
		{
			m_pPlatforms[i]->Update();
			if (m_pPlatforms[i]->GetDstP()->x < -128)
			{
				delete m_pPlatforms[i];
				m_pPlatforms.erase(m_pPlatforms.begin() + i);
				m_pPlatforms.shrink_to_fit();
			}
		}
		for (int i = 0; i < (int)m_pObstacles.size(); ++i)  // Other random sprites
		{
			m_pObstacles[i]->Update();
			if (m_pObstacles[i]->GetDstP()->x < -128)
			{
				delete m_pObstacles[i];
				m_pObstacles.erase(m_pObstacles.begin() + i);
				m_pObstacles.shrink_to_fit();
			}
		}
		m_distance += 3;
		if (m_distance > 384)
		{
			m_distance = 0;
			// Spawns random obstacles
			int random = rand() % 4;
			switch (random)
			{
			case 0:
				m_pPlatforms.push_back(new Flyingplatform());
				break;
			case 1:
				m_pObstacles.push_back(new Circularsaw());
				break;
			case 2:
				m_pObstacles.push_back(new Spikes());
				break;
			case 3:
				m_pObstacles.push_back(new Spikewall());
				break;
			}
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
				Mix_PlayChannel(-1, m_vSounds[0], 0);
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
		// Updates timer
		ti->Update();
		// Collision resolution
		CheckCollision(); // Easy to comment out a function call if needed.
	}
	else
	{
		m_pPlayer->Animate();
	}		
}

void GameState::Render()
{
	SDL_SetRenderDrawColor(Engine::Instance().GetRenderer(), 0, 0, 0, 255);
	SDL_RenderClear(Engine::Instance().GetRenderer()); // Clear the screen with the draw color.
	
	// Draw the backgrounds
	for (auto i : m_pBackground)
	{
		i->Render(m_pBackgroundText);
	}
	for (auto i : m_pMidBackground)
	{
		i->Render(m_pBackgroundText);
	}
	// Draw the obstacles
	for (auto i : m_pObstacles)
	{
		i->Render(m_pObsText);
	}
	// Draw the platforms.
	for (int i = 0; i < 3; ++i)
	{
		m_pPlatforms[i]->Render(m_pBackgroundText);
	}
	for (int i = 3; i < (int)m_pPlatforms.size(); ++i)
	{
		m_pPlatforms[i]->Render(m_pObsText);
	}
	// Draw the player
	m_pPlayer->Render(m_pPlayerText);
	// Draw the timer
	string temp = "Time: " + ti->GetTime();
	if (ti->HasChanged())
	{
		t->SetText(temp.c_str());
	}
	t->Render();
	t2->Render();
}

void GameState::Exit()
{
	// Saves the best score
	if (m_bestRecord < stoi(ti->GetTime()))
	{
		XMLDocument xmlDoc;
		xmlDoc.LoadFile("best_record.xml");
		XMLElement* pElement = xmlDoc.FirstChildElement("bestrecord");
		pElement->SetAttribute("time", ti->GetTime().c_str());
		xmlDoc.SaveFile("best_record.xml");
	}	
	// Deletes timer and texts
	delete t;
	delete t2;
	delete ti;
	// Delete sprites
	delete m_pPlayer;
	for (auto i : m_pBackground)
	{
		delete i;
		i = nullptr;
	}
	for (auto i : m_pMidBackground)
	{
		delete i;
		i = nullptr;
	}
	for (auto i : m_pPlatforms)
	{
		delete i;
		i = nullptr;
	}
	m_pPlatforms.clear();
	m_pPlatforms.shrink_to_fit();
	for (auto i : m_pObstacles)
	{
		delete i;
		i = nullptr;
	}
	m_pObstacles.clear();
	m_pObstacles.shrink_to_fit();
	// Destroy textures
	SDL_DestroyTexture(m_pBackgroundText);
	SDL_DestroyTexture(m_pPlayerText);
	SDL_DestroyTexture(m_pObsText);
	// Deals with musics and sound resources
	for (int i = 0; i < (int)m_vSounds.size(); i++)
	{
		Mix_FreeChunk(m_vSounds[i]);
	}		
	Mix_FreeMusic(m_pMusic);
	m_vSounds.clear();
	m_vSounds.shrink_to_fit();
}

void GameState::Resume() 
{ 
	cout << "Resuming Game..." << endl;
	ti->Resume();
}

void GameState::CheckCollision()
{
	// Collisions with platforms
	for (int i = 0; i < (int)m_pPlatforms.size(); ++i)
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

	// Collision with obstacles
	for (int i = 0; i < (int)m_pObstacles.size(); ++i)
	{
		if (SDL_HasIntersection(m_pPlayer->GetCollisionRect(), m_pObstacles[i]->GetCollisionRect()))
		{
			m_pPlayer->SetDeath();
			Mix_PlayChannel(-1, m_vSounds[1], 0);
		}
	}
}
#pragma endregion

#pragma region Begin TitleState.
TitleState::TitleState() {}

void TitleState::Enter()
{ 
	cout << "Entering Title..." << endl;
	m_pBGTexture = IMG_LoadTexture(Engine::Instance().GetRenderer(), "Img/title_bg.png");
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
	SDL_RenderPresent(Engine::Instance().GetRenderer());
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
