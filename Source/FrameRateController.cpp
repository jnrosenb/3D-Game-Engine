///HEADER

#include "FrameRateController.h"
#include <iostream>
#include <../External/Includes/SDL2/SDL_timer.h>



FrameRateController::FrameRateController(int framesPerSecond) 
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
}

Uint32 FrameRateController::getFrameTime()
{
	return pFrameTime;
}

void FrameRateController::FrameEnd() 
{
	pTicksEnd = SDL_GetTicks();

	while (pTicksEnd - pTicksBegin < pTicksNeededPerFrame) 
	{
		pTicksEnd = SDL_GetTicks();
	}

	pFrameTime = pTicksEnd - pTicksBegin;
}