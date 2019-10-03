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
		float sumInv = 1.0f / sum;

		for (int i = 0; i < weights.size(); ++i) 
			weights[i] = weights[i] * sumInv;
	}
}