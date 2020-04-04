///HEADER

#include "Fourier.h"
#include <iostream>

#define PI      3.14159265359f  //PI


namespace AuxMath
{

	void DFT_1D(std::complex<float> *samples, int width,
		std::complex<float> *dft)
	{
		unsigned N = width;
		std::complex<float> i = std::sqrt(-1.0f);

		//L should be the real length or dimension of the wave
		float L = 100.0f; //For now, later make this a param

		//On^2 DFT - n refers to freq n (0 to N-1)
		for (int n = 0; n < N; ++n)
		{
			float freq_n = (n > N / 2) ? n / L : (N - n) / L;

			//Now, we add over all elements of the sample, with this frequency
			//Wrap up of samples over unit circle, with radius equal to sample value - Measure of C.O.M.
			std::complex<float> fn = 0;
			for (int p = 0; p < N; ++p)
			{
				//Doing extra calculations here for now (to make it clearer while Im learning)
				fn += std::exp(-i * (2 * PI) * freq_n * (p*L / N));
			}

			//Store value of fn in some array
			/// TODO
		}
	}

	//Starting from sample, get DFT coefficients
	void DFT_2D(std::vector<std::complex<float>>& samples, int width,
		int height, std::vector<std::complex<float>>& C, std::vector<std::complex<float>>& OutDFT, float L)
	{
		std::complex<float> i = std::sqrt(std::complex<float>(-1.0f));

		//We need to create matrix C(row, col) from samples(row, col)	****** (ON^3)
		for (int row = 0; row < height; ++row)
		{
			for (int col = 0; col < width; ++col)
			{
				// p: row, q: col
				int p = row, q = col;
				std::complex<float> Cpq = 0;

				//using q as the frequency value
				float freq_q = (q < width / 2) ? q / L : (width - q) / L;

				//For a particular Cpq, we add over all columns
				for (int j = 0; j < width; ++j)
				{
					//Doing extra calculations here for now (to make it clearer while Im learning)
					Cpq += samples[p*width + j] * std::exp(-i * (2 * PI) * freq_q * (j*L / width));
				}

				//Add Cpq to intermediate matrix
				C[row*width + col] = Cpq;
			}
		}

		//Now we do same, but over C to fill the DFT matrix				****** (ON^3)
		for (int row = 0; row < height; ++row)
		{
			for (int col = 0; col < width; ++col)
			{
				// p: row, q: col
				int p = row, q = col;
				std::complex<float> DFTjq = 0;

				//using p as the frequency value
				float freq_p = (p < height / 2) ? p / L : (height - p) / L;

				//For a particular Cpq, we add over all columns
				for (int j = 0; j < height; ++j)
				{
					//Doing extra calculations here for now (to make it clearer while Im learning)
					DFTjq += C[j*width + q] * std::exp(-i * (2 * PI) * freq_p * (j*L / height));
				}

				//Add Cpq to intermediate matrix
				OutDFT[row*width + col] = DFTjq;
			}
		}
	}

	//Starting from DFT coeffs, get signal sample
	void IDFT_2D(std::vector<std::complex<float>>& InDFT, int width,
		int height, std::vector<std::complex<float>>& C, 
		std::vector<std::complex<float>>& Out, float L)
	{
		std::complex<float> i = std::sqrt(std::complex<float>(-1.0f));
		float Winv = 1.0f / width;
		float Hinv = 1.0f / height;

		//We need to create matrix C(row, col) from DFT(row, col)		****** (ON^3)
		for (int row = 0; row < height; ++row)
		{
			for (int col = 0; col < width; ++col)
			{
				// p: row, q: col
				int p = row, q = col;
				std::complex<float> Cjq = 0;

				//using p as the frequency value
				float freq_p = (p < height / 2) ? p / L : (height - p) / L;

				//For a particular Cpq, we add over all columns
				for (int j = 0; j < height; ++j)
				{
					//Doing extra calculations here for now (to make it clearer while Im learning)
					Cjq += InDFT[j*width +q] * std::exp(i * (2 * PI) * freq_p * (j*L / height));
				}

				//Add Cpq to intermediate matrix
				C[row*width + col] = Hinv * Cjq;
			}
		}

		//Now we do same, but over C to fill the samples matrix			****** (ON^3)
		for (int row = 0; row < height; ++row)
		{
			for (int col = 0; col < width; ++col)
			{
				// p: row, q: col
				int p = row, q = col;
				std::complex<float> SAMPpj = 0;

				//using q as the frequency value
				float freq_q = (q < width / 2) ? q / L : (width - q) / L;

				//For a particular Cpq, we add over all columns
				for (int j = 0; j < width; ++j)
				{
					//Doing extra calculations here for now (to make it clearer while Im learning)
					SAMPpj += C[p*width + j] * std::exp(i * (2 * PI) * freq_q * (j*L / width));
				}

				//Add Cpq to intermediate matrix
				Out[row*width + col] = /*Winv * */ SAMPpj;
			}
		}

	}
}

