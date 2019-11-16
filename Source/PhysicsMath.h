#pragma once

#include <iostream>
#include <string>
#include <vector>
#include "../External/Includes/glm/glm.hpp"
#include "../External/Includes/glm/gtc/matrix_transform.hpp"

#define EPSILON		0.000001f
#define PI			3.141592f

namespace AuxMath 
{
	void EulerIntegration(float dt, int numSteps, 
		float Xo, float xVo, float Yo, float yVo,
		std::vector<glm::vec4>& outPointsX, 
		std::vector<glm::vec4>& outPointsY);

	void Runge_Kutta_4_Integration(float dt, int numSteps, 
		float Xo, float xVo, float Yo, float yVo,
		std::vector<glm::vec4>& outPointsX, 
		std::vector<glm::vec4>& outPointsY);

	void LaplaceEval(float dt, int numSteps,
		float Xo, float xVo, float Yo, float yVo,
		std::vector<glm::vec4>& outPointsX,
		std::vector<glm::vec4>& outPointsY);
}
