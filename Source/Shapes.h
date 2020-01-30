///HEADER STUFF

///INCLUDES
#pragma once
#include "../External/Includes/glm/glm.hpp"


struct AABB
{
	glm::vec3 min;
	glm::vec3 max;
};

struct SPHERE
{
	glm::vec3 center;
	float radius;
};