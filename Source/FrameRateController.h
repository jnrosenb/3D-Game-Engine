///HEADER

#pragma once

#include "../External/Includes/SDL2/SDL_stdinc.h"

class FrameRateController 
{
public:
	FrameRateController(int framesPerSecond);
	~FrameRateController();

	void FrameStart();
	void FrameEnd(); 
	Uint32 getFrameTime();

private:
	Uint32 pTicksBegin;
	Uint32 pTicksEnd;
	Uint32 pTicksNeededPerFrame;

	Uint32 pFrameTime;
};