////////////////////////////////
//// Jose Rosenbluth Chiu	////
//// CS561					////
//// Written Homework 03	////
////////////////////////////////

#pragma once

#include "FFT2.h"

//TEMPORARY
#include "Stat.h"


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
		
		return {0.35f, 0};//hFreq, vFreq }; //0.25-0.35
	}
}




