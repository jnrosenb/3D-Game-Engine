#pragma once

#include <vector>
#include <cmath>
#include <complex>
#include "../External/Includes/glm/glm.hpp"

namespace AuxMath
{
	void DFT_1D(std::complex<float> *samples, int width,
		std::complex<float> *dft);

	//Starting from sample, get DFT coefficients
	void DFT_2D(std::vector<std::complex<float>>& samples, int width,
		int height, std::vector<std::complex<float>>& C, std::vector<std::complex<float>>& OutDFT, float L);

	//Starting from dft coeffs, get signal sample
	void IDFT_2D(std::vector<std::complex<float>>& InDFT, int width,
		int height, std::vector<std::complex<float>>& C, std::vector<std::complex<float>>& Out, float L);
}
