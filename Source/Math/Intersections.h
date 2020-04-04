#pragma once

#include <vector>
#include "../External/Includes/glm/glm.hpp"
#include "../Shapes.h"

namespace AuxMath
{
	bool AABBToAABBIntersection(glm::vec3 const& center01, glm::vec3 const& radius01,
		glm::vec3 const& center02, glm::vec3 const& radius02);

	bool SphereToAABBIntersection(SPHERE const& obj1, AABB const& obj2);

	bool AABBToSphereIntersection(AABB const& obj1, SPHERE const& obj2);

	bool SphereToSphereIntersection(SPHERE const& obj1, SPHERE const& obj2);
}
