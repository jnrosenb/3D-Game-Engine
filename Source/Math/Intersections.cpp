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
	bool AABBToAABBIntersection(AABB const& obj1, AABB const& obj2)
	{
		if (obj1.min.x > obj2.max.x || obj1.max.x < obj2.min.x ) return false;
		if (obj1.min.y > obj2.max.y || obj1.max.y < obj2.min.y) return false;
		if (obj1.min.z > obj2.max.z || obj1.max.z < obj2.min.z) return false;
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

