///HEADER

#include "FrameRateController.h"
#include <iostream>
#include <../External/Includes/SDL2/SDL_timer.h>



FrameRateController::FrameRateController(int framesPerSecond) : 
	paused(false), step(false)
{
	pTicksBegin = 0, pTicksEnd = 0, pFrameTime;

	if (framesPerSecond > 0)
	{
		//Because ticks are in milliseconds
		pTicksNeededPerFrame = 1000 / framesPerSecond;
	}
	else
		pTicksNeededPerFrame = 0;

	//This is just for the first frame
	pFrameTime = pTicksNeededPerFrame;

	std::cout << "FramerateController constructor." << std::endl;
}

FrameRateController::~FrameRateController() 
{
	std::cout << "FramerateController destructor." << std::endl;
}

void FrameRateController::FrameStart()
{
	pTicksBegin = SDL_GetTicks(); 
	handleInput();
}

void FrameRateController::TogglePause()
{
	this->paused = !paused;
}

void FrameRateController::Step()
{
	step = true;
	paused = false;
}

Uint32 FrameRateController::getFrameTime()
{
	return pFrameTime;
}

bool FrameRateController::isPaused()
{
	return paused;
}

void FrameRateController::FrameEnd()
{
	if (paused)
		return;
	if (step)
	{		
		paused = true;
		step = false;
	}

	pTicksEnd = SDL_GetTicks();

	while (pTicksEnd - pTicksBegin < pTicksNeededPerFrame) 
	{
		pTicksEnd = SDL_GetTicks();
	}

	pFrameTime = pTicksEnd - pTicksBegin;
	++frameNumber;
}


////////////////////////////////////
////   INPUT MANAGEMENT         ////
////////////////////////////////////
#include "InputManager.h"
extern InputManager *inputMgr;
void FrameRateController::handleInput()
{
	if (inputMgr->getKeyTrigger(SDL_SCANCODE_L))
	{
		TogglePause();
	}
	if (inputMgr->getKeyTrigger(SDL_SCANCODE_K))
	{
		Step();
	}
}