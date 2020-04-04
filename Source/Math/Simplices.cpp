#pragma once

#include "Simplices.h"

#define EPSILON			0.000001f //Thresshold to avoid floating point errors


namespace AuxMath
{
	////////////////////////////////////
	//// SIMPLEX (methods)          ////
	////////////////////////////////////
	void Simplex::CorrectConvexHull(std::vector<glm::vec4>& simplexCH,
		FeatureBase *outFeat)
	{
		//Given the type of feature, delete the unused 
		//convex hull elements from simplexCH
		if (outFeat->type == TRIANGLE)
		{
			if (simplexCH.size() > 3) 
			{
				simplexCH.clear();
				simplexCH.push_back(outFeat->pA);
				simplexCH.push_back(outFeat->pB);
				simplexCH.push_back(outFeat->pC);
			}
		}
		else if (outFeat->type == LINE)
		{
			if (simplexCH.size() > 2)
			{
				simplexCH.clear();
				simplexCH.push_back(outFeat->pA);
				simplexCH.push_back(outFeat->pB);
			}							   
		}
		else
		{
			if (simplexCH.size() > 1)
			{
				simplexCH.clear();
				simplexCH.push_back(outFeat->pA);
			}
		}
	}


	void Simplex::ExpandConvexHull(std::vector<glm::vec4>& simplexCH,
		glm::vec4 const& newPoint)
	{
		//Expand convex hull elements by adding newPoint to simplexCH
		//Check if any type corrections are needed here
		simplexCH.push_back(newPoint);
	}


	//RETURNS 1 if intersects, 0 if not, -1 if error
	int Simplex::MinimumNormOnSimplex(std::vector<glm::vec4> const& simplexCH,
		glm::vec4& outPoint, FeatureBase *outFeat)
	{
		//Based on number of elements on simplexCH, we 
		//call corresponding voronoi region routine
		switch (simplexCH.size())
		{
		case 1:
			return PointSimplexCase(simplexCH, outPoint, outFeat);
		case 2:
			return EdgeSimplexCase(simplexCH, outPoint, outFeat);
		case 3:
			return FaceSimplexCase(simplexCH, outPoint, outFeat);
		case 4:
			return TetrahedronSimplexCase(simplexCH, outPoint, outFeat);
		default:
			std::cout << "ERROR, convex hull of simplex has " <<
				simplexCH.size() << " elements!" << std::endl;
			return -1;
		};
	}


	///////////////////////////////////////////////
	///////	Simplex Voronoi Tests			///////
	///////////////////////////////////////////////
	int Simplex::PointSimplexCase(std::vector<glm::vec4> const& simplexCH,
		glm::vec4& outPoint, FeatureBase *outFeat)
	{
		//In this case, we know the point itself is min norm
		//Still call this method just to make code consistent
		return minimumNormOnPoint(simplexCH, outPoint, outFeat);
	}


	int Simplex::EdgeSimplexCase(std::vector<glm::vec4> const& simplexCH,
		glm::vec4& outPoint, FeatureBase *outFeat)
	{
		glm::vec4 A = simplexCH[0];
		glm::vec4 B = simplexCH[1];
		glm::vec4 AB = (B - A);

		//Test for vertex 1 on edge
		float dot = glm::dot(-A, AB);
		if (dot < 0.0f)
			return minimumNormOnPoint({ A }, outPoint, outFeat);

		//Test for vertex 2 on edge
		dot = glm::dot(-B, -AB);
		if (dot < 0.0f)
			return minimumNormOnPoint({ B }, outPoint, outFeat);


		//Closest point has to be on voronoi of edge
		return minimumNormOnEdge({A, B}, outPoint, outFeat);
	}


