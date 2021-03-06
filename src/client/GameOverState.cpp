#include "GameOverState.h"
#include "LobbyState.h"
#include "StateManager.h"
#include "TextRenderer.h"
#include "Window.h"

#include "../Graphics/Shader.h"
#include "ClientGame.h"
#include "Graphics\Scene.h"


GOState::GOState(CStateManager* pManager)
	: CGameState(pManager)
{
	// Create the text controls of the menu.
	sprite_renderer = new SpriteRenderer();
	initialized = false;
}

GOState::~GOState()
{
	delete sprite_renderer;
	delete bg;
	delete win;
	delete lose;
	delete tied;
}

GOState* GOState::GetInstance(CStateManager* pManager)
{
	static GOState Instance(pManager);
	return &Instance;
}

void GOState::OnKeyDown(int action, int key)
{
	switch (key)
	{
	case GLFW_KEY_ESCAPE:
		glfwSetWindowShouldClose(ClientGame::instance()->window, GL_TRUE);
		break;
	default:
		break;
	}
}

void GOState::OnClick(int button, int action, double x, double y) {
	GLubyte res[4];
	GLint viewport[4];

	// render selection 
	RenderSelection();

	glGetIntegerv(GL_VIEWPORT, viewport);
	glReadPixels(x, viewport[3] - y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &res);

    if (action == GLFW_PRESS)
    {
        switch (res[0]) {
            case 0: printf("None clicked\n"); break;
            case 1: printf("play again clicked\n");
                // todo - reset game  
                // change state
                m_pStateManager->ChangeState(CMenuState::GetInstance(m_pStateManager));
                break;
            default: printf("%d clicked%s\n", res[0]);
        }
    }
}

void GOState::RenderSelection() {
	InitTextures();

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	////////////// PLAY AGAIN BUTTON /////////////////////////////////////

}


void GOState::Draw()
{
	InitTextures();

	int x = Texture::GetWindowCenter(bg->Width());
	int y = Window::height/2 - bg->Height()/2;

	sprite_renderer->DrawSprite(*bg, glm::vec2(x, y), glm::vec2(bg->Width(), bg->Height()), 0.0f, glm::vec3(1.0f, 1.0f, 1.0f));

	glClear(GL_DEPTH_BUFFER_BIT); // remove depth info so text and buttons go on top

	//////////////// DISPLAY WINNER ////////////////////////////////
	if (ClientGame::instance()->GetWinner() == -1) { // tie 
		sprite_renderer->DrawSprite(*tied, glm::vec2(x, y), glm::vec2(tied->Width(), tied->Height()), 0.0f, glm::vec3(1.0f, 1.0f, 1.0f));

	} else if (ClientGame::instance()->GetWinner() == ClientGame::instance()->GetClientTeam()) {
		sprite_renderer->DrawSprite(*win, glm::vec2(x, y), glm::vec2(win->Width(), win->Height()), 0.0f, glm::vec3(1.0f, 1.0f, 1.0f));
	}
	else {
		sprite_renderer->DrawSprite(*lose, glm::vec2(x, y), glm::vec2(lose->Width(), lose->Height()), 0.0f, glm::vec3(1.0f, 1.0f, 1.0f));
	}
}

void GOState::EnterState()
{
	ClientGame::instance()->StopMenuSound();
	Window::mouseCaptured = false;
	glfwSetInputMode(ClientGame::instance()->window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

void GOState::Update(DWORD dwCurrentTime)
{
}

void::GOState::InitTextures() {
	if (!initialized) {
		// Create the different images
		bg = new Texture(GL_TEXTURE_2D, "assets/ui/gameover/gameover.png");

		win = new Texture(GL_TEXTURE_2D, "assets/ui/gameover/win.png");
		lose = new Texture(GL_TEXTURE_2D, "assets/ui/gameover/lose.png");
		tied = new Texture(GL_TEXTURE_2D, "assets/ui/gameover/tied.png");

		initialized = true;
	}
}