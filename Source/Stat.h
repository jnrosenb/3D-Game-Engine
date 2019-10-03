#pragma once

#include <vector>

namespace AuxMath
{
	void genGaussianWeights(int kernelRadius, std::vector<float>& weights);

	void normalizeWeights(std::vector<float>& weights, float sum);
}