	int Simplex::FaceSimplexCase(std::vector<glm::vec4> const& simplexCH,
		glm::vec4& outPoint, FeatureBase *outFeat)
	{
		//TODO: find out which voronoi region you are in
		glm::vec4 A = simplexCH[0];
		glm::vec4 B = simplexCH[1];
		glm::vec4 C = simplexCH[2];
		glm::vec4 AB = (B - A);
		glm::vec4 BC = (C - B);
		glm::vec4 AC = (C - A);

		//Cache useful dots and stuff
		glm::vec4 n = (vec4Cross(AB, AC));

		//Test for vertex A
		float dot_PA_AB = glm::dot(-A, AB);
		float dot_PA_AC = glm::dot(-A, AC);
		if (dot_PA_AB <= 0.0f && dot_PA_AC <= 0.0f)
			return minimumNormOnPoint({ A }, outPoint, outFeat);

		//Test for vertex B
		float dot_PB_BA = glm::dot(-B, -AB);
		float dot_PB_BC = glm::dot(-B, BC);
		if (dot_PB_BA <= 0.0f && dot_PB_BC <= 0.0f)
			return minimumNormOnPoint({ B }, outPoint, outFeat);

		//Test for vertex C
		float dot_PC_CB = glm::dot(-C, -BC);
		float dot_PC_CA = glm::dot(-C, -AC);
		if (dot_PC_CB <= 0.0f && dot_PC_CA <= 0.0f)
			return minimumNormOnPoint({ C }, outPoint, outFeat);


		//Barycentric areas
		///glm::vec4 P(0);
		///glm::vec4 R = P - n * glm::dot( (P-A), n );
		///R.w = 1.0f;
		///float Area_AB = glm::dot(n, vec4Cross(R - B, A - B));
		///float Area_BC = glm::dot(n, vec4Cross(R - C, B - C));
		///float Area_AC = glm::dot(n, vec4Cross(R - A, C - A));
		
		float Area_AB = glm::dot(n, vec4Cross(A, B));
		float Area_BC = glm::dot(n, vec4Cross(B, C));
		float Area_AC = glm::dot(n, vec4Cross(C, A));
		///float totalArea = Area_AB + Area_BC + Area_AC;

		//Test edge A------B
		if (Area_AB < 0.0f && dot_PA_AB >= 0.0f && dot_PB_BA >= 0.0f)
			return minimumNormOnEdge({ A, B }, outPoint, outFeat);

		//Test edge B------C
		if (Area_BC < 0.0f && dot_PB_BC >= 0.0f && dot_PC_CB >= 0.0f)
			return minimumNormOnEdge({ B, C }, outPoint, outFeat);

		//Test edge C------A
		if (Area_AC < 0.0f && dot_PA_AC >= 0.0f && dot_PC_CA >= 0.0f)
			return minimumNormOnEdge({ C, A }, outPoint, outFeat);


		//Test triangle if none of the previous
		return minimumNormOnTriangle({ A, B, C }, outPoint, outFeat);
	}


