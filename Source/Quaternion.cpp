///HEADER

#include "Quaternion.h"

namespace AuxMath
{
	Quaternion operator*(Quaternion const& rhs, float f);
	Quaternion operator*(float f, Quaternion const& rhs);
	Quaternion operator/(Quaternion const& rhs, float f);

	Quaternion operator*(Quaternion const& rhs, float f)
	{
		Quaternion copy = rhs;
		copy.s *= f;
		copy.x *= f;
		copy.y *= f;
		copy.z *= f;
		return copy;
	}

	Quaternion operator*(float f, Quaternion const& rhs) 
	{
		Quaternion copy = rhs;
		copy.s *= f;
		copy.x *= f;
		copy.y *= f;
		copy.z *= f;
		return copy;
	}


	Quaternion operator/(Quaternion const& rhs, float f)
	{
		Quaternion copy = rhs;
		copy.s /= f;
		copy.x /= f;
		copy.y /= f;
		copy.z /= f;
		return copy;
	}
}