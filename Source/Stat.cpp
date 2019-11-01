///HEADER

#include "Stat.h"
#include <cmath>
#include <iostream>
	
namespace AuxMath
{

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
				if (kk & 1)
					u += p;
			float v = (k + 0.5) / N;

			glm::vec2 pair(u, v); 
			pair = glm::normalize(pair);
			block->pairs.push_back(pair);
		}
	}
}

