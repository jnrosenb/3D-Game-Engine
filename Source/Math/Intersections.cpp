///HEADER

#include "Intersections.h"
#include "../Collider.h"
#include <cmath>
#include <iostream>
	
namespace AuxMath
{
	//////////////////////////////////
	/////     RAYS TO SHAPES     /////
	//////////////////////////////////



	//////////////////////////////////
	/////     SHAPES TO SHAPES   /////
	//////////////////////////////////
	bool AABBToAABBIntersection(glm::vec3 const& center01, glm::vec3 const& radius01, 
		glm::vec3 const& center02, glm::vec3 const& radius02)
	{
		if ((center01 - radius01).x > (center02 + radius02).x || (center01 + radius01).x < (center02 - radius02).x) return false;
		if ((center01 - radius01).y > (center02 + radius02).y || (center01 + radius01).y < (center02 - radius02).y) return false;
		if ((center01 - radius01).z > (center02 + radius02).z || (center01 + radius01).z < (center02 - radius02).z) return false;
		return true;
	}
	
	bool SphereToAABBIntersection(SPHERE const& obj1, AABB const& obj2)
	{
		return AABBToSphereIntersection(obj2, obj1);
	}

	bool AABBToSphereIntersection(AABB const& obj1, SPHERE const& obj2)
	{
		return false;
	}

	bool SphereToSphereIntersection(SPHERE const& obj1, SPHERE const& obj2)
	{
		return std::powf(obj1.center.x - obj2.center.x, 2) + std::powf(obj1.center.y - obj2.center.y, 2) + 
			std::powf(obj1.center.z - obj2.center.z, 2) < std::powf(obj1.radius + obj2.radius, 2);
	}
}

