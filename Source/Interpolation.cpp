///HEADER

#include "Interpolation.h"
#include "Quaternion.h"

#define EPSILON		0.05f


namespace AuxMath
{

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
}