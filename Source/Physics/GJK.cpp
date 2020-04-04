///HEADER STUFF

#include <iostream>
#include <assert.h>
#include "GJK.h"
#include <limits>


namespace AuxMath
{
	//EPA
	void GJKSolver::EPA(std::vector<glm::vec4> const& simplex,
		AuxMath::GJK_Manifold_V1& manifold) 
	{
		//Deal with cases in which simplex has less than 4 points
		if (simplex.size() < 4) 
		{
		}

		//Main scenario
		//TODO
	}


	//GJK intersection
	bool GJKSolver::GJK_Intersects(OBB const& aOBB, OBB const& bOBB,
		std::vector<glm::vec4>& outInfo)
	{
		//We need to create a list of vertices which will be used for the GJK routine
		//Both in world space, otherwise it doesnt work
		std::vector<GJK_MinkowskiMap> md_list(0);
		std::vector<glm::vec4> AVertices(0);
		std::vector<glm::vec4> BVertices(0);
		AuxMath::VerticesFromOBB(aOBB, AVertices);
		AuxMath::VerticesFromOBB(bOBB, BVertices);

		//Simplex convex hull, stores the vertices of simplex
		std::vector<glm::vec4> simplexCH;

		//Pick random sample from Minkowski diff
		glm::vec4 smp = (AVertices[0] - BVertices[0]);
		glm::vec3 dir(-smp.x, -smp.y, -smp.z);
		glm::vec4 supportA0 = GJKSolver::SupportMap(AVertices, dir);
		glm::vec4 supportB0 = GJKSolver::SupportMap(BVertices, -dir);
		glm::vec4 first = supportA0 - supportB0;
		first.w = 1.0f;
		RegisterMinkowskiMapping(md_list, supportA0, supportB0, first);
		simplexCH.push_back(first);
		
		//Now we have a starting simplex, begin iterative algorithm
		while (1)
		{
			//1- Find closest point to origin on current simplexConvexHull
			glm::vec4 outPoint(0);
			AuxMath::FeatureBase outFeat = {0};
			int containsOrigin = AuxMath::Simplex::MinimumNormOnSimplex(simplexCH,
				outPoint, &outFeat);

			//INTERSECTION! If containsOrigin, means simplex has origin. Return true
			if (containsOrigin == 1)
			{
				//Here we would need to return simplex and maybe feature for info
				for (int i = 0; i < simplexCH.size(); ++i)
					outInfo.push_back(simplexCH[i]);
				return true;
			}
			else if (containsOrigin == 0)
			{
				//Given feature containing closest point, update simplex convex hull
				//so it only contains points needed to compute the new closest point
				AuxMath::Simplex::CorrectConvexHull(simplexCH, &outFeat);

				//Now, use support mapping on minkowski diff samples 
				//to find next point to expand convex hull
				glm::vec3 d(-outPoint.x, -outPoint.y, -outPoint.z);
				glm::vec4 supportA = GJKSolver::SupportMap(AVertices, d);
				glm::vec4 supportB = GJKSolver::SupportMap(BVertices, -d);
				glm::vec4 support = supportA - supportB;
				support.w = 1.0f;
				RegisterMinkowskiMapping(md_list, supportA, supportB, support);

				//NON INTERSECTION (CLOSES DISTANCE)
				//Check if support point is already in convexHull (termination) or
				//Check if new support point is further than current closest
				if (AuxMath::Simplex::Contains(simplexCH, support) || 
					AuxMath::Simplex::IsSupportCoplanar(simplexCH, support) )/// ||
					///!AuxMath::Simplex::DoesPointProjectInsideSimplex(simplexCH, support) )
				{
					FindClosestPoints(md_list, &outFeat, outInfo);
					return false;
				}

				//If not closest point, add support to simplex convex hull
				AuxMath::Simplex::ExpandConvexHull(simplexCH, support);
			}
			else 
			{
				std::cout << "Error on GJK. closestPoint returned: " << containsOrigin << std::endl;
				assert(false);
				return false;
			}
		}
	}



