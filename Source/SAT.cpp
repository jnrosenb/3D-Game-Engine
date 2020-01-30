//HEADER STUFF

#pragma once

#include "../External/Includes/glm/glm.hpp"
#include "SAT.h"
#include <iostream>

#define EPSILON		0.000001f


namespace AuxMath
{
	//SAT to test if two OBB overlap or now
	bool TestOBB_OBB(OBB const& A, OBB const& B, glm::vec4& restitution) 
	{
		//Assuming axis are world space reps. Doing all in world space then
		glm::vec3 T = A.position - B.position;
		glm::vec3 L;
		float dist, radA, radB; 		
		float minRestitution = 1000000.0f;
		glm::vec3 restitutionN = glm::vec3(0);

		//Three axis of A
		L = A.axis[0];
		dist = fabsf(glm::dot(L, T));
		radA = A.radius.x;
		radB = fabsf(glm::dot(L, B.axis[0] * B.radius.x)) + fabsf(glm::dot(L, B.axis[1] * B.radius.y)) + fabsf(glm::dot(L, B.axis[2] * B.radius.z));
		if (dist > (radA + radB))
			return false;
		else 
		{
			float rest = (radA + radB) - dist;
			if (rest < minRestitution) 
			{
				minRestitution = rest;
				restitutionN = L;
			}
		}

		L = A.axis[1];
		dist = fabsf(glm::dot(L, T));
		radA = A.radius.y;
		radB = fabsf(glm::dot(L, B.axis[0] * B.radius.x)) + fabsf(glm::dot(L, B.axis[1] * B.radius.y)) + fabsf(glm::dot(L, B.axis[2] * B.radius.z));
		if (fabsf(dist) > (radA) + (radB))
			return false;
		else
		{
			float rest = (radA + radB) - dist;
			if (rest < minRestitution)
			{
				minRestitution = rest;
				restitutionN = L;
			}
		}

		L = A.axis[2];
		dist = fabsf(glm::dot(L, T));
		radA = A.radius.z;
		radB = fabsf(glm::dot(L, B.axis[0] * B.radius.x)) + fabsf(glm::dot(L, B.axis[1] * B.radius.y)) + fabsf(glm::dot(L, B.axis[2] * B.radius.z));
		if (fabsf(dist) > (radA) + (radB))
			return false;
		else
		{
			float rest = (radA + radB) - dist;
			if (rest < minRestitution)
			{
				minRestitution = rest;
				restitutionN = L;
			}
		}


		//Three axis of B
		L = B.axis[0];
		dist = fabsf(glm::dot(L, T));
		radA = fabsf(glm::dot(L, A.axis[0] * A.radius.x)) + fabsf(glm::dot(L, A.axis[1] * A.radius.y)) + fabsf(glm::dot(L, A.axis[2] * A.radius.z));
		radB = B.radius.x;
		if (fabsf(dist) > (radA) + (radB))
			return false;
		else
		{
			float rest = (radA + radB) - dist;
			if (rest < minRestitution)
			{
				minRestitution = rest;
				restitutionN = L;
			}
		}

		L = B.axis[1];
		dist = fabsf(glm::dot(L, T));
		radA = fabsf(glm::dot(L, A.axis[0] * A.radius.x)) + fabsf(glm::dot(L, A.axis[1] * A.radius.y)) + fabsf(glm::dot(L, A.axis[2] * A.radius.z));
		radB = B.radius.y;
		if (fabsf(dist) > (radA) + (radB))
			return false;
		else
		{
			float rest = (radA + radB) - dist;
			if (rest < minRestitution)
			{
				minRestitution = rest;
				restitutionN = L;
			}
		}

		L = B.axis[2];
		dist = fabsf(glm::dot(L, T));
		radA = fabsf(glm::dot(L, A.axis[0] * A.radius.x)) + fabsf(glm::dot(L, A.axis[1] * A.radius.y)) + fabsf(glm::dot(L, A.axis[2] * A.radius.z));
		radB = B.radius.z;
		if (dist > (radA + radB))
			return false;
		else
		{
			float rest = (radA + radB) - dist;
			if (rest < minRestitution)
			{
				minRestitution = rest;
				restitutionN = L;
			}
		}

		if (CheckAxis(glm::cross(A.axis[0], B.axis[0]), A, B, minRestitution, restitutionN) == -1)
			return false;
		if (CheckAxis(glm::cross(A.axis[0], B.axis[1]), A, B, minRestitution, restitutionN) == -1)
			return false;
		if (CheckAxis(glm::cross(A.axis[0], B.axis[2]), A, B, minRestitution, restitutionN) == -1)
			return false;
		if (CheckAxis(glm::cross(A.axis[1], B.axis[0]), A, B, minRestitution, restitutionN) == -1)
			return false;
		if (CheckAxis(glm::cross(A.axis[1], B.axis[1]), A, B, minRestitution, restitutionN) == -1)
			return false;
		if (CheckAxis(glm::cross(A.axis[1], B.axis[2]), A, B, minRestitution, restitutionN) == -1)
			return false;
		if (CheckAxis(glm::cross(A.axis[2], B.axis[0]), A, B, minRestitution, restitutionN) == -1)
			return false;
		if (CheckAxis(glm::cross(A.axis[2], B.axis[1]), A, B, minRestitution, restitutionN) == -1)
			return false;
		if (CheckAxis(glm::cross(A.axis[2], B.axis[2]), A, B, minRestitution, restitutionN) == -1)
			return false;

		//THERE IS COLLISION
		int sign = (glm::dot(T, restitutionN) > 0) ? 1 : -1;
		restitution = glm::vec4(restitutionN.x, restitutionN.y, restitutionN.z, 0) * (sign * minRestitution);
		return true;
	}


	//Checks given an axis L and two OBB, if axis separates. Returns -1 if overlap, 0 else
	int CheckAxis(glm::vec3 const& L, AuxMath::OBB const& A, AuxMath::OBB const& B, 
		float& minRestitution, glm::vec3& restitutionNormal)
	{
		//Check first that L is not a zero vec (can happen with cross products
		if (fabs(L.x) < EPSILON && fabs(L.y) < EPSILON && fabs(L.z) < EPSILON)
			return 0;
		
		//Get the relevant data
		glm::vec3 T = A.position - B.position;
		float dist = fabsf(glm::dot(L, T));
		float radA = fabsf(glm::dot(L, A.axis[0] * A.radius.x)) + fabsf(glm::dot(L, A.axis[1] * A.radius.y)) + fabsf(glm::dot(L, A.axis[2] * A.radius.z));
		float radB = fabsf(glm::dot(L, B.axis[0] * B.radius.x)) + fabsf(glm::dot(L, B.axis[1] * B.radius.y)) + fabsf(glm::dot(L, B.axis[2] * B.radius.z));
		
		//Do the check
		float restitution = (radA + radB) - dist;
		if (dist > (radA + radB))
			return -1;
		else if (restitution < minRestitution)
		{
			minRestitution = restitution;
			restitutionNormal = L;
		}
		return 0;
	}
}

