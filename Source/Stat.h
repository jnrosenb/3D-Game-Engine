#pragma once

#include <vector>
#include "../External/Includes/glm/glm.hpp"

namespace AuxMath
{
	//Low discrepancy random pairs
	struct HammersleyBlock
	{
		int N;
		std::vector<glm::vec4> pairs;
	};

	void genGaussianWeights(int kernelRadius, std::vector<float>& weights);

	void normalizeWeights(std::vector<float>& weights, float sum);

	void genLowDiscrepancyPairs(int N, HammersleyBlock *block);
}
