#pragma once

#include <iostream>
#include <assert.h>
#include <string>
#include "../External/Includes/glm/glm.hpp"
#include "../External/Includes/glm/gtc/matrix_transform.hpp"

#define SQRT_EPSILON 0.001f

namespace AuxMath 
{
	struct Quaternion 
	{
		friend Quaternion operator*(float f, Quaternion const& rhs);
		friend Quaternion operator*(Quaternion const& rhs, float f);
		friend Quaternion operator/(Quaternion const& rhs, float f);
		friend Quaternion operator/(float f, Quaternion const& rhs);

		float s;
		float x, y, z;

		Quaternion() : s(0.0f), 
			x(0.0f), y(0.0f), z(0.0f) 
		{}

		Quaternion(float s, float x, 
			float y, float z) : s(s),
			x(x), y(y), z(z)
		{}

		Quaternion(float s, glm::vec3 const& v) : s(s),
			x(v.x), y(v.y), z(v.z)
		{}

		Quaternion(glm::vec4 const& q) : s(q.w),
			x(q.x), y(q.y), z(q.z)
		{}

		Quaternion(Quaternion const& rhs) : s(rhs.s),
			x(rhs.x), y(rhs.y), z(rhs.z)
		{}

		Quaternion(glm::mat4 const& rhs)
		{
			this->s = 0.5f * std::sqrtf(rhs[0][0] + rhs[1][1] + rhs[2][2] + 1);
			this->x = (rhs[2][1] - rhs[1][2]) / 4*s;
			this->y = (rhs[0][2] - rhs[2][0]) / 4*s;
			this->z = (rhs[1][0] - rhs[0][1]) / 4*s;
		}

		Quaternion& operator=(Quaternion const& rhs)
		{
			if (this != &rhs) 
			{
				this->s = rhs.s;
				this->x = rhs.x;
				this->y = rhs.y;
				this->z = rhs.z;
			}
			return *this;
		}

		Quaternion& operator=(glm::vec4 const& rhs)
		{
			this->s = rhs.w;
			this->x = rhs.x;
			this->y = rhs.y;
			this->z = rhs.z;
			return *this;
		}

		static Quaternion QuaternionFromAA(float degree, glm::vec3 const& axis) 
		{
			//Transform degrees, just in case this affects
			/// if (degree > 360.0f || degree < -360.0f) 
			/// {
			/// 	int aux = static_cast<int>(degree/360.0f);
			/// 	degree = degree - aux * 360.0f;
			/// }

			float radians = glm::radians(degree);
			float temp = radians / 2;
			glm::vec3 A = glm::normalize(axis);
			return Quaternion(cosf(temp), sinf(temp) * A);
		}

		//BOTH CASES (pending)
		//	float a = q[2];
		//	q[3] = 234.0f;
		float operator[](unsigned int index) 
		{
			assert(index >= 0 && index < 4);

			switch (index) 
			{
			case 0:
				return this->x;
			case 1:
				return this->y;
			case 2:
				return this->z;
			case 3:
				return this->s;
			};
		}
		//Second case (not yet)
		
		//////////////////////////
		////    Operations    ////
		//////////////////////////

		//Addition
		Quaternion& operator+=(Quaternion const& rhs)
		{
			this->s += rhs.s;
			this->x += rhs.x;
			this->y += rhs.y;
			this->z += rhs.z;
			return *this;
		}

		Quaternion const operator+(Quaternion const& rhs) const
		{
			Quaternion copy = Quaternion(*this);
			copy += rhs;
			return copy;
		}
		
		////multiplication
		Quaternion& operator*=(Quaternion const& rhs)
		{
			glm::vec3 v1(x, y, z);
			glm::vec3 v2(rhs.x, rhs.y, rhs.z);
			glm::vec3 v(s * v2 + rhs.s * v1 + glm::cross(v1, v2));
			this->s = (s * rhs.s) - (x * rhs.x + y * rhs.y + z * rhs.z);
			this->x = v.x;
			this->y = v.y;
			this->z = v.z;

			return *this;
		}
		
