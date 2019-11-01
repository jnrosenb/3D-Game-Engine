/// HEADER STUFF

/// Includes
#include "InputManager.h"
#include "../External/Includes/SDL2/SDL_keyboard.h"
#include "../External/Includes/SDL2/SDL_mouse.h"
#include <iostream>



InputManager::InputManager()
{
	Uint32 keyboardLen = 512;

	this->currMouseState = 0;
	this->prevMouseState = 0;
	mouseX = 0;
	mouseY = 0;
	prevX = 0;
	prevY = 0;

	SDL_memset(this->currKeyboardState, 0, keyboardLen * sizeof(Uint8));
	SDL_memset(this->prevKeyboardState, 0, keyboardLen * sizeof(Uint8));

	std::cout << "Input manager constructor." << std::endl;
}

InputManager::~InputManager()
{
	std::cout << "Input manager destructor." << std::endl;
}

void InputManager::update(float dt)
{
	///if (ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow))
	///{
	///	SDL_memset(m_Keyboard_Current_State, 0, 512 * sizeof(Uint8));
	///	SDL_memset(m_Keyboard_Previous_State, 0, 512 * sizeof(Uint8));
	///	m_Mouse_Current_State = 0;
	///	m_Mouse_Previous_State = 0;
	///	m_wheel_y = 0.f;
	///	return;
	///}

	//Keyboard update
	int keyboardLen;
	Uint8 const *keyboardState = SDL_GetKeyboardState(&keyboardLen);

	if (keyboardLen > 512)
		keyboardLen = 512;

	SDL_memcpy(this->prevKeyboardState, this->currKeyboardState, keyboardLen);
	SDL_memcpy(this->currKeyboardState, keyboardState, keyboardLen);

	//Mouse update
	prevX = mouseX;
	prevY = mouseY;
	prevMouseState = currMouseState;
	currMouseState = SDL_GetMouseState(&mouseX, &mouseY);
}

bool InputManager::getKeyPress(unsigned int keyscancode)
{
	if (keyscancode > 512)
		return false;

	if (currKeyboardState[keyscancode])
		return true;
	return false;
}

bool InputManager::getKeyTrigger(unsigned int keyscancode)
{
	if (keyscancode > 512)
		return false;

	if (currKeyboardState[keyscancode] && !prevKeyboardState[keyscancode])
		return true;
	return false;
}

bool InputManager::getKeyReleased(unsigned int keyscancode)
{
	if (keyscancode > 512)
		return false;

	if (!currKeyboardState[keyscancode] && prevKeyboardState[keyscancode])
		return true;
	return false;
}

bool InputManager::getRightClick()
{
	return (currMouseState & SDL_BUTTON(SDL_BUTTON_RIGHT)) &&
		!(prevMouseState & SDL_BUTTON(SDL_BUTTON_RIGHT));
}

bool InputManager::getRightClickPress()
{
	return (currMouseState & SDL_BUTTON(SDL_BUTTON_RIGHT));
}

bool InputManager::getRightClickRelease()
{
	return !(currMouseState & SDL_BUTTON(SDL_BUTTON_RIGHT)) &&
		(prevMouseState & SDL_BUTTON(SDL_BUTTON_RIGHT));
}

bool InputManager::getLeftClick()
{
	return (currMouseState & SDL_BUTTON(SDL_BUTTON_LEFT)) &&
		!(prevMouseState & SDL_BUTTON(SDL_BUTTON_LEFT));
}

bool InputManager::getLeftClickPress()
{
	return (currMouseState & SDL_BUTTON(SDL_BUTTON_LEFT));
}

bool InputManager::getLeftClickRelease()
{
	return !(currMouseState & SDL_BUTTON(SDL_BUTTON_LEFT)) &&
		(prevMouseState & SDL_BUTTON(SDL_BUTTON_LEFT));
}

int InputManager::getMouseX()
{
	return mouseX;
}

int InputManager::getMouseY()
{
	return mouseY;
}

Uint8 const *InputManager::getCurrentKeyboardState()
{
	return currKeyboardState;
}

Uint8 const *InputManager::getPreviousKeyboardState()
{
	return prevKeyboardState;
}

Uint32 const InputManager::getCurrentMouseState()
{
	return currMouseState;
}

Uint32 const InputManager::getPreviousMouseState()
{
	return prevMouseState;
}