	int Simplex::TetrahedronSimplexCase(std::vector<glm::vec4> const& simplexCH,
		glm::vec4& outPoint, FeatureBase *outFeat)
	{
		//TODO: find out which voronoi region you are in
		glm::vec4 q1 = simplexCH[0];
		glm::vec4 q2 = simplexCH[1];
		glm::vec4 q3 = simplexCH[2];
		glm::vec4 q4 = simplexCH[3];
		glm::vec4 e12 = (q2 - q1);
		glm::vec4 e23 = (q3 - q2);
		glm::vec4 e13 = (q3 - q1);
		glm::vec4 e14 = (q4 - q1);
		glm::vec4 e24 = (q4 - q2);
		glm::vec4 e34 = (q4 - q3);

		//Cache useful dots and stuff
		glm::vec4 n123 = (vec4Cross(e12, e13));
		glm::vec4 n142 = (vec4Cross(e14, e12));
		glm::vec4 n243 = (vec4Cross(e24, e23));
		glm::vec4 n134 = (vec4Cross(e13, e14));

		//Test for vertex 1
		float dot_1_12 = glm::dot(-q1, e12);
		float dot_1_13 = glm::dot(-q1, e13);
		float dot_1_14 = glm::dot(-q1, e14);
		if (dot_1_12 < 0.0f && dot_1_13 < 0.0f && dot_1_14 < 0.0f)
			return minimumNormOnPoint({ q1 }, outPoint, outFeat);

		//Test for vertex 2
		float dot_2_21 = glm::dot(-q2, -e12);
		float dot_2_23 = glm::dot(-q2, e23);
		float dot_2_24 = glm::dot(-q2, e24);
		if (dot_2_21 < 0.0f && dot_2_23 < 0.0f && dot_2_24 < 0.0f)
			return minimumNormOnPoint({ q2 }, outPoint, outFeat);

		//Test for vertex 3
		float dot_3_31 = glm::dot(-q3, -e13);
		float dot_3_32 = glm::dot(-q3, -e23);
		float dot_3_34 = glm::dot(-q3, e34);
		if (dot_3_31 < 0.0f && dot_3_32 < 0.0f && dot_3_34 < 0.0f)
			return minimumNormOnPoint({ q3 }, outPoint, outFeat);

		//Test for vertex 4
		float dot_4_41 = glm::dot(-q4, -e14);
		float dot_4_42 = glm::dot(-q4, -e24);
		float dot_4_43 = glm::dot(-q4, -e34);
		if (dot_4_41 < 0.0f && dot_4_42 < 0.0f && dot_4_43 < 0.0f)
			return minimumNormOnPoint({ q4 }, outPoint, outFeat);



		//Test for edge 12 *********
		float dot_12_142 = glm::dot(-q1, vec4Cross(n142, e12));
		float dot_12_123 = glm::dot(-q1, vec4Cross(n123, -e12));
		if (dot_1_12 >= 0.0f && dot_2_21 >= 0.0f && dot_12_123 >= 0.0f && dot_12_142 >= 0.0f)
			return minimumNormOnEdge({ q1, q2 }, outPoint, outFeat);

		//Test for edge 13 *********
		float dot_13_123 = glm::dot(-q1, vec4Cross(n123, e13));
		float dot_13_134 = glm::dot(-q1, vec4Cross(n134, -e13));
		if (dot_1_13 >= 0.0f && dot_3_31 >= 0.0f && dot_13_123 >= 0.0f && dot_13_134 >= 0.0f)
			return minimumNormOnEdge({ q1, q3 }, outPoint, outFeat);

		//Test for edge 14 *********
		float dot_14_142 = glm::dot(-q1, vec4Cross(n142, -e14));
		float dot_14_134 = glm::dot(-q1, vec4Cross(n134, e14));
		if (dot_1_14 >= 0.0f && dot_4_41 >= 0.0f && dot_14_142 >= 0.0f && dot_14_134 >= 0.0f)
			return minimumNormOnEdge({ q1, q4 }, outPoint, outFeat);

		//Test for edge 23 *********
		float dot_23_123 = glm::dot(-q2, vec4Cross(n123, -e23));
		float dot_23_243 = glm::dot(-q2, vec4Cross(n243, e23));
		if (dot_2_23 >= 0.0f && dot_3_32 >= 0.0f && dot_23_123 >= 0.0f && dot_23_243 >= 0.0f)
			return minimumNormOnEdge({ q2, q3 }, outPoint, outFeat);

		//Test for edge 24 *********
		float dot_24_243 = glm::dot(-q2, vec4Cross(n243, -e24));
		float dot_24_142 = glm::dot(-q2, vec4Cross(n142, e24));
		if (dot_2_24 >= 0.0f && dot_4_42 >= 0.0f && dot_24_243 >= 0.0f && dot_24_142 >= 0.0f)
			return minimumNormOnEdge({ q2, q4 }, outPoint, outFeat);

		//Test for edge 34 *********
		float dot_34_134 = glm::dot(-q3, vec4Cross(n134, -e34));
		float dot_34_243 = glm::dot(-q3, vec4Cross(n243, e34));
		if (dot_3_34 >= 0.0f && dot_4_43 >= 0.0f && dot_34_134 >= 0.0f && dot_34_243 >= 0.0f)
			return minimumNormOnEdge({ q3, q4 }, outPoint, outFeat);




		//Testing for face 123 (against point 4)
		float dot_123_4 = glm::dot(n123, -q1)*glm::dot(n123, q4 - q1);
		if (dot_123_4 <= 0.0f)
			return minimumNormOnTriangle({ q1, q2, q3 }, outPoint, outFeat);

		//Testing for face 134 (against point 2)
		float dot_134_2 = glm::dot(n134, -q1)*glm::dot(n134, q2 - q1);
		if (dot_134_2 <= 0.0f)
			return minimumNormOnTriangle({ q1, q3, q4 }, outPoint, outFeat);

		//Testing for face 142 (against point 3)
		float dot_142_3 = glm::dot(n142, -q1)*glm::dot(n142, q3 - q1);
		if (dot_142_3 <= 0.0f)
			return minimumNormOnTriangle({ q1, q4, q2 }, outPoint, outFeat);

		//Testing for face 243 (against point 1)
		float dot_243_1 = glm::dot(n243, -q2)*glm::dot(n243, q1 - q2);
		if (dot_243_1 <= 0.0f)
			return minimumNormOnTriangle({ q2, q4, q3 }, outPoint, outFeat);


		//If it failed all the voronoi tests, then it is inside the tetrahedron
		//Return true, and also the feature that allows to calculate the point
		outFeat->type = TETRA;
		outFeat->pA = q1;
		outFeat->pB = q2;
		outFeat->pC = q3;
		outFeat->pD = q4;
		return 1;
	}



