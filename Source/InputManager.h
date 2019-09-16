///HEADER STUFF

#pragma once

///INCLUDES
#include "../External/Includes/SDL2/SDL.h"
#include "../External/Includes/SDL2/SDL_stdinc.h"


class InputManager
{
public:
	InputManager();
	~InputManager();

	void update(float dt);

	bool getKeyPress(unsigned int keyscancode);
	bool getKeyTrigger(unsigned int keyscancode);
	bool getKeyReleased(unsigned int keyscancode);

	bool getRightClick();
	bool getRightClickPress();
	bool getRightClickRelease();
	bool getLeftClick();
	bool getLeftClickPress();
	bool getLeftClickRelease();
	int getMouseX();
	int getMouseY();

private:
	Uint8 const *getCurrentKeyboardState();
	Uint8 const *getPreviousKeyboardState();
	Uint32 const getCurrentMouseState();
	Uint32 const getPreviousMouseState();

private:
	Uint8 currKeyboardState[512];
	Uint8 prevKeyboardState[512];

	Uint32 currMouseState;
	Uint32 prevMouseState;
	int mouseX, mouseY;
	int prevX, prevY;
};