		Quaternion& operator*=(float scalar)
		{
			this->s *= scalar;
			this->x *= scalar;
			this->y *= scalar;
			this->z *= scalar;
			return *this;
		}

		Quaternion& operator/=(float scalar)
		{
			this->s /= scalar;
			this->x /= scalar;
			this->y /= scalar;
			this->z /= scalar;
			return *this;
		}

		Quaternion operator-() const
		{
			Quaternion q;
			q.s = -this->s;
			q.x = -this->x;
			q.y = -this->y;
			q.z = -this->z;
			return q;
		}

		Quaternion operator*(Quaternion const& rhs) const
		{
			Quaternion copy = Quaternion(*this);
			copy *= rhs;
			return copy;
		}

		Quaternion operator*(float scalar)
		{
			Quaternion copy = Quaternion(*this);
			copy *= scalar;
			return copy;
		}

		Quaternion operator/(float scalar)
		{
			Quaternion copy = Quaternion(*this);
			copy /= scalar;
			return copy;
		}
	
		void print(std::string name) const
		{
			std::cout << name << ": [" << this->s << ", " << this->x << ", " << this->y << ", " << this->z << "]" << std::endl;
		}

		//Inverse
		Quaternion Conjugate() const 
		{
			return Quaternion(s, -x, -y, -z);
		}
		
		Quaternion Inverse() const
		{
			if (IsUnitQuaternion())
				return Conjugate();

			return Conjugate() / SqrLen();
		}


		Quaternion Normalize() const
		{
			if (IsUnitQuaternion())
				return Quaternion(s, x, y, z);

			return Quaternion(s, x, y, z) / Len();
		}

		//Length
		float SqrLen() const
		{
			return s * s + x * x + y * y + z * z;
		}
		
		float Len() const
		{
			return sqrtf( this->SqrLen() );
		}

		bool IsUnitQuaternion() const
		{
			return fabs(SqrLen() - 1.0f) <= SQRT_EPSILON;
		}

		//Dot
		float Dot(Quaternion const& rhs) const
		{
			return (s * rhs.s) + (x * rhs.x) + (y * rhs.y) + (z * rhs.z);
		}

		//Matrix
		glm::mat4 GetRotationMatrix() const 
		{
			float s2 = s * s;
			float x2 = x * x;
			float y2 = y * y;
			float z2 = z * z;

			float _2sx = 2 * s * x;
			float _2sy = 2 * s * y;
			float _2sz = 2 * s * z;

			float _2xy = 2 * x * y;
			float _2xz = 2 * x * z;
			float _2yz = 2 * y * z;

			glm::mat4 matrix = glm::mat4(1);

			matrix[0][0] = 1 - 2 * (y2 + z2);
			matrix[1][1] = 1 - 2 * (x2 + z2);
			matrix[2][2] = 1 - 2 * (x2 + y2);

			//column major
			matrix[1][0] = _2xy - _2sz;
			matrix[2][0] = _2xz + _2sy;

			matrix[0][1] = _2xy + _2sz;
			matrix[2][1] = _2yz - _2sx;

			matrix[0][2] = _2xz - _2sy;
			matrix[1][2] = _2yz + _2sx;

			return matrix;
		}


		//Rotation
		static Quaternion Unit()
		{
			return Quaternion (1, 0, 0, 0);
		}

		//Rotation
		static glm::vec3 Rotate1(float degrees, glm::vec3 const& axis, glm::vec3 const& r)
		{
			float radians = glm::radians(degrees);
			float temp = radians / 2;

			glm::vec3 A  = glm::normalize(axis);
			
			Quaternion q = Quaternion(cosf(temp), sinf(temp) * A);
			Quaternion result = q * Quaternion(0, r) * q.Conjugate();
			return glm::normalize(glm::vec3(result.x, result.y, result.z));
		}

		static glm::vec3 Rotate2(Quaternion const& q, glm::vec3 const& r)
		{
			Quaternion result = q * Quaternion(0, r) * q.Conjugate();
			return glm::normalize(glm::vec3(result.x, result.y, result.z));
		}
	};
}