	//INEFFICIENT***
	glm::vec4 GJKSolver::SupportMap(std::vector<glm::vec4> const& set,
		glm::vec3 const& d)
	{
		glm::vec4 support(0);
		float maxDot = std::numeric_limits<float>::lowest();
		for (int i = 0; i < set.size(); ++i)
		{
			glm::vec3 sample = static_cast<glm::vec3>(set[i]);
			float dot = glm::dot(d, sample);
			if (dot > maxDot)
			{
				maxDot = dot;
				support = glm::vec4(sample.x, sample.y, sample.z, 1.0f);
			}
		}
		return support;
	}


	void GJKSolver::FindClosestPoints(std::vector<GJK_MinkowskiMap> const& md_list, 
		AuxMath::FeatureBase *outFeat, std::vector<glm::vec4>& outInfo)
	{
		if (outFeat->type == POINT) 
		{
			for (auto map : md_list) 
			{
				if (map.MDiff == outFeat->pA) 
				{
					outInfo.push_back(map.ptA);
					outInfo.push_back(map.ptB);
					return;
				}
			}
		}
		else if (outFeat->type == LINE)
		{
			GJK_MinkowskiMap map1 = {};
			GJK_MinkowskiMap map2 = {};
			unsigned found1 = 0;
			unsigned found2 = 0;
			
			//Find the correct minkowski map for both vertices on the line segment
			//This is so we can access the points on the original bodies, not in the MD
			for (auto map : md_list)
			{
				if (map.MDiff == outFeat->pA)
				{
					map1 = map;
					found1 = 1;
				}
				else if (map.MDiff == outFeat->pB)
				{
					map2 = map;
					found2 = 1;
				}

				if (found1 && found2)
					break;
			}

			//Use barycentrics to get the resulting point on each body
			float alpha = outFeat->alpha;
			float beta = 1.0f - alpha;
			outInfo.push_back((map1.ptA * alpha) + (map2.ptA * beta));
			outInfo.push_back((map1.ptB * alpha) + (map2.ptB * beta));
		}
		else if (outFeat->type == TRIANGLE)
		{
			GJK_MinkowskiMap map1 = {};
			GJK_MinkowskiMap map2 = {};
			GJK_MinkowskiMap map3 = {};
			unsigned found1 = 0;
			unsigned found2 = 0;
			unsigned found3 = 0;

			//Find the correct minkowski map for both vertices on the line segment
			//This is so we can access the points on the original bodies, not in the MD
			for (auto map : md_list)
			{
				if (map.MDiff == outFeat->pA)
				{
					map1 = map;
					found1 = 1;
				}
				else if (map.MDiff == outFeat->pB)
				{
					map2 = map;
					found2 = 1;
				}
				else if (map.MDiff == outFeat->pC)
				{
					map3 = map;
					found3 = 1;
				}

				if (found1 && found2 && found3)
					break;
			}

			//Use barycentrics to get the resulting point on each body
			float alpha = outFeat->alpha;
			float beta = outFeat->beta;
			float gamma = 1.0f - alpha - beta;
			outInfo.push_back( (map1.ptA * alpha) + (map2.ptA * beta) + (map3.ptA * gamma));
			outInfo.push_back( (map1.ptB * alpha) + (map2.ptB * beta) + (map3.ptB * gamma));
		}
	}
	
	
	void GJKSolver::RegisterMinkowskiMapping(std::vector<GJK_MinkowskiMap>& minkowskiList,
		glm::vec4 const& A, glm::vec4 const& B, glm::vec4 const& support)
	{
		AuxMath::GJK_MinkowskiMap map = {};
		map.ptA = A;
		map.ptB = B;
		map.MDiff = support;
		minkowskiList.push_back(map);
	}
}