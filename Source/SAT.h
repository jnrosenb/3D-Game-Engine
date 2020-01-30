//HEADER STUFF

#pragma once

#include "../External/Includes/glm/glm.hpp"


namespace AuxMath
{
	//OBB, for separate axis technique
	struct OBB
	{
		glm::vec3 position; //In world space
		glm::vec3 radius;
		glm::vec3 axis[3];  //In body space, trivial. In world space, columns of R

		OBB(glm::vec4 const& pos, glm::vec3 const& rad, glm::mat4 const& R)
		{
			this->position = glm::vec3(pos.x, pos.y, pos.z);
			this->radius = glm::vec3(rad.x, rad.y, rad.z);

			axis[0] = glm::vec3(R[0].x, R[0].y, R[0].z);
			axis[1] = glm::vec3(R[1].x, R[1].y, R[1].z);
			axis[2] = glm::vec3(R[2].x, R[2].y, R[2].z);
		}
	};

	//SAT to test if two OBB overlap or now
	bool TestOBB_OBB(OBB const& A, OBB const& B, glm::vec4& restitution);

	//Checks given an axis L and two OBB, if axis separates. Returns -1 if overlap, 0 else
	int CheckAxis(glm::vec3 const& L, AuxMath::OBB const& A, AuxMath::OBB const& B,
		float& minRestitution, glm::vec3& restitutionNormal);
}

