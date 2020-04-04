////////////////////////////////
//// Jose Rosenbluth Chiu	////
//// CS561					////
//// Written Homework 03	////
////////////////////////////////

#pragma once

#include <vector>
#include <complex>
#include <cmath>
#include "FFT.h"
#include "../External/Includes/glm/glm.hpp"


namespace AuxMath 
{
	////////////////////////
	///// GRID CLASS	////
	////////////////////////
	class Grid 
	{
	public:
		
		Grid() : width(0), height(0),
		physW(0), physH(0), cellW(0), cellH(0),
		baseGrid(std::vector<std::complex<float>>(0))
		{
		}

		Grid(int rows, int cols, float xLen, float yLen) : width(cols), height(rows), 
			physW(xLen), physH(yLen), cellW(xLen/cols), cellH(yLen/rows),
			baseGrid(std::vector<std::complex<float>>(rows*cols))
		{
		}

		void Init(int rows, int cols, float xLen, float yLen) 
		{
			width = cols;
			height = rows;
			physW = xLen;
			physH = yLen;

			cellW = xLen / cols;
			cellH = yLen / rows;

			baseGrid.resize(width * height);
		}

		//CASES
		// grid(3, 4) = value;
		// std::cout << grid(3, 4);
		std::complex<float>& operator()(unsigned x, unsigned y)
		{
			return baseGrid[width*y + x];
		}
		std::complex<float> operator()(unsigned x, unsigned y) const
		{
			return baseGrid[width*y + x];
		}

		int getWidth() const { return width; }
		int getHeight() const { return height; }
		int getWidth() { return width; }
		int getHeight() { return height; }

		float getPhysicalWidth() const { return physW; }
		float getPhysicalHeight() const { return physH; }
		float getPhysicalWidth() { return physW; }
		float getPhysicalHeight() { return physH; }

	private:
		int width, height;
		float cellW, cellH;
		float physW, physH;
		std::vector<std::complex<float>> baseGrid;
	};



	////////////////////////
	///// GET WAVE VEC	////
	////////////////////////
	glm::vec2 frequencyVector(AuxMath::Grid const& grid, int col, int row);



	////////////////////////
	///// FFT2D Base	////
	////////////////////////
	template <typename R>
	class FFT2_Base 
	{
	public:
		FFT2_Base(int lgW, int lgH);
		virtual ~FFT2_Base(void) {}

	protected:
		unsigned const logDimW;
		unsigned const logDimH;
		unsigned const dimW;
		unsigned const dimH;

		std::vector<std::complex<float>> tempRow;
		std::vector<std::complex<float>> tempCol;
	};


	template <class R>
	class FFT2 : public FFT2_Base<R>
	{
	public:
		FFT2(int lgW, int lgH);
		~FFT2(void){}

		void operator()(Grid &A);
	};


	template <class R>
	class IFFT2 : public FFT2_Base<R>
	{
	public:
		IFFT2(int lgW, int lgH);
		~IFFT2(void){}

		void operator()(Grid &A);
	};

	
	
	/////////////////////////////////////////
	//// IMPLEMENTATION (Instantiations) ////
	/////////////////////////////////////////
	template <class R>
	FFT2_Base<R>::FFT2_Base(int lgW, int lgH) :
		logDimW(lgW), logDimH(lgH), dimW(1 << lgW), dimH(1 << lgH),
		tempRow(std::vector<std::complex<float>>(dimW)),
		tempCol(std::vector<std::complex<float>>(dimH))
	{
	}


	template <typename R>
	FFT2<R>::FFT2(int lgW, int lgH) :
		FFT2_Base(lgW, lgH)
	{
	}


	template <typename R>
	IFFT2<R>::IFFT2(int lgW, int lgH) :
		FFT2_Base(lgW, lgH)
	{
	}


	template <class R>
	void FFT2<R>::operator()(Grid& A)
	{
		//---------------------------------------------
		//Let us do it first for rows, then for columns
		//Grid is stored as row major

		//Each element (x,y) will be a DFT coeff 
		//that adds over all row y, with freq x
		//Then, we will get a full row if we call a FFT on a row of A
		//---------------------------------------------
		FFT<float> rowSolver(logDimW);
		FFT<float> colSolver(logDimH);

		//Need intermediate grid
		Grid C = A;

		//Compute intermediate amplitudes from Grid A
		for (unsigned r = 0; r < dimH; r++)
		{
			//Get r-th row into an array
			unsigned const rowLen = dimW;
			for (int i = 0; i < rowLen; ++i)
				tempRow[i] = A(dimW * r + i);
			std::complex<float> *row = rowSolver(&tempRow[0]);

			//Store in same row of intermediate
			for (int i = 0; i < dimW; ++i)
				C(i, r) = row[i];
		}

		//Compute final amplitudes from Grid C into A
		for (unsigned c = 0; c < dimW; c++)
		{
			//Get c-th column into array
			int const colLen = dimH;
			for (int i = 0; i < colLen; ++i)
				tempCol[i] = C(dimW * i + c);
			std::complex<float> *col = colSolver(&tempCol[0]);

			for (int i = 0; i < dimH; ++i)
				A(c, i) = col[i];
		}
	}


	template <class R>
	void IFFT2<R>::operator()(Grid &A)
	{
		//---------------------------------------------
		//Let us do it first for columns, then for rows (inverse)
		//Grid is stored as row major
		//---------------------------------------------
		IFFT<float> rowSolver(logDimW);
		IFFT<float> colSolver(logDimH);

		//Need intermediate grid
		Grid C = A;

		//Compute intermediate amplitudes from Grid A
		for (unsigned r = 0; r < dimH; r++)
		{
			//Get r-th row into an array
			int const rowLen = dimW;
			for (int i = 0; i < rowLen; ++i)
				tempRow[i] = A(i, r);
				//tempRow[i] = A(dimW * r + i);************************
			std::complex<float> *row = rowSolver(&tempRow[0]);

			//Store in same row of intermediate
			for (int i = 0; i < dimW; ++i)
				C(i, r) = row[i];
		}

		//Compute final amplitudes from Grid C into A
		for (unsigned c = 0; c < dimW; c++)
		{
			//Get c-th column into array
			int const colLen = dimH;
			for (int i = 0; i < colLen; ++i)
				tempCol[i] = C(i, c);
				//tempCol[i] = C(dimW * i + c);************************
			std::complex<float> *col = colSolver(&tempCol[0]);

			for (int i = 0; i < dimH; ++i)
				A(c, i) = col[i];
		}
	}
}