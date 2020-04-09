////////////////////////////////
//// Jose Rosenbluth Chiu	////
//// CS561					////
//// Written Homework 03	////
////////////////////////////////

#pragma once

#include "FFT2.h"

//TEMPORARY
#include <cmath>


namespace AuxMath 
{
	glm::vec2 frequencyVector(AuxMath::Grid const& grid, int col, int row)
	{
		//For horizontal frequency
		float physWidth = grid.getPhysicalWidth();
		int gridW = grid.getWidth();
		float hFreq = (col <= gridW / 2) ? col / physWidth : (-gridW + col) / physWidth;

		//For vertical frequency
		float physHeight = grid.getPhysicalHeight();
		int gridH = grid.getHeight();
		float vFreq = (row <= gridH / 2) ? row / physHeight : (-gridH + row) / physHeight;
		
		return {hFreq * 6.3f, vFreq * 6.3f };
		//return {0.25f, 0.0f}; //0.25-0.35
	}
}




