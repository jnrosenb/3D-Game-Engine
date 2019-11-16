///HEADER

#include <iostream>
#include <cmath>
#include "Affine.h"
#include "Quaternion.h"

namespace AuxMath
{
	float epsilon = 0.000001f;

	glm::mat4 scale(float r)
	{
		glm::mat4 result = { r, 0, 0, 0, 0, r, 0, 0, 0, 0, r, 0, 0, 0, 0, 1 };
		return result;
	}

	glm::mat4 scale(float rx, float ry, float rz)
	{
		glm::mat4 result = { rx, 0, 0, 0, 0, ry, 0, 0, 0, 0, rz, 0, 0, 0, 0, 1 };
		return result;
	}

	glm::mat4 translate(const glm::vec4& v)
	{
		glm::mat4 result = { 1, 0, 0, 0, 
							 0, 1, 0, 0, 
							 0, 0, 1, 0, 
							 v.x, v.y, v.z, 1 };
		return result;
	}

	glm::mat4 rotate(float degree, glm::vec3 const& axis) 
	{
		return AuxMath::Quaternion::QuaternionFromAA(degree, axis).GetRotationMatrix();
	}

	glm::mat4 rotate(float t, const glm::vec4& v)
	{
		//check if angle is zero or if axis is zero vector
		if (t == 0 || v == glm::vec4{ 0, 0, 0, 0 })
			return glm::mat4{ 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };

		//turn degrees into rads
		float PI = 4.0f * atan(1.0f);
		float rads = (PI * t) / 180.0f;

		//Calculations to derive the rotation matrix
		glm::mat3 I = { 1, 0, 0, 0, 1, 0, 0, 0, 1};
		glm::mat3 VVT =
		{
			v.x*v.x, v.x*v.y, v.x*v.z,
			v.y*v.x, v.y*v.y, v.y*v.z,
			v.z*v.x, v.z*v.y, v.z*v.z
		};
		glm::mat3 CrossProductM =
		{
			 0,   -v.z,  v.y,
			 v.z,  0,   -v.x,
			-v.y,  v.x,  0,
		};

		glm::mat3 R = cos(rads) * I + ((1 - cos(rads)) / (v.x*v.x + v.y*v.y + v.z*v.z)) * VVT + (sin(rads)/(sqrt((v.x*v.x + v.y*v.y + v.z*v.z)))) * CrossProductM;
		return 
		{
			R[0][0], R[1][0], R[2][0], 0,
			R[0][1], R[1][1], R[2][1], 0,
			R[0][2], R[1][2], R[2][2], 0,
			0, 0, 0, 1,
		};
	}

	glm::mat4 transpose3x3(const glm::mat4& A)
	{
		glm::mat4 B;
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				if (i == 3 || j == 3)
					B[i][j] = A[i][j];
				else
					B[i][j] = A[j][i];
			}
		}

		return B;
	}

	glm::vec4 cross(const glm::vec4 &u, const glm::vec4 &v)
	{
		glm::vec4 result =
		{
			u.y*v.z - u.z*v.y,
			u.z*v.x - u.x*v.z,
			u.x*v.y - u.y*v.x,
			0
		};
		return result;
	}


	glm::mat4 view(const glm::vec4 &eye, const glm::vec4 &look, const glm::vec4 &rup)
	{
		glm::vec4 right, up, u, v, n;

		right = AuxMath::cross(look, rup);
		up = AuxMath::cross(right, look);

		float rightLength = sqrt(pow(right.x, 2) + pow(right.y, 2) + pow(right.z, 2));
		float upLength = sqrt(pow(up.x, 2) + pow(up.y, 2) + pow(up.z, 2));
		float lookLength = sqrt(pow(look.x, 2) + pow(look.y, 2) + pow(look.z, 2));
		if (fabs(rightLength) <= epsilon || fabs(upLength) <= epsilon || fabs(lookLength) <= epsilon)
		{
			//ERROR, up or right are zero
			std::cout << "Error in view method. Look or rup have zero length" << std::endl;
			return glm::mat4();
		}
		u = { right.x / rightLength, right.y / rightLength, right.z / rightLength, 0 };
		v = { up.x / upLength, up.y / upLength, up.z / upLength, 0 };
		n = { -look.x / lookLength, -look.y / lookLength, -look.z / lookLength, 0 };

		glm::vec3 LtP = {
			-(u.x * eye.x) - (u.y * eye.y) - (u.z * eye.z),
			-(v.x * eye.x) - (v.y * eye.y) - (v.z * eye.z),
			-(n.x * eye.x) - (n.y * eye.y) - (n.z * eye.z),
		};

		return { u.x, v.x, n.x, 0,
				u.y, v.y, n.y, 0,
				u.z, v.z, n.z, 0,
				LtP.x, LtP.y, LtP.z, 1 };
	}

	glm::mat4 perspective(float fov, float aspect, float near)
	{
		if (fabs(near) <= epsilon || fabs(aspect) <= epsilon)
		{
			//TODO see what to return in these cases
			std::cout << "Error in perspective method." << std::endl;
			return glm::mat4();
		}

		float PI = 4.0f * atan(1.0f);
		float radians = (fov * PI) / 180.0f;
		float W = 2 * near * (float)tan(radians / 2.0f);
		float H = W / aspect;

		float A = -1;
		float B = -2 * near;
		float C = (2 * near) / W;
		float D = (2 * near) / H;
		glm::mat4 persp = {
			C, 0, 0, 0,
			0, D, 0, 0,
			0, 0, A, -1,
			0, 0, B, 0
		};

		return persp;
	}


	glm::mat4 perspective(float fov, float aspect, float near, float far)
	{
		float epsilon = 0.0000001f;
		if (fabs(near) <= epsilon || fabs(aspect) <= epsilon || fabs(near - far) <= epsilon)
		{
			//TODO see what to return in these cases
			std::cout << "Error in perspective method." << std::endl;
			return glm::mat4();
		}

		float PI = 4.0f * atan(1.0f);
		float radians = (fov * PI) / 180.0f;
		float W = 2 * near * (float)tan(radians / 2.0f);
		float H = W / aspect;

		float A = (near + far) / (near - far);
		float B = (2 * near * far) / (near - far);
		float C = (2 * near) / W;
		float D = (2 * near) / H;
		glm::mat4 persp = {
			C, 0, 0, 0,
			0, D, 0, 0,
			0, 0, A, -1,
			0, 0, B, 0
		};

		return persp;
	}


	glm::mat4 orthographic(float width, float height, float aspect, float near, float far)
	{
		float epsilon = 0.0000001f;
		if (fabs(near) <= epsilon || fabs(aspect) <= epsilon || fabs(near - far) <= epsilon)
		{
			//TODO see what to return in these cases
			std::cout << "Error in perspective method." << std::endl;
			return glm::mat4();
		}

		float W = width;
		float H = height;

		float A = (near + far) / (near - far);
		float B = (2) / (near - far);
		float C = (2) / W;
		float D = (2) / H;
		glm::mat4 orthographic = {
			C, 0, 0, 0,
			0, D, 0, 0,
			0, 0, A, 0,
			0, 0, B, 1
		};

		return orthographic;
	}
}