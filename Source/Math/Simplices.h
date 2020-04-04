#pragma once

#include <vector>
#include <iostream>
#include "../External/Includes/glm/glm.hpp"


#define EPSILON			0.000001f //Thresshold to avoid floating point errors


namespace AuxMath
{
	enum FeatureType 
	{
		POINT = 0,
		LINE,
		TRIANGLE,
		TETRA
	};

	//Base class
	struct FeatureBase
	{
		FeatureType type;

		// FOR FACES AND NORMALS:
		// - CCW for normal (CHECK how assimp stores them)
		//     
		//		Pb--------Pa
		//		--------   -
		//		-----      -
		//		Pc--------Pd
		//
		// - Normal : (Pb - Pa)x(Pc - Pa)

		glm::vec4 pA;
		glm::vec4 pB;
		glm::vec4 pC;
		glm::vec4 pD;

		//Barycentric (at most will use two)
		float alpha, beta;
	};


	////////////////////////////////////
	//// SIMPLEX (methods)          ////
	////////////////////////////////////
	struct Simplex
	{

		static void CorrectConvexHull(std::vector<glm::vec4>& simplexCH,
			FeatureBase *outFeat);


		static void ExpandConvexHull(std::vector<glm::vec4>& simplexCH,
			glm::vec4 const& newPoint);


		//RETURNS 1 if intersects, 0 if not, -1 if error
		static int MinimumNormOnSimplex(std::vector<glm::vec4> const& simplexCH,
			glm::vec4& outPoint, FeatureBase *outFeat);


		///////////////////////////////////////////////
		///////	Simplex Voronoi Tests			///////
		///////////////////////////////////////////////
		static int PointSimplexCase(std::vector<glm::vec4> const& simplexCH,
			glm::vec4& outPoint, FeatureBase *outFeat);

		static int EdgeSimplexCase(std::vector<glm::vec4> const& simplexCH,
			glm::vec4& outPoint, FeatureBase *outFeat);

		static int FaceSimplexCase(std::vector<glm::vec4> const& simplexCH,
			glm::vec4& outPoint, FeatureBase *outFeat);

		static int TetrahedronSimplexCase(std::vector<glm::vec4> const& simplexCH,
			glm::vec4& outPoint, FeatureBase *outFeat);


		///////////////////////////////////////////////
		///////	Minimum norm methods			///////
		///////////////////////////////////////////////
		static unsigned int minimumNormOnPoint(std::vector<glm::vec4> const& points,
			glm::vec4& outPoint, FeatureBase *outFeat);


		static unsigned int minimumNormOnEdge(std::vector<glm::vec4> const& points,
			glm::vec4& outPoint, FeatureBase *outFeat);


		static unsigned int minimumNormOnTriangle(std::vector<glm::vec4> const& points,
			glm::vec4& outPoint, FeatureBase *outFeat);



		///////////////////////////////////////////////
		///////	Other stuff						///////
		///////////////////////////////////////////////
		static bool Contains(std::vector<glm::vec4> const& simplexCH,
			glm::vec4 const& support);

		static bool IsSupportCoplanar(std::vector<glm::vec4> const& simplexCH,
			glm::vec4 const& support);

		static bool DoesPointProjectInsideSimplex(std::vector<glm::vec4> const& simplexCH,
			glm::vec4 const& support);

		static glm::vec4 vec4Cross(glm::vec4 const& A, 
			glm::vec4 const& B);
	};

}
