///HEADER

#include "Interpolation.h"
#include "Quaternion.h"

#define EPSILON		0.05f


namespace AuxMath
{

	//OPERATOR OVERLOADING
	glm::vec4 operator*(glm::vec4 const& rhs, float f)
	{
		glm::vec4 copy = rhs;
		copy.x *= f;
		copy.y *= f;
		copy.z *= f;
		copy.w *= f;
		return copy;
	}

	glm::vec4 operator*(float f, glm::vec4 const& rhs)
	{
		glm::vec4 copy = rhs;
		copy.x *= f;
		copy.y *= f;
		copy.z *= f;
		copy.w *= f;
		return copy;
	}

	glm::vec4 operator/(glm::vec4 const& rhs, float f)
	{
		glm::vec4 copy = rhs;
		copy.x /= f;
		copy.y /= f;
		copy.z /= f;
		copy.w /= f;
		return copy;
	}


	//METHODS
	glm::vec3 Lerp(glm::vec3 const& origin, glm::vec3 const& destination, float alpha) 
	{
		glm::vec3 interp;

		interp.x = (1 - alpha) * origin.x + alpha * destination.x;
		interp.y = (1 - alpha) * origin.y + alpha * destination.y;
		interp.z = (1 - alpha) * origin.z + alpha * destination.z;

		return interp;
	}


	Quaternion Slerp(Quaternion const& origin, Quaternion const& destination, float alpha) 
	{
		Quaternion interp = origin;
		Quaternion orig_aux = origin;

		float d = origin.Dot(destination);
		float a;

		if (d >= (1.0f - EPSILON)) 
		{
			interp = origin * (1 - alpha) + destination * alpha;
			return interp.Normalize();
		}
		else if (d < 0) 
		{
			orig_aux = -origin;
			a = acosf(-d);
		}
		else 
			a = acosf(d);

		//Continue and return
		float sin_a = sinf(a);
		float sin_ta = sinf(alpha * a);
		interp = ((orig_aux * sinf((1 - alpha) * a) + destination * sin_ta)) / sin_a;
		return interp;
	}


	glm::mat4 GenVQSMatrix(glm::vec3 const& v, Quaternion const& q, glm::vec3 const& s) 
	{
		//SCALE
		glm::mat4 S = glm::mat4(1);
		S[0][0] = s.x;
		S[1][1] = s.y;
		S[2][2] = s.z;

		//ROTATION
		glm::mat4 Q = q.GetRotationMatrix();

		//TRANSLATION
		glm::mat4 V = glm::mat4(1);
		V[3][0] = v.x;
		V[3][1] = v.y;
		V[3][2] = v.z;

		return V*Q*S;
	}


	//THIS WORKS ONLY FOR C2 SPLINE CURVE
	void GenerateCurve(int subdivission, float T, std::vector<glm::vec4>& controlPoints, 
		std::map<float, glm::vec4>& curve, std::vector<glm::vec4>& curve_vector)
	{
		//Number of segments
		int numSegments = controlPoints.size() - 3;
		float TperSegment = T / numSegments;

		//t starts at zero, and each iter augments in dt
		float t = 0.0f;
		float dt = T / subdivission;

		for (int i = 0; i <= subdivission; ++i) 
		{
			//Find out in which segment we are
			int segmentBaseIndex = static_cast<int>(t/TperSegment);

			//Get the local t
			float localt = (t - segmentBaseIndex * TperSegment) / TperSegment;

			if (segmentBaseIndex + 3 >= controlPoints.size()) 
				return;

			//Get the 4 points that affect this t
			glm::vec4 P0 = controlPoints[segmentBaseIndex];
			glm::vec4 P1 = controlPoints[segmentBaseIndex + 1];
			glm::vec4 P2 = controlPoints[segmentBaseIndex + 2];
			glm::vec4 P3 = controlPoints[segmentBaseIndex + 3];

			//Weird vector to use for thing (B-SPLINE)
			glm::vec4 P_0 = (-P0 + 3*P1 -3*P2 + P3) / 6.0f;
			glm::vec4 P_1 = (3*P0 -6*P1 + 3*P2) / 6.0f;
			glm::vec4 P_2 = (-3*P0 + 3*P2) / 6.0f;
			glm::vec4 P_3 = (P0 + 4*P1 + P2) / 6.0f;

			//Weird vector to use for thing (CATMULL-ROM)
			///glm::vec4 P_0 = (-P0 + 3*P1 - 3*P2 + P3) / 2.0f;
			///glm::vec4 P_1 = (2*P0 - 5*P1 + 4*P2 - P3) / 2.0f;
			///glm::vec4 P_2 = (-P0 + P2) / 2.0f;
			///glm::vec4 P_3 = (2*P1) / 2.0f;

			//Calculate the curve value for this t
			glm::vec4 Pfinal = P_0*powf(localt, 3) + P_1*powf(localt, 2) + P_2* localt + P_3;

			//Store new point in final list
			curve.emplace(std::make_pair(t, Pfinal));
			curve_vector.push_back(Pfinal);

			//Increment t by step
			t += dt;
		}
	}
}