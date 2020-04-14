///HEADER STUFF

#include <iostream>
#include <assert.h>
#include "GJK.h"
#include <limits>

//Experiment, to use closest point functions
#include "../Math/Simplices.h"

#define	MAX_ITER		25	//Maximum number of EPA iterations


namespace AuxMath
{
	//EPA
	void GJKSolver::EPA(std::vector<GJK_MinkowskiMap>& md_list,
		OBB const& aOBB, OBB const& bOBB,
		std::vector<glm::vec4>& simplex,
		AuxMath::GJK_Manifold_V1& manifold) 
	{
		//We need this to access support points on MD
		std::vector<BodyWorldPair> AVertices(0);
		std::vector<BodyWorldPair> BVertices(0);
		AuxMath::ObjWorldPairsFromOBB(aOBB, AVertices);
		AuxMath::ObjWorldPairsFromOBB(bOBB, BVertices);
		
		//Used for exiting the loop
		glm::vec4 previousClosest(0);

		//I'm going to need a hash structure to easily find my features
		std::priority_queue<ClosestPack, std::vector<ClosestPack>, GlmIsLessThan> queue;
		std::vector<PolyBase*> features;
		features.reserve(128);

		///Deal with cases in which simplex has less than 4 points
		if (simplex.size() == 1)
		{
			//If simplex is point, then return that as the closest feature
			glm::vec4 p01 = simplex[0];
			for (auto map : md_list)
			{
				if (map.MDiff == p01)
				{
					//Out info----------------------
					manifold.ptsA.push_back(map.ptA);
					manifold.ptsB.push_back(map.ptB);
					return;
				}
			}
		}
		else if (simplex.size() == 2) 
		{
			float sqrLen0 = glm::dot(simplex[0], simplex[0]);
			float sqrLen1 = glm::dot(simplex[1], simplex[1]);
			glm::vec4 pclosest = simplex[0];
			if (sqrLen1 < sqrLen0)
				pclosest = simplex[1];

			for (auto map : md_list)
			{
				if (map.MDiff == pclosest)
				{
					//Out info----------------------
					manifold.ptsA.push_back(map.ptA);
					manifold.ptsB.push_back(map.ptB);
					return;
				}
			}
		}
		else if (simplex.size() == 3) 
		{
			//Gotta create a double tetrahedron and initialize it
			glm::vec3 AB = static_cast<glm::vec3>(simplex[1] - simplex[0]);
			glm::vec3 AC = static_cast<glm::vec3>(simplex[2] - simplex[0]);
			glm::vec3 n = glm::cross(AB, AC);

			//Find support on MinkowskiDiff using that point (as in GJK)
			BodyWorldPair supportA = GJKSolver::SupportMap(AVertices, n);
			BodyWorldPair supportB = GJKSolver::SupportMap(BVertices, -n);
			BodyWorldPair support = supportA - supportB;
			support.world.w = 1.0f;
			//We need to register this new point for later potential use
			RegisterMinkowskiMapping(md_list, supportA, supportB, support);
			simplex.push_back(support.world);

			supportA = GJKSolver::SupportMap(AVertices, -n);
			supportB = GJKSolver::SupportMap(BVertices, n);
			support = supportA - supportB; 
			support.world.w = 1.0f;
			//We need to register this new point for later potential use
			RegisterMinkowskiMapping(md_list, supportA, supportB, support);
			simplex.push_back(support.world);

			GeneratePolytopeInfoFrom3Simplex(simplex, queue, features);
		}
		else 
			GeneratePolytopeInfoFrom4Simplex(simplex, queue, features);


		//MAIN LOOP
		int iterations = 0;
		while (true)
		{
			//std::cout << "Nums of iterations: " << iterations++ << std::endl;

			///Get closest item from queue (only valid ones)
			ClosestPack closest = queue.top();
			queue.pop();
			bool invalid = closest.feature->markedRemove
				|| closest.feature->type != 2;
			while (invalid)
			{
				//WEIRD CASE********************
				assert(queue.size() > 0);

				closest = queue.top();
				queue.pop();
				invalid = closest.feature->markedRemove;
			}


			//Support finding
			BodyWorldPair support = {};
			bool isSupportCoplanar = FindSupportForEPA(md_list, AVertices, BVertices, closest, support);
			if (isSupportCoplanar) 
			{
				//Support point found is not further from origin than current feature (is coplanar)
				AuxMath::GJKSolver::EPATerminationRoutine(md_list, closest, 
					iterations, manifold);///////////////////////////////////*************
				break;
			}


			//Gotta check if support is already in polytope. If it is, we dont need to keep going
			for (PolyBase *base : features) 
			{
				float epsilon = 0.0001f;

				if (base->type == 0) 
				{
					PolyVertex *vtx = static_cast<PolyVertex*>(base);
					if ((std::abs(vtx->point.x - support.world.x) < epsilon &&
						std::abs(vtx->point.y - support.world.y) < epsilon &&
						std::abs(vtx->point.z - support.world.z) < epsilon) &&
						vtx->markedRemove == false)
					{
						//Support point found is already added to the polytope
						AuxMath::GJKSolver::EPATerminationRoutine(md_list, closest, 
							iterations, manifold);///////////////////////////////////*************
						break;
					}
				}
			}

			//DEBUGGING (vertex not deleted)
			/// for (PolyBase* base : features)
			/// {
			/// 	if (base->type == 0)
			/// 	{
			/// 		PolyVertex *vtx = static_cast<PolyVertex*>(base);
			/// 
			/// 		if (vtx->markedRemove == false && vtx->GetFaceCount() == 0)
			/// 			std::cout << "";
			/// 	}
			/// }

			///Now we have the support, we need to do the following steps
			//1- Algorithm to start eliminating features (faces, edges, and at the end, points)
			std::vector<PolyEdge*> OuterEdges;
			AuxMath::GJKSolver::EPA_MarkFeaturesForRemoval(support.world, closest, OuterEdges);


			//DEBUGGING (vertex not deleted)
			/// for (PolyBase* base : features) 
			/// {
			/// 	if (base->type == 0) 
			/// 	{
			/// 		PolyVertex *vtx = static_cast<PolyVertex*>(base);
			/// 
			/// 		if (vtx->markedRemove == false && vtx->GetFaceCount() == 0) 
			/// 			std::cout << "";
			/// 	}
			/// }


			//CHECK IF THE VERTICES ARE BEING DELETED PROPERLY
			//Be careful that any feature marked for deletion is erased from feature list and queue
			RemoveFeatures(features, queue); /// IN PROGRESS


			//DEBUGGING (vertex not deleted)
			/// for (PolyBase* base : features)
			/// {
			/// 	if (base->type == 0)
			/// 	{
			/// 		PolyVertex *vtx = static_cast<PolyVertex*>(base);
			/// 
			/// 		if (vtx->markedRemove == false && vtx->GetFaceCount() == 0)
			/// 			std::cout << "";
			/// 	}
			/// }


			//Create the new features in between, which will be one vertex, and many faces/edges. 
			//When creating, they have to be added to the queue
			GenerateNewFeatures(OuterEdges, support.world, features, queue); /// NOT FINISHED YET


			//DEBUGGING (vertex not deleted)
			/// for (PolyBase* base : features)
			/// {
			/// 	if (base->type == 0)
			/// 	{
			/// 		PolyVertex *vtx = static_cast<PolyVertex*>(base);
			/// 
			/// 		if (vtx->markedRemove == false && vtx->GetFaceCount() == 0)
			/// 			std::cout << "";
			/// 	}
			/// }


			//End condition check - Filling out manifold information
			float diff = std::abs(glm::length(closest.closest) - glm::length(previousClosest));
			if (diff < 0.01f || iterations > MAX_ITER)
			{
				AuxMath::GJKSolver::EPATerminationRoutine(md_list, closest, 
					iterations, manifold); ///////////////////////////////////*************
				break;
			}
			previousClosest = closest.closest;
		}

		//Clean up all the information (for now) 
		//INEFICIENT (Need not to be using new and delete here)
		for (AuxMath::PolyBase *node : features)
			delete node;
	}



