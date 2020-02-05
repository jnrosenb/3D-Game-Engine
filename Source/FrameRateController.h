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
	
	//Debug functions
	void TogglePause();
	void Step();
	
	Uint32 getFrameTime();
	bool isPaused();

private:
	size_t frameNumber;
	bool paused;
	bool step;
	Uint32 pTicksBegin;
	Uint32 pTicksEnd;
	Uint32 pTicksNeededPerFrame;

	Uint32 pFrameTime;

private:
	////////////////////////////////////
	////   INPUT MANAGEMENT         ////
	////////////////////////////////////
	void handleInput();
};