///HEADER

#include "PhysicsMath.h"


namespace AuxMath
{
	void EulerIntegration(float dt, int numSteps,
		float Xo, float xVo, float Yo, float yVo,
		std::vector<glm::vec4>& outPointsX, 
		std::vector<glm::vec4>& outPointsY)
	{
		float time = 0.0f;

		float g = 9.8f;
		float m = 5.0f;
		float b = 0.5f;
		float b_over_m = b / m;

		//Define the initial conditions
		glm::vec4 initConditions = glm::vec4(Xo, xVo, Yo, yVo);

		//Push first point
		outPointsX.push_back(glm::vec4(time, Xo, 0.0f, 1.0f));
		outPointsY.push_back(glm::vec4(time, Yo, 0.0f, 1.0f));

		for (int i = 0; i < numSteps; ++i) 
		{
			glm::vec4 G = glm::vec4(
				initConditions[1],
				-b_over_m * initConditions[1],
				initConditions[3],
				-b_over_m * initConditions[3] - g
			);

			glm::vec4 finalCond = initConditions + dt * G;
			initConditions = finalCond;

			time += dt;

			//Add the point into the curve
			outPointsX.push_back(glm::vec4(time, finalCond[0], 0.0f, 1.0f));
			outPointsY.push_back(glm::vec4(time, finalCond[2], 0.0f, 1.0f));
		}
	}

	
	void Runge_Kutta_4_Integration(float dt, int numSteps,
		float Xo, float xVo, float Yo, float yVo,
		std::vector<glm::vec4>& outPointsX, 
		std::vector<glm::vec4>& outPointsY)
	{
		float time = 0.0f;

		float g = 9.8f;
		float m = 5.0f;
		float b = 0.5f;
		float b_over_m = b / m;

		//Define the initial conditions
		glm::vec4 initConditions = glm::vec4(Xo, xVo, Yo, yVo);
		//Push first point
		outPointsX.push_back(glm::vec4(time, Xo, 0.0f, 1.0f));
		outPointsY.push_back(glm::vec4(time, Yo, 0.0f, 1.0f));

		for (int i = 0; i < numSteps; ++i)
		{
			//Get the K values
			glm::vec4 K1 = glm::vec4(
				initConditions[1],
				-b_over_m * initConditions[1],
				initConditions[3],
				-b_over_m * initConditions[3] - g
			);

			glm::vec4 asd1 = initConditions + (dt/2) * K1;
			glm::vec4 K2 = glm::vec4(
				asd1[1],
				-b_over_m * asd1[1],
				asd1[3],
				-b_over_m * asd1[3] - g
			);

			glm::vec4 asd2 = initConditions + (dt / 2) * K2;
			glm::vec4 K3 = glm::vec4(
				asd2[1],
				-b_over_m * asd2[1],
				asd2[3],
				-b_over_m * asd2[3] - g
			);

			glm::vec4 asd3 = initConditions + dt * K3;
			glm::vec4 K4 = glm::vec4(
				asd3[1],
				-b_over_m * asd3[1],
				asd3[3],
				-b_over_m * asd3[3] - g
			);

			K2 = glm::vec4(2 * K2.x, 2 * K2.y, 2 * K2.z, 2 * K2.w);
			K3 = glm::vec4(2 * K3.x, 2 * K3.y, 2 * K3.z, 2 * K3.w);

			//Now get the final point calculation with all the K's
			glm::vec4 finalCond = initConditions + (dt / 6.0f) * (K1 + K2 + K3 + K4);
			initConditions = finalCond;

			time += dt;

			//Add the point into the curve
			outPointsX.push_back(glm::vec4(time, finalCond[0], 0.0f, 1.0f));
			outPointsY.push_back(glm::vec4(time, finalCond[2], 0.0f, 1.0f));
		}
	}


	void LaplaceEval(float dt, int numSteps,
		float Xo, float xVo, float Yo, float yVo,
		std::vector<glm::vec4>& outPointsX,
		std::vector<glm::vec4>& outPointsY)
	{
		float time = 0.0f;

		float g = 9.8f;
		float m = 5.0f;
		float b = 0.5f;
		float b_over_m = b / m;

		//Push first point
		outPointsX.push_back(glm::vec4(time, Xo, 0.0f, 1.0f));
		outPointsY.push_back(glm::vec4(time, Yo, 0.0f, 1.0f));

		for (int i = 0; i < numSteps; ++i)
		{
			float yt = (30 * m / b + m * m*g / (b*b))*(1.0f - expf(-b_over_m * time)) - (m*g / b)*time;
			float xt = (60*cosf(PI/6.0f) * m / b)*(1.0f - expf(-b_over_m * time));

			time += dt;

			//Add the point into the curve
			outPointsX.push_back(glm::vec4(time, xt, 0.0f, 1.0f));
			outPointsY.push_back(glm::vec4(time, yt, 0.0f, 1.0f));
		}
	}
}