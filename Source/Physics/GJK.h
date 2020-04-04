///HEADER STUFF

#pragma once

///INCLUDES
#include <vector>
#include "../External/Includes/glm/glm.hpp"

//TODO - For not including this here, get 
//OBB out of AST translation unit ***
#include "../SAT.h" 

#include "../Math/Simplices.h"


namespace AuxMath
{
	struct GJK_Manifold_V1 
	{
		glm::vec4 ptA;
		glm::vec4 ptB;
		glm::vec3 restitution;
	};


	struct GJK_MinkowskiMap 
	{
		glm::vec4 ptA;
		glm::vec4 ptB;
		glm::vec4 MDiff;
	};


	struct GJKSolver
	{
		static bool GJK_Intersects(OBB const& aOBB, OBB const& bOBB, 
			std::vector<glm::vec4>& outInfo);

		static void EPA(std::vector<glm::vec4> const& simplex, 
			AuxMath::GJK_Manifold_V1& manifold);

		static glm::vec4 SupportMap(std::vector<glm::vec4> const& set,
			glm::vec3 const& d);

		static void FindClosestPoints(std::vector<GJK_MinkowskiMap> const& md_list,
			AuxMath::FeatureBase *outFeat, std::vector<glm::vec4>& outInfo);

		static void RegisterMinkowskiMapping(std::vector<GJK_MinkowskiMap>& minkowskiList,
			glm::vec4 const& A, glm::vec4 const& B, glm::vec4 const& support);
	};
}