	//////////////////////////////////////////////////////////////////////////////////////////*************
	void GJKSolver::EPATerminationRoutine(std::vector<GJK_MinkowskiMap> const& md_list, 
		ClosestPack const& closest, int iterations,
		AuxMath::GJK_Manifold_V1& manifold)
	{
		//Printing
		///std::cout << "Closest feature distance  : " << closest.closest.x << ", " << closest.closest.y << ", " << closest.closest.z << std::endl;
		///std::cout << "Nums of iterations: " << iterations++ << std::endl;
		///std::cout << "--------------------------" << std::endl;

		//For now, I'm only allowing closest feat to be a face
		//This will be changed in a bit**********************
		if (closest.feature->type == 0) 
		{
			PolyVertex *outFeat = static_cast<PolyVertex*>(closest.feature);
			glm::vec4 pA = glm::vec4(outFeat->point.x, outFeat->point.y, outFeat->point.z, 1.0f);
			for (auto map : md_list)
			{
				if (map.MDiff == pA)
				{
					//Out info-------------
					manifold.ptsA.push_back(map.ptA);
					manifold.ptsB.push_back(map.ptB);
					return;
				}
			}
		}
		else if (closest.feature->type == 1)
		{
			PolyEdge *outFeat = static_cast<PolyEdge*>(closest.feature);
			glm::vec4 pA = glm::vec4(outFeat->vertices[0]->point.x, outFeat->vertices[0]->point.y, outFeat->vertices[0]->point.z, 1.0f);
			glm::vec4 pB = glm::vec4(outFeat->vertices[1]->point.x, outFeat->vertices[1]->point.y, outFeat->vertices[1]->point.z, 1.0f);

			GJK_MinkowskiMap map1 = {};
			GJK_MinkowskiMap map2 = {};
			unsigned found1 = 0;
			unsigned found2 = 0;

			//Find the correct minkowski map for both vertices on the line segment
			//This is so we can access the points on the original bodies, not in the MD
			for (auto map : md_list)
			{
				if (map.MDiff == pA)
				{
					map1 = map;
					found1 = 1;
				}
				else if (map.MDiff == pB)
				{
					map2 = map;
					found2 = 1;
				}

				if (found1 && found2)
					break;
			}

			//Use barycentrics to get the resulting point on each body
			glm::vec4 d = glm::normalize(pB - pA);
			glm::vec4 outPoint = pA + d * glm::dot(-pA, d);
			float alpha = 1.0f - glm::length(outPoint - pA) / glm::length(pB - pA);
			float beta = 1.0f - alpha;

			//Out info----------------------
			manifold.ptsA.push_back((map1.ptA * alpha) + (map2.ptA * beta));
			manifold.ptsB.push_back((map1.ptB * alpha) + (map2.ptB * beta));
		}
		else 
		{
			PolyFace *outFeat = static_cast<PolyFace*>(closest.feature);
			glm::vec4 A = glm::vec4(outFeat->vertices[0]->point.x, outFeat->vertices[0]->point.y, outFeat->vertices[0]->point.z, 1.0f);
			glm::vec4 B = glm::vec4(outFeat->vertices[1]->point.x, outFeat->vertices[1]->point.y, outFeat->vertices[1]->point.z, 1.0f);
			glm::vec4 C = glm::vec4(outFeat->vertices[2]->point.x, outFeat->vertices[2]->point.y, outFeat->vertices[2]->point.z, 1.0f);

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
				if (map.MDiff == A)
				{
					map1 = map;
					found1 = 1;
				}
				else if (map.MDiff == B)
				{
					map2 = map;
					found2 = 1;
				}
				else if (map.MDiff == C)
				{
					map3 = map;
					found3 = 1;
				}

				if (found1 && found2 && found3)
					break;
			}

			//Barycentric areas (SHOULD BE CACHED)***************************(above comment)
			glm::vec4 n = AuxMath::Simplex::vec4Cross(B-A, C-A);
			glm::vec4 P(0);
			glm::vec4 R = P - n * glm::dot( (P-A), n );
			R.w = 1.0f;
			float Area_AB = glm::dot(n, AuxMath::Simplex::vec4Cross(R - B, A - B));
			float Area_BC = glm::dot(n, AuxMath::Simplex::vec4Cross(R - C, B - C));
			float Area_AC = glm::dot(n, AuxMath::Simplex::vec4Cross(R - A, C - A));
			float totalArea = Area_AB + Area_AC + Area_BC;
			float gamma = Area_AB / totalArea;
			float beta = Area_AC / totalArea;
			float alpha = 1.0f - gamma - beta;

			//Out info----------------------
			manifold.ptsA.push_back( (map1.ptA * alpha) + (map2.ptA * beta) + (map3.ptA * gamma));
			manifold.ptsB.push_back( (map1.ptB * alpha) + (map2.ptB * beta) + (map3.ptB * gamma));
		}
	}
	//////////////////////////////////////////////////////////////////////////////////////////*************




	//Finds support point on a given direction. Also, returns boolean showing if
	//support found is coplanar or not
	bool GJKSolver::FindSupportForEPA(std::vector<GJK_MinkowskiMap>& md_list,
		std::vector<BodyWorldPair> const& AVertices,
		std::vector<BodyWorldPair> const& BVertices, ClosestPack const& closest, 
		BodyWorldPair& support)
	{
		//Find support on MinkowskiDiff using that point (as in GJK)
		glm::vec4 outPoint = closest.closest;
		glm::vec3 d(outPoint.x, outPoint.y, outPoint.z);
		BodyWorldPair supportA = GJKSolver::SupportMap(AVertices, d);
		BodyWorldPair supportB = GJKSolver::SupportMap(BVertices, -d);
		support = supportA - supportB; 
		support.world.w = 1.0f;

		//We need to register this new point for later potential use
		RegisterMinkowskiMapping(md_list, supportA, supportB, support);

		//Check if support is coplanar to current closest feature
		bool isCoplanar = false;
		if (closest.feature->type == 0) 
		{
			//SHOULD NEVER ENTER HERE
			PolyVertex *vtx = static_cast<PolyVertex*>(closest.feature);
			//TODO
		}
		else if(closest.feature->type == 1)
		{
			//SHOULD NEVER ENTER HERE
			PolyEdge *edge = static_cast<PolyEdge*>(closest.feature);
			//TODO
		}
		else
		{
			PolyFace *face = static_cast<PolyFace*>(closest.feature);
			
			std::vector<glm::vec4> vec;
			vec.push_back({ face->vertices[0]->point.x, face->vertices[0]->point.y, face->vertices[0]->point.z, 1.0f });
			vec.push_back({ face->vertices[1]->point.x, face->vertices[1]->point.y, face->vertices[1]->point.z, 1.0f });
			vec.push_back({ face->vertices[2]->point.x, face->vertices[2]->point.y, face->vertices[2]->point.z, 1.0f });
			isCoplanar = AuxMath::Simplex::IsSupportCoplanar(vec, support.world);
		}

		return isCoplanar;
	}

	
	
	//Epa, see which features need to be marked for removal
	void GJKSolver::EPA_MarkFeaturesForRemoval(glm::vec4 const& support,
		ClosestPack const& closest,
		std::vector<PolyEdge*>& OuterEdges) 
	{
		//Setting up the "stack" that will be used for the faces
		std::vector<PolyFace*> stack;

		//CHECK FOR ERROR INPUT
		if (closest.feature->markedRemove || closest.feature->removed)
			std::cout << "";

		//we need to, starting from a face, start a Deep First Search on the face-edge relation
		PolyFace *current = nullptr;
		if (closest.feature->type == 0)
		{
			PolyVertex* vtx = static_cast<PolyVertex*>(closest.feature);
			
			//current = vtx->faces[0];
			for (PolyFace *face : vtx->faces)
			{
				if (CheckIfFaceVisibleToPoint(face, support))
				{		
					current = face;
					break;
				}
			}

			//CHECK FOR ERROR INPUT
			if (current->markedRemove || current->removed)
				std::cout << "";
		}
		else if (closest.feature->type == 1)
		{
			PolyEdge* edge = static_cast<PolyEdge*>(closest.feature);
			
			//current = edge->adjacentFaces[0];
			for (PolyFace *face : edge->adjacentFaces)
			{
				if (CheckIfFaceVisibleToPoint(face, support))
				{
					current = face;
					break;
				}
			}

			//CHECK FOR ERROR INPUT
			if (current->markedRemove || current->removed)
				std::cout << "";
		}
		else 
		{
			current = static_cast<PolyFace*>(closest.feature);
			if (CheckIfFaceVisibleToPoint(current, support) == false)
				std::cout << "";
		}


		//DEBUGGING*************************
		std::vector<PolyEdge*> EdgesToBeDeleted;

		//Debugging
		if (current == nullptr)
			assert(false);

		//Push first face to stack
		stack.push_back(current);

		//Now, begin DFS algorithm (need a stack)
		while (stack.size() > 0) 
		{
			//Get face, and mark it for remove
			current = stack.back();
			stack.pop_back();

			//If this was already visited, then we skip to next element of stack
			//Better than this would be to not allow insertion into stack of an already visited face
			//Temporary measure
			if (current->markedRemove)
				continue;
			current->MarkForRemoval();

			//For each edge, check if the adjacent face is visible to the support
			for (auto edge : current->edges)
			{
				//DEBUGGING / ERROR
				//Apparently there is something wrong with the checkIfVisible function
				//Sometimes, it may throw that a face marked for removal is not visible
				//Which makes the edge of that face be both on outer edge and marked as removed
				//This is a patch solution
				if (edge->markedRemove)
					continue;

				PolyFace *adjacent = edge->GetAdjacent(current);
				bool isVisible = CheckIfFaceVisibleToPoint(adjacent, support);

				//If it is, jump to that face, mark the edge as removed, and repeat
				if (isVisible)
				{
					//If the face is already marked for deletion, means we already visited it
					//Still, gotta mark the edge as for removal
					if (adjacent->markedRemove)
					{
						//Mark edge as toBeRemoved
						edge->markedRemove = true;
						
						//DEBUGGING*************************
						EdgesToBeDeleted.push_back(edge);

						//DEBUGGING  -If it breaks here, wrong! Means edge to be 
						//removed has an adjacent face not marked for deletion
						for (PolyFace *f : edge->adjacentFaces)
							if (f->markedRemove == false)
								std::cout << "";
					}
					else
					{
						//Push adjacent face to the stack
						stack.push_back(adjacent);

						//Mark edge as toBeRemoved
						edge->markedRemove = true;

						//DEBUGGING*************************
						EdgesToBeDeleted.push_back(edge);
					}
				}
				else 
				{
					/// FACE IS NOT VISIBLE
					//Handle outer ring of edges and vertices stuff
					bool contained = false;
					for (PolyEdge *e : OuterEdges)
					{
						if (e == edge)
						{
							contained = true;
							break;
						}
					}


					//DEBUGGING
					if (edge->adjacentFaces[0]->markedRemove == false &&
						edge->adjacentFaces[1]->markedRemove == false)
						std::cout << "";


					if (contained == false)
						OuterEdges.push_back(edge);
				}
			}
		}


		//DEBUGGING  -If it breaks here, wrong! Means edge to be 
		//removed has an adjacent face not marked for deletion
		for (PolyEdge *edge : EdgesToBeDeleted)
			for (PolyFace *f : edge->adjacentFaces)
				if (f->markedRemove == false)
					std::cout << "";

		//DEBUGGING
		for (PolyEdge *asd : OuterEdges) 
		{
			if (asd->markedRemove || asd->removed)
				std::cout << "";
		}
	}


	bool GJKSolver::CheckIfFaceVisibleToPoint(PolyFace *face,
		glm::vec4 const& point) 
	{
		////////////////////////////////////////////////
		////   glm::vec4 n123 = (vec4Cross(e12, e13));
		////   glm::vec4 n142 = (vec4Cross(e14, e12));
		////   glm::vec4 n243 = (vec4Cross(e24, e23));
		////   glm::vec4 n134 = (vec4Cross(e13, e14));
		////////////////////////////////////////////////

		//Get face normal
		glm::vec3 AB = face->vertices[1]->point - face->vertices[0]->point;
		glm::vec3 AC = face->vertices[2]->point - face->vertices[0]->point;
		glm::vec3 n = glm::cross(AB, AC);

		//Get dir from first vertex to support
		glm::vec3 pt = glm::vec3( point.x, point.y, point.z ) - face->vertices[0]->point;

		return (glm::dot(n, pt) >= 0.0f);
	}


	void GJKSolver::GenerateNewFeatures(std::vector<PolyEdge*> const& OuterEdges, glm::vec4 const& support,
		std::vector<PolyBase*>& feats,
		std::priority_queue<ClosestPack, std::vector<ClosestPack>, GlmIsLessThan>& queue) 
	{
		//WEIRD CASE***************************************************************************//////////////////
		if (OuterEdges.size() == 0)
			return;

		//DEBUGGING
		for (PolyEdge *edge : OuterEdges)
			if (edge->adjacentFaces.size() > 1)
				std::cout << "";



		//Create and add new vertex into the feature stuff
		PolyVertex *point = new PolyVertex();
		point->point = { support.x, support.y, support.z };
		feats.push_back(point);
		///------------
		ClosestPack cell = {};
		cell.closest = support;
		cell.feature = point;
		cell.validFlag = 1;
		queue.push(cell);
		
		//Have a special temp vector to store new edges. 
		//This will be used to avoid iterating through all feats when creating the faces
		std::vector<PolyEdge*> tempEdgeContainer;

		//TODO - Fix issue with duplicates edges on OuterRing************************

		//For each edge, create the support edges
		for (PolyEdge *edge : OuterEdges)
		{
			//TEMPORARY MEASURE*************************************************************************
			if (edge->markedRemove)
				continue;

			//This index is just to get both vertices on an edge
			for (int i = 0; i < 2; ++i)
			{
				//Check if the edge to be created exists**********************************************************
				bool alreadyCreated = false;
				for (PolyEdge *e : tempEdgeContainer)
				{
					if (PolyEdge(edge->vertices[i], point) == *e)
					{
						alreadyCreated = true;
						break;
					}
				}

				//Create the edges if they have not been created yet
				if (alreadyCreated == false)
				{
					PolyVertex *v1 = edge->vertices[i];
					PolyVertex *v2 = point;
					PolyEdge *newEdge = new PolyEdge(v1, v2);
					feats.push_back(newEdge);
					tempEdgeContainer.push_back(newEdge);
					///--------------
					ClosestPack cell = {};
					AuxMath::Simplex::minimumNormOnEdge({ {v1->point.x, v1->point.y, v1->point.z, 1.0f},
						{v2->point.x, v2->point.y, v2->point.z, 1.0f} }, cell.closest, nullptr);
					cell.feature = newEdge;
					cell.validFlag = 1;
					queue.push(cell);
				}
			}
		}

		//TODO - Fix issue with duplicates edges on OuterRing************************
		//Otherwise, here we will have a duplicate face
		//After all that is done, create the faces
		for (PolyEdge *edge : OuterEdges) 
		{
			//TEMPORARY MEASURE
			if (edge->markedRemove)
				continue;

			//Create feature and push it into vertices
			PolyVertex *v1 = edge->vertices[0];
			PolyVertex *v2 = point;
			PolyVertex *v3 = edge->vertices[1];
			

			//NEW
			PolyFace *face = nullptr;
			glm::vec3 n = glm::cross(v2->point - v1->point, v3->point - v1->point); // E12 x E13
			glm::vec3 E_1_Or = -v1->point;
			if (glm::dot(n, E_1_Or) < 0.0f)
				//Normal is pointing away from origin. Its good
				face = new PolyFace(v1, v2, v3);
			else
				//Normal is pointing towards origin. So we need to turn it	
				face = new PolyFace(v1, v3, v2);
			feats.push_back(face);
			assert(face != nullptr);
			
			//OLD
			/// PolyFace *face = new PolyFace(v1, v3, v2); //OLD - 123
			/// feats.push_back(face);


			//Finding inmediately the closest point to origin, and storing it on heap
			ClosestPack cell = {};
			AuxMath::Simplex::minimumNormOnTriangle({ {v1->point.x, v1->point.y, v1->point.z, 1.0f},
				{v2->point.x, v2->point.y, v2->point.z, 1.0f}, {v3->point.x, v3->point.y, v3->point.z, 1.0f} },
				cell.closest, nullptr);
			cell.feature = face;
			cell.validFlag = 1;
			queue.push(cell);

			//Face pointer for vertex
			v1->AddFace(face);
			v2->AddFace(face);
			v3->AddFace(face);

			//THIS SHOULD NOT BE HAPPENING
			if (edge->adjacentFaces.size() > 1)
				std::cout << "";

			//Face pointer for edge
			edge->AddAdjacentFace(face);
			face->edges.push_back(edge);
			for (PolyEdge *eddie : tempEdgeContainer)
			{			
				if (eddie->vertices[0] == v1 && eddie->vertices[1] == v2 ||
					eddie->vertices[0] == v2 && eddie->vertices[1] == v1)
				{
					eddie->AddAdjacentFace(face);
					face->edges.push_back(eddie);
				}
				else if (eddie->vertices[0] == v2 && eddie->vertices[1] == v3 ||
					eddie->vertices[0] == v3 && eddie->vertices[1] == v2)
				{
					eddie->AddAdjacentFace(face);
					face->edges.push_back(eddie);
				}
				else if (eddie->vertices[0] == v3 && eddie->vertices[1] == v1 ||
					eddie->vertices[0] == v1 && eddie->vertices[1] == v3)
				{
					eddie->AddAdjacentFace(face);
					face->edges.push_back(eddie);
				}

				//THIS SHOULD NOT BE HAPPENING
				if (eddie->adjacentFaces.size() > 2)
					std::cout << "";
			}

			std::cout << "";
		}

		//Debugging
		if (point->markedRemove == false && point->GetFaceCount() == 0)
			std::cout << "";


		//WEIRD*************************************************************///////////////////
		if (queue.size() > 500)
			return;
	}


	void GJKSolver::RemoveFeatures(std::vector<PolyBase*>& feats,
		std::priority_queue<ClosestPack, std::vector<ClosestPack>, GlmIsLessThan>& queue) 
	{
		/// Somehow also remove from queue
		// -For now, I'll assume this is not necessary

		//DEBUGGING - EDGES WITH REMOVED FACES
		/// for (PolyBase* base : feats)
		/// {
		/// 	if (base->type == 1)
		/// 	{
		/// 		PolyEdge *edge = static_cast<PolyEdge*>(base);
		/// 		for (PolyFace *f : edge->adjacentFaces)
		/// 			if (f->markedRemove && f->removed)
		/// 				assert(false);
		/// 	}
		/// }

		///Iterate through the feats and delete the ones that are marked for removal
		std::vector<int> removalIndices(0);
		for (int i = 0; i < feats.size(); ++i)
		{
			PolyBase *base = feats[i];
			if (base->markedRemove && !base->removed)
			{
				removalIndices.push_back(i);
				if (base->type == 0) 
				{
					///Removing a Vertex
					//In principle, if a vertex is removed its because 
					//no feature exists that holds it, so I should be 
					//ok not looking for anything to remove

					//DEBUGGING
					PolyVertex *vtx = static_cast<PolyVertex*>(base);
					for (PolyFace *f : vtx->faces)
						if (f->markedRemove == false)
							std::cout << "";
				}
				else if (base->type == 1) 
				{
					///Removing an Edge
					//Gotta alert the faces
					PolyEdge *edge = static_cast<PolyEdge*>(base);
					for (PolyFace *f : edge->adjacentFaces) 
					{
						//DEBUGGING  -If it breaks here, wrong! Means edge to be 
						//removed has an adjacent face not marked for deletion
						if (f->markedRemove == false)
								std::cout << "";
						/// if (f->edges[0] == edge)
						/// 	f->edges[0] = nullptr;
						/// else if (f->edges[1] == edge)
						/// 	f->edges[1] = nullptr;
						/// else if (f->edges[2] == edge)
						/// 	f->edges[2] = nullptr;
					}
				}
				else if (base->type == 2)
				{
					///Removing a Face
					//Gotta alert the vertices and the edges
					PolyFace *face = static_cast<PolyFace*>(base);
					for (PolyEdge *e: face->edges)
					{
						if (e->adjacentFaces.size() == 2)
						{
							//Here we have to actually erase from the face container
							PolyFace *other = (e->adjacentFaces[0] == face) ? e->adjacentFaces[1] : e->adjacentFaces[0];
							e->adjacentFaces.clear();
							e->AddAdjacentFace(other);
						}
						else if (e->adjacentFaces.size() == 1)
						{
							e->adjacentFaces.clear();

							//DEBUGGING
							if (e->markedRemove == false)
								assert(false);
						}
						else
							assert(false);
					}

					//Make vertices remove ref to this face, since it can cause errors
					//In later stages
					for (PolyVertex *vtx : face->vertices)
						vtx->RemoveFace(face);
				}
				
				//Mark it definitely as removed
				base->removed = true;
			}
		}

		///Actually remove the elements now
		// -For now, I'll assume this is not necessary


		//DEBUGGING - EDGES WITH REMOVED FACES
		/// for (PolyBase* base : feats)
		/// {
		/// 	if (base->type == 1)
		/// 	{
		/// 		PolyEdge *edge = static_cast<PolyEdge*>(base);
		/// 		for (PolyFace *f : edge->adjacentFaces)
		/// 			if (f->markedRemove && f->removed)
		/// 				assert(false);
		/// 	}
		/// }
	}



	///////////////////////////////////////////////////
	////////       GJK intersection             ///////
 	///////////////////////////////////////////////////
	bool GJKSolver::GJK_Intersects(std::vector<GJK_MinkowskiMap>& md_list,
		OBB const& aOBB, OBB const& bOBB,
		std::vector<glm::vec4>& outInfo)
	{
		//We need to create a list of vertices which will be used for the GJK routine
		//Both in world space, otherwise it doesnt work
		std::vector<BodyWorldPair> AVertices(0);
		std::vector<BodyWorldPair> BVertices(0);
		AuxMath::ObjWorldPairsFromOBB(aOBB, AVertices);
		AuxMath::ObjWorldPairsFromOBB(bOBB, BVertices);

		//Simplex convex hull, stores the vertices of simplex
		std::vector<glm::vec4> simplexCH;

		//Pick random sample from Minkowski diff
		glm::vec4 smp = (AVertices[0].world - BVertices[0].world);
		glm::vec3 dir(-smp.x, -smp.y, -smp.z);
		BodyWorldPair supportA0 = GJKSolver::SupportMap(AVertices, dir);
		BodyWorldPair supportB0 = GJKSolver::SupportMap(BVertices, -dir);
		BodyWorldPair first = supportA0 - supportB0;
		///first.w = 1.0f;
		RegisterMinkowskiMapping(md_list, supportA0, supportB0, first);
		simplexCH.push_back(first.world);
		
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
				BodyWorldPair supportA = GJKSolver::SupportMap(AVertices, d);
				BodyWorldPair supportB = GJKSolver::SupportMap(BVertices, -d);
				BodyWorldPair support = supportA - supportB;
				support.world.w = 1.0f;
				RegisterMinkowskiMapping(md_list, supportA, supportB, support);

				//Fast exit TEST------------------------
				float doppo = glm::dot(-d, glm::vec3(support.world.x, support.world.y, support.world.z));
				if (doppo >= -0.1f)//0.0f)//0.001f)
					return false;
				//Fast exit TEST------------------------

				//NON INTERSECTION (CLOSES DISTANCE)
				//Check if support point is already in convexHull (termination) or
				//Check if new support point is further than current closest
				if (AuxMath::Simplex::Contains(simplexCH, support.world) || 
					AuxMath::Simplex::IsSupportCoplanar(simplexCH, support.world) )/// ||
					///!AuxMath::Simplex::DoesPointProjectInsideSimplex(simplexCH, support) )
				{
					FindClosestPoints(md_list, &outFeat, outInfo);
					return false;
				}

				//If not closest point, add support to simplex convex hull
				AuxMath::Simplex::ExpandConvexHull(simplexCH, support.world);
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
	BodyWorldPair GJKSolver::SupportMap(std::vector<BodyWorldPair> const& set,
		glm::vec3 const& d)
	{
		BodyWorldPair support = {};
		float maxDot = std::numeric_limits<float>::lowest();
		for (int i = 0; i < set.size(); ++i)
		{
			//Comparison against d is done in world, so get world and cast it to vec3
			glm::vec3 sample = static_cast<glm::vec3>(set[i].world);
			float dot = glm::dot(d, sample);
			if (dot > maxDot)
			{
				maxDot = dot;
				support = set[i];
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
					outInfo.push_back(map.ptA.world);
					outInfo.push_back(map.ptB.world);
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
			outInfo.push_back(((map1.ptA * alpha) + (map2.ptA * beta)).world);
			outInfo.push_back(((map1.ptB * alpha) + (map2.ptB * beta)).world);
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
			outInfo.push_back( ((map1.ptA * alpha) + (map2.ptA * beta) + (map3.ptA * gamma)).world);
			outInfo.push_back( ((map1.ptB * alpha) + (map2.ptB * beta) + (map3.ptB * gamma)).world);
		}
	}

	
	void GJKSolver::RegisterMinkowskiMapping(std::vector<GJK_MinkowskiMap>& minkowskiList,
		BodyWorldPair const& A, BodyWorldPair const& B, BodyWorldPair const& support)
	{
		AuxMath::GJK_MinkowskiMap map = {};
		map.ptA = A;
		map.ptB = B;
		map.MDiff = support.world;
		minkowskiList.push_back(map);
	}



	////////////////////////////////////
	//// EPA SUPPORT FUNCTIONS    //////
	////////////////////////////////////
	void GeneratePolytopeInfoFrom4Simplex(std::vector<glm::vec4> const& simplex,
		std::priority_queue<ClosestPack, std::vector<ClosestPack>, GlmIsLessThan>& queue,
		std::vector<PolyBase*>& features)
	{
		//First step, create vertex information
		AddPolytopeFeature(features, queue, simplex[0]);
		AddPolytopeFeature(features, queue, simplex[1]);
		AddPolytopeFeature(features, queue, simplex[2]);
		AddPolytopeFeature(features, queue, simplex[3]);
		 
		PolyVertex *v0 = static_cast<PolyVertex*>(features[0]);
		PolyVertex *v1 = static_cast<PolyVertex*>(features[1]);
		PolyVertex *v2 = static_cast<PolyVertex*>(features[2]);
		PolyVertex *v3 = static_cast<PolyVertex*>(features[3]);

		//Second step, create edge information
		AddPolytopeFeature(features, queue, v0, v1);
		AddPolytopeFeature(features, queue, v1, v2);
		AddPolytopeFeature(features, queue, v0, v2);
		AddPolytopeFeature(features, queue, v0, v3);
		AddPolytopeFeature(features, queue, v1, v3);
		AddPolytopeFeature(features, queue, v2, v3);

		//Attempt to make outer pointing normals/////
		glm::vec3 n = glm::cross(v1->point - v0->point, v2->point - v0->point); // E01 x E02
		glm::vec3 E_0_3 = v3->point - v0->point;
		if (glm::dot(n, E_0_3) < 0.0f) 
		{
			//Normal is pointing away from origin. Its good
			AddPolytopeFeature(features, queue, v0, v1, v2);
			AddPolytopeFeature(features, queue, v0, v3, v1);
			AddPolytopeFeature(features, queue, v1, v3, v2);
			AddPolytopeFeature(features, queue, v0, v2, v3);
		}
		else 
		{
			//Normal is pointing towards origin. So we need to turn it			
			AddPolytopeFeature(features, queue, v0, v2, v1);
			AddPolytopeFeature(features, queue, v0, v1, v3);
			AddPolytopeFeature(features, queue, v1, v2, v3);
			AddPolytopeFeature(features, queue, v0, v3, v2);
		}
	}


	void GeneratePolytopeInfoFrom3Simplex(std::vector<glm::vec4> const& simplex,
		std::priority_queue<ClosestPack, std::vector<ClosestPack>, GlmIsLessThan>& queue,
		std::vector<PolyBase*>& features)
	{
		//First step, create vertex information
		AddPolytopeFeature(features, queue, simplex[0]);
		AddPolytopeFeature(features, queue, simplex[1]);
		AddPolytopeFeature(features, queue, simplex[2]);
		//Two extreme points---------------
		AddPolytopeFeature(features, queue, simplex[3]);
		AddPolytopeFeature(features, queue, simplex[4]);

		PolyVertex *v0 = static_cast<PolyVertex*>(features[0]);
		PolyVertex *v1 = static_cast<PolyVertex*>(features[1]);
		PolyVertex *v2 = static_cast<PolyVertex*>(features[2]);
		//Two extreme points-----------------
		PolyVertex *v3 = static_cast<PolyVertex*>(features[3]);
		PolyVertex *v4 = static_cast<PolyVertex*>(features[4]);

		//Second step, create edge information
		AddPolytopeFeature(features, queue, v0, v1);
		AddPolytopeFeature(features, queue, v1, v2);
		AddPolytopeFeature(features, queue, v0, v2);
		//To one extreme--------------
		AddPolytopeFeature(features, queue, v0, v3);
		AddPolytopeFeature(features, queue, v1, v3);
		AddPolytopeFeature(features, queue, v2, v3);
		//To other extreme--------------
		AddPolytopeFeature(features, queue, v0, v4);
		AddPolytopeFeature(features, queue, v1, v4);
		AddPolytopeFeature(features, queue, v2, v4);

		//This part not sure how to make it***********
		//Towards extreme vertex 1 (normal pointing)
		AddPolytopeFeature(features, queue, v0, v1, v3);
		AddPolytopeFeature(features, queue, v1, v2, v3);
		AddPolytopeFeature(features, queue, v0, v3, v2);
		//Towards extreme vertex 2
		AddPolytopeFeature(features, queue, v0, v4, v1);
		AddPolytopeFeature(features, queue, v1, v4, v2);
		AddPolytopeFeature(features, queue, v0, v2, v4);
	}


	/////////////////////////////////////////////////////////////
	//// Functions to add polytope features into the vectors ////
	/////////////////////////////////////////////////////////////
	void AddPolytopeFeature(std::vector<PolyBase*>& feats,
		std::priority_queue<ClosestPack, std::vector<ClosestPack>, GlmIsLessThan>& queue,
		glm::vec4 const& v1)
	{
		//Create feature and push it into vertices
		PolyVertex *point = new PolyVertex();
		point->point = { v1.x, v1.y, v1.z };
		feats.push_back(point);

		//REMOVE LATER*************************************
		point->initialFeat = true;

		///	2- Finding inmediately the closest point to origin, and storing it on heap
		ClosestPack cell = {};
		cell.closest = v1;
		cell.feature = point;
		cell.validFlag = 1;
		queue.push(cell);

		///	(there has to be an easy way to find these and remove them when features goes bye bye)
		//TODO
	}

	void AddPolytopeFeature(std::vector<PolyBase*>& feats,
		std::priority_queue<ClosestPack, std::vector<ClosestPack>, GlmIsLessThan>& queue,
		PolyVertex *v1, PolyVertex *v2)
	{
		///Create feature and push it into vertices
		PolyEdge *edge = new PolyEdge();
		edge->vertices.push_back(v1);
		edge->vertices.push_back(v2);
		feats.push_back(edge);

		//REMOVE LATER*************************************
		edge->initialFeat = true;

		///	1- Doing all conections needed with other features
		//Apparently not needed for edges

		///	2- Finding inmediately the closest point to origin, and storing it on heap
		ClosestPack cell = {};
		AuxMath::Simplex::minimumNormOnEdge({ {v1->point.x, v1->point.y, v1->point.z, 1.0f}, 
			{v2->point.x, v2->point.y, v2->point.z, 1.0f} }, cell.closest, nullptr);
		cell.feature = edge;
		cell.validFlag = 1;
		queue.push(cell);
	}

	void AddPolytopeFeature(std::vector<PolyBase*>& feats,
		std::priority_queue<ClosestPack, std::vector<ClosestPack>, GlmIsLessThan>& queue,
		PolyVertex *v1, PolyVertex *v2, PolyVertex *v3)
	{
		///Create feature and push it into vertices
		PolyFace *face = new PolyFace(v1, v2, v3);
		feats.push_back(face);

		//REMOVE LATER*************************************
		face->initialFeat = true;

		///	1- Finding inmediately the closest point to origin, and storing it on heap
		ClosestPack cell = {};
		AuxMath::Simplex::minimumNormOnTriangle({{v1->point.x, v1->point.y, v1->point.z, 1.0f},
			{v2->point.x, v2->point.y, v2->point.z, 1.0f}, {v3->point.x, v3->point.y, v3->point.z, 1.0f}}, 
			cell.closest, nullptr);
		cell.feature = face;
		cell.validFlag = 1;
		queue.push(cell);

		///	2- Doing all conections needed with other features
		// 1.1- Face pointer for vertex
		v1->AddFace(face);
		v2->AddFace(face);
		v3->AddFace(face);

		// 2.2- Face pointer for edge
		for (PolyBase *base : feats) 
		{
			//Edge
			if (base->type == 1) 
			{
				PolyEdge *edge = static_cast<PolyEdge*>(base);

				if (edge->vertices[0] == v1 && edge->vertices[1] == v2 ||
					edge->vertices[0] == v2 && edge->vertices[1] == v1)
				{
					edge->AddAdjacentFace(face);
					face->edges.push_back(edge);
				}
				else if (edge->vertices[0] == v2 && edge->vertices[1] == v3 ||
					edge->vertices[0] == v3 && edge->vertices[1] == v2)
				{
					edge->AddAdjacentFace(face);
					face->edges.push_back(edge);
				}
				else if (edge->vertices[0] == v3 && edge->vertices[1] == v1 ||
					edge->vertices[0] == v1 && edge->vertices[1] == v3)
				{
					edge->AddAdjacentFace(face);
					face->edges.push_back(edge);
				}


				//DEBUGGING
				if (edge->adjacentFaces.size() > 2)
					std::cout << "";
			}
		}
	}


	//Method to, from OBB info, get a set of both OBB in world 
	//space and OBB in object space (which is an AABB really)
	void ObjWorldPairsFromOBB(OBB const& obb,
		std::vector<BodyWorldPair>& vertices)
	{
		//Get relevant OBB info (world space)
		glm::vec4 pos(obb.position.x, obb.position.y, obb.position.z, 1.0f);
		glm::vec4 rad(obb.radius.x, obb.radius.y, obb.radius.z, 1.0f);
		glm::vec4 axis[3] = {
			glm::vec4(obb.axis[0].x, obb.axis[0].y, obb.axis[0].z, 0.0f),
			glm::vec4(obb.axis[1].x, obb.axis[1].y, obb.axis[1].z, 0.0f),
			glm::vec4(obb.axis[2].x, obb.axis[2].y, obb.axis[2].z, 0.0f)
		};

		//Build the contacts, with both body and world components
		BodyWorldPair p = {};
		glm::vec4 worldX = { 1, 0, 0, 0 };
		glm::vec4 worldY = { 0, 1, 0, 0 };
		glm::vec4 worldZ = { 0, 0, 1, 0 };

		p.world = pos + (axis[0] * rad.x) + (axis[1] * rad.y) + (axis[2] * rad.z);
		p.body = +(worldX) + (worldY) + (worldZ);
		vertices.push_back(p);

		p.world = pos + (axis[0] * rad.x) + (axis[1] * rad.y) - (axis[2] * rad.z);
		p.body = +(worldX) + (worldY) - (worldZ);
		vertices.push_back(p);

		p.world = pos + (axis[0] * rad.x) - (axis[1] * rad.y) + (axis[2] * rad.z);
		p.body = +(worldX) - (worldY) + (worldZ);
		vertices.push_back(p);

		p.world = pos + (axis[0] * rad.x) - (axis[1] * rad.y) - (axis[2] * rad.z);
		p.body = +(worldX) - (worldY) - (worldZ);
		vertices.push_back(p);

		p.world = pos - (axis[0] * rad.x) + (axis[1] * rad.y) + (axis[2] * rad.z);
		p.body = -(worldX) + (worldY) + (worldZ);
		vertices.push_back(p);

		p.world = pos - (axis[0] * rad.x) + (axis[1] * rad.y) - (axis[2] * rad.z);
		p.body = -(worldX) + (worldY) - (worldZ);
		vertices.push_back(p);

		p.world = pos - (axis[0] * rad.x) - (axis[1] * rad.y) + (axis[2] * rad.z);
		p.body = -(worldX) - (worldY) + (worldZ);
		vertices.push_back(p);

		p.world = pos - (axis[0] * rad.x) - (axis[1] * rad.y) - (axis[2] * rad.z);
		p.body = -(worldX) - (worldY) - (worldZ);
		vertices.push_back(p);

		//Old way, only had for world space
		/// vertices.push_back(pos + (axis[0] * rad.x) + (axis[1] * rad.y) + (axis[2] * rad.z));
		/// vertices.push_back(pos + (axis[0] * rad.x) + (axis[1] * rad.y) - (axis[2] * rad.z));
		/// vertices.push_back(pos + (axis[0] * rad.x) - (axis[1] * rad.y) + (axis[2] * rad.z));
		/// vertices.push_back(pos + (axis[0] * rad.x) - (axis[1] * rad.y) - (axis[2] * rad.z));
		/// vertices.push_back(pos - (axis[0] * rad.x) + (axis[1] * rad.y) + (axis[2] * rad.z));
		/// vertices.push_back(pos - (axis[0] * rad.x) + (axis[1] * rad.y) - (axis[2] * rad.z));
		/// vertices.push_back(pos - (axis[0] * rad.x) - (axis[1] * rad.y) + (axis[2] * rad.z));
		/// vertices.push_back(pos - (axis[0] * rad.x) - (axis[1] * rad.y) - (axis[2] * rad.z));
	}
}