	///////////////////////////////////////////////
	///////	Minimum norm methods			///////
	///////////////////////////////////////////////
	unsigned int Simplex::minimumNormOnPoint(std::vector<glm::vec4> const& points,
		glm::vec4& outPoint, FeatureBase *outFeat)
	{
		outPoint = points[0];
		outFeat->type = POINT;
		outFeat->pA = outPoint;

		//If point is origin, return true
		if (std::abs(outPoint.x) <= EPSILON && std::abs(outPoint.y) <= EPSILON && std::abs(outPoint.z) <= EPSILON)
			return 1;
		return 0;
	}


	unsigned int Simplex::minimumNormOnEdge(std::vector<glm::vec4> const& points,
		glm::vec4& outPoint, FeatureBase *outFeat)
	{
		glm::vec4 A = points[0];
		glm::vec4 B = points[1];
		glm::vec4 d = glm::normalize(B - A);
		outPoint = A + d * glm::dot(-A, d);

		//Set feature as edge
		outFeat->type = LINE;
		outFeat->pA = A;
		outFeat->pB = B;
		outFeat->alpha = 1.0f - glm::length(outPoint-A) / glm::length(B-A); //CHECK THIS///////////////////////****************************************************

		//If point in edge is the origin, return true (INTERSECTION)
		if (std::abs(outPoint.x) <= EPSILON && std::abs(outPoint.y) <= EPSILON && std::abs(outPoint.z) <= EPSILON)
			return 1;
		return 0;
	}


	unsigned int Simplex::minimumNormOnTriangle(std::vector<glm::vec4> const& points,
		glm::vec4& outPoint, FeatureBase *outFeat) 
	{
		//Stuff we need to calculate (extra cals, for now its ok)
		glm::vec4 A = points[0];
		glm::vec4 B = points[1];
		glm::vec4 C = points[2];
		glm::vec4 n = glm::normalize(vec4Cross(B - A, C - A));

		//Barycentric areas (EXTRA CALCS, ALREADY HAVE THEM IN SOME CASES)

		//Barycentric areas ALTERNATIVE
		///glm::vec4 P(0);
		///glm::vec4 R = P - n * glm::dot( (P-A), n );
		///R.w = 1.0f;
		///float Area_AB_1 = glm::dot(n, vec4Cross(R - B, A - B));
		///float Area_BC_1 = glm::dot(n, vec4Cross(R - C, B - C));
		///float Area_AC_1 = glm::dot(n, vec4Cross(R - A, C - A));

		float Area_AB = glm::dot(n, vec4Cross(A, B));
		float Area_BC = glm::dot(n, vec4Cross(B, C));
		float Area_AC = glm::dot(n, vec4Cross(C, A));
		float totalArea = Area_AB + Area_BC + Area_AC;

		//Barycentric coords
		float g = Area_AB / totalArea;
		float b = Area_AC / totalArea;
		float a = 1.0f - b - g;

		//Test triangle if none of the previous
		outFeat->type = TRIANGLE;
		outFeat->pA = A;
		outFeat->pB = B;
		outFeat->pC = C;
		outFeat->alpha = a;
		outFeat->beta = b;

		//Find closest point in face voronoi (triangle)
		outPoint = A*a + B*b + C*g;
		outPoint.w = 1.0f;

		//If point in edge is the origin, return true
		if (std::abs(outPoint.x) <= EPSILON && std::abs(outPoint.y) <= EPSILON && std::abs(outPoint.z) <= EPSILON)
			return 1;
		return 0;
	}



	///////////////////////////////////////////////
	///////	Other stuff						///////
	///////////////////////////////////////////////
	bool Simplex::Contains(std::vector<glm::vec4> const& simplexCH,
		glm::vec4 const& support)
	{
		for (int i = 0; i < simplexCH.size(); ++i) 
			if (support == simplexCH[i])
				return true;
		return false;
	}


	bool Simplex::DoesPointProjectInsideSimplex(std::vector<glm::vec4> const& simplexCH,
		glm::vec4 const& support)
	{
		//This method should return true if the point 
		//projects outside of the triangle (so, if the 
		//support point makes the tetrahedron oblique)

		//Line Case
		if (simplexCH.size() == 2)
		{
			float dot_AP_AB = glm::dot(support - simplexCH[0], simplexCH[1] - simplexCH[0]);
			float dot_BP_BA = glm::dot(support - simplexCH[1], simplexCH[0] - simplexCH[1]);
			if (dot_AP_AB >= 0.0f && dot_BP_BA >= 0.0f)
				return true;
			return false;
		}
		//Triangle case
		else if (simplexCH.size() == 3)
		{
			//Stuff we need to calculate (extra cals, for now its ok)
			glm::vec4 A = simplexCH[0];
			glm::vec4 B = simplexCH[1];
			glm::vec4 C = simplexCH[2];
			glm::vec4 n = glm::normalize(vec4Cross(B - A, C - A));

			//Barycentric areas ALTERNATIVE
			glm::vec4 P(support);
			glm::vec4 R = P - n * glm::dot( (P-A), n );
			R.w = 1.0f;
			float Area_AB_1 = glm::dot(n, vec4Cross(R - B, A - B));
			float Area_BC_1 = glm::dot(n, vec4Cross(R - C, B - C));
			float Area_AC_1 = glm::dot(n, vec4Cross(R - A, C - A));

			if (Area_AB_1 < 0.0f || Area_AC_1 < 0.0f || Area_BC_1 < 0.0f)
				return false;
			return true;
		}
		//Point case
		else
			return true;
	}


	bool Simplex::IsSupportCoplanar(std::vector<glm::vec4> const& simplexCH,
		glm::vec4 const& support)
	{
		if (simplexCH.size() < 3)
			return false;

		glm::vec4 A = simplexCH[0];
		glm::vec4 B = simplexCH[1];
		glm::vec4 C = simplexCH[2];
		glm::vec4 n = glm::normalize(vec4Cross(B - A, C - A));
		float dot = glm::dot(support - A, n);

		if (std::abs(dot) < 0.75f)//0.001f //With 1.0, the error is minimized
			return true;
		return false;
	}

	glm::vec4 Simplex::vec4Cross(glm::vec4 const& A,
		glm::vec4 const& B) 
	{
		glm::vec3 a = glm::vec3(A);
		glm::vec3 b = glm::vec3(B);
		glm::vec3 cross = glm::cross(a, b);
		return {cross.x, cross.y, cross.z, 0.0f};
	}
}
