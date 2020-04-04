///HEADER

#include "Stat.h"
#include <cmath>
#include <iostream>
	
#define PI		3.14159265359f
#define TwoPi	2*PI


namespace AuxMath
{

	glm::vec2 SampleGaussian_BoxMuller(float stdev) 
	{
		glm::vec2 result(0);
		float u = rand() / static_cast<float>(RAND_MAX);
		float v = rand() / static_cast<float>(RAND_MAX);

		result.x = std::sqrt(-2 * stdev * std::log(u)) * std::cos(TwoPi*v);
		result.y = std::sqrt(-2 * stdev * std::log(u)) * std::sin(TwoPi*v);
		return result;
	}


	void genGaussianWeights(int kernelRadius, std::vector<float>& weights) 
	{
		int w = kernelRadius;
		float stdDevInv = 2.0f / w;
		float sum = 0.0f;

		for (int i = -w; i <= w; ++i)
		{
			float val = i * stdDevInv;
			float weight = expf( -(0.5f) * (val * val) );
			weights.push_back(weight);
			sum += weight;
		}

		normalizeWeights(weights, sum);
	}


	void normalizeWeights(std::vector<float>& weights, float sum) 
	{
		float sumInv = 0.0f;
		if (sum != 0.0f)
			sumInv = 1.0f / sum;

		for (int i = 0; i < weights.size(); ++i) 
			weights[i] = weights[i] * sumInv;
	}


	void genLowDiscrepancyPairs(int N, HammersleyBlock *block)
	{
		block->N = N; 
		int kk = 0;
		for (int k = 0; k < N; k++) 
		{
			float u = 0.0f;
			float p = 0.0f;
			for (p = 0.5f, kk = k; kk; p *= 0.5f, kk >>= 1)
			{
				if (kk & 1)
					u += p;
			}
			if (N == 0)
				continue;
			float v = (k + 0.5f) / N;
			glm::vec4 pair(u, v, 0, 0); 
			block->pairs.push_back(pair);
		}
	}
}

