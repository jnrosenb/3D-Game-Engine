///HEADER STUFF

#pragma once

///INCLUDES
#include <vector>
#include <assert.h>
#include "../External/Includes/glm/glm.hpp"

//TODO - For not including this here, get 
//OBB out of AST translation unit ***
#include "../SAT.h" 

#include "../Math/Simplices.h"

#include <queue>
#include <functional>


namespace AuxMath
{
	//Used to store both body and world coords of a contact
	//So later, I can easily map both together
	struct BodyWorldPair 
	{
		glm::vec4 body;
		glm::vec4 world;

		BodyWorldPair() : body(glm::vec4(0,0,0,1)),
			world(glm::vec4(0,0,0,1)), valid(true)
		{}

		BodyWorldPair(glm::vec4 const& b, 
			glm::vec4 const& w) : body(b), 
			world(w), valid(true)
		{}

		//FOR NOW, FORCING W COMP TO BE 1
		BodyWorldPair operator+(BodyWorldPair const& rhs) 
		{
			BodyWorldPair sum = {};
			sum.world = this->world + rhs.world;
			sum.world.w = 1.0f;
			sum.body = this->body + rhs.body;
			sum.body.w = 1.0f;
			return sum;
		}

		//FOR NOW, FORCING W COMP TO BE 1
		BodyWorldPair operator-(BodyWorldPair const& rhs) 
		{
			BodyWorldPair sum = {};
			sum.world = this->world - rhs.world;
			sum.world.w = 1.0f;
			sum.body = this->body - rhs.body;
			sum.body.w = 1.0f;
			return sum;
		}

		//FOR NOW, FORCING W COMP TO BE 1
		BodyWorldPair operator*(float rhs) 
		{
			BodyWorldPair res = {};
			res.world = this->world * rhs;
			res.world.w = 1.0f;
			res.body = this->body * rhs;
			res.body.w = 1.0f;
			return res;
		}

		void MarkAsInvalid()
		{
			assert(valid);
			valid = false;
		}

		bool IsValid() 
		{
			return valid;
		}

	private:
		bool valid;
	};


	//This will hold the manifold information. 
	//In progress ***
	struct GJK_Manifold_V1 
	{
		std::vector<BodyWorldPair> ptsA;
		std::vector<BodyWorldPair> ptsB;

		//Lagrange multipliers
		std::vector<float> Lambdas;

		glm::vec3 restitution; //Normal, unit
		float penetrationDepth;
	};


	//Used to being able to, from a minkowski difference 
	//vertex, get the underlying A and B vertices
	//(Maps minkowski to A and B points)
	struct GJK_MinkowskiMap 
	{
		//The minkowski corresponds to the world 
		//sustraction of ptA and ptB
		BodyWorldPair ptA;
		BodyWorldPair ptB;
		glm::vec4 MDiff;
	};


	////////////////////////////////////////////////////////////////////
	//// EPA useful structures                                     /////
	////////////////////////////////////////////////////////////////////
	struct PolyVertex;
	struct PolyEdge;
	struct PolyFace;

	//Just so we can group features together
	struct PolyBase 
	{
		int type;
		bool markedRemove;
		bool removed;

		//REMOVE LATER
		bool initialFeat;

		PolyBase(int tp = 0, bool flag = false) :
			type(tp), markedRemove(flag), removed(false)
		{
			//REMOVE LATER
			initialFeat = false;
		}
	};
	
	// 3*4 + N*4 aprox~ 44 bytes if one vertex touches 8 faces 
	struct PolyVertex : public PolyBase
	{
		glm::vec3 point;
		std::vector<PolyFace*> faces;

		PolyVertex() : PolyBase(0, false), 
			faceCount(0)
		{}
		
		void AddFace(PolyFace *face)
		{
			++faceCount;
			faces.push_back(face);
		}

		void RemoveFace(PolyFace *face)
		{
			std::vector<PolyFace*>::iterator it, toBeRmvd;
			for (it = faces.begin(); it != faces.end(); ++it) 
			{
				if (*it == face)
					toBeRmvd = it;
			}

			faces.erase(toBeRmvd);
		}

		void DecreaseFaceCount() 
		{
			--faceCount;
			if (faceCount == 0)
				markedRemove = true;
		}

		int GetFaceCount() 
		{
			return faceCount;
		}

	private:
		int faceCount;
	};

	// 4*4 = 16 bytes (all pointers) CHECK
	struct PolyEdge : public PolyBase
	{
		std::vector<PolyVertex*> vertices;
		std::vector<PolyFace*> adjacentFaces;
		
		PolyEdge() : PolyBase(1, false)
		{}

		PolyEdge(PolyVertex *a, PolyVertex *b) : PolyBase(1, false)
		{
			vertices.push_back(a);
			vertices.push_back(b);
		}

		bool operator==(PolyEdge const& rhs) 
		{
			return (vertices[0] == rhs.vertices[0] && vertices[1] == rhs.vertices[1]) || 
				(vertices[0] == rhs.vertices[1] && vertices[1] == rhs.vertices[0]);
		}

		void AddAdjacentFace(PolyFace *face) 
		{
			adjacentFaces.push_back(face);
		}

		PolyFace *GetAdjacent(PolyFace *face)
		{
			assert(adjacentFaces.size() == 2);

			if (face == adjacentFaces[0])
				return adjacentFaces[1];
			return adjacentFaces[0];
		}
	}; 
	
	// 3*4 + 3*4 = 24 bytes (all pointers) CHECK
	struct PolyFace : public PolyBase
	{
		std::vector<PolyVertex*> vertices;
		std::vector<PolyEdge*> edges;

		PolyFace() : PolyBase(2, false)
		{}

		PolyFace(PolyVertex *a, PolyVertex *b, PolyVertex *c) : 
			PolyBase(2, false)
		{
			vertices.push_back(a);
			vertices.push_back(b);
			vertices.push_back(c);
		}

		void MarkForRemoval() 
		{
			if (markedRemove == false) 
			{
				markedRemove = true;

				for (PolyVertex *vertex : vertices)
					vertex->DecreaseFaceCount();
			}
		}
	};

	//This struct will be what's ordered by the priority queue
	struct ClosestPack
	{
		PolyBase *feature;
		glm::vec4 closest;
		bool validFlag;
	};

	//Priority functor
	class GlmIsLessThan
	{
	public:
		bool operator()(ClosestPack left, ClosestPack right)
		{
			return (std::pow(left.closest.x, 2) + std::pow(left.closest.y, 2) + std::pow(left.closest.z, 2)) >
				(std::pow(right.closest.x, 2) + std::pow(right.closest.y, 2) + std::pow(right.closest.z, 2));
		};
	};



	//First attempt at method to generate all features 
	//and conections starting from a 4simplex
	void GeneratePolytopeInfoFrom4Simplex(std::vector<glm::vec4> const& simplex,
		std::priority_queue<ClosestPack, std::vector<ClosestPack>, GlmIsLessThan>& queue,
		std::vector<PolyBase*>& features);

	void GeneratePolytopeInfoFrom3Simplex(std::vector<glm::vec4> const& simplex,
		std::priority_queue<ClosestPack, std::vector<ClosestPack>, GlmIsLessThan>& queue,
		std::vector<PolyBase*>& features);


	//Adding a polytope feature to the lists
	void AddPolytopeFeature(std::vector<PolyBase*>& feats,
		std::priority_queue<ClosestPack, std::vector<ClosestPack>, GlmIsLessThan>& queue,
		glm::vec4 const& v1);
	
	void AddPolytopeFeature(std::vector<PolyBase*>& feats,
		std::priority_queue<ClosestPack, std::vector<ClosestPack>, GlmIsLessThan>& queue,
		PolyVertex *v1, PolyVertex *v2);

	void AddPolytopeFeature(std::vector<PolyBase*>& feats,
		std::priority_queue<ClosestPack, std::vector<ClosestPack>, GlmIsLessThan>& queue,
		PolyVertex *v1, PolyVertex *v2, PolyVertex *v3);



	//All related to GJK and EPA solving methods
	struct GJKSolver
	{
		//GJK------------------
		static bool GJK_Intersects(std::vector<GJK_MinkowskiMap>& md_list,
			OBB const& aOBB, OBB const& bOBB,
			std::vector<glm::vec4>& outInfo);

		static BodyWorldPair SupportMap(std::vector<BodyWorldPair> const& set,
			glm::vec3 const& d);

		static void FindClosestPoints(std::vector<GJK_MinkowskiMap> const& md_list,
			AuxMath::FeatureBase *outFeat, std::vector<glm::vec4>& outInfo);

		static void RegisterMinkowskiMapping(std::vector<GJK_MinkowskiMap>& minkowskiList,
			BodyWorldPair const& A, BodyWorldPair const& B, BodyWorldPair const& support);

		//EPA-------------------
		static void EPA(std::vector<GJK_MinkowskiMap>& md_list,
			OBB const& aOBB, OBB const& bOBB,
			std::vector<glm::vec4>& simplex,
			AuxMath::GJK_Manifold_V1& manifold);

		static void EPA_MarkFeaturesForRemoval(glm::vec4 const& support,
			ClosestPack const& closest,
			std::vector<PolyEdge*>& OuterEdges);

		static bool CheckIfFaceVisibleToPoint(PolyFace *face,
			glm::vec4 const& point);

		static void GenerateNewFeatures(std::vector<PolyEdge*> const& OuterEdges, glm::vec4 const& support,
			std::vector<PolyBase*>& features,
			std::priority_queue<ClosestPack, std::vector<ClosestPack>, GlmIsLessThan>& queue);

		static void RemoveFeatures(std::vector<PolyBase*>& feats,
			std::priority_queue<ClosestPack, std::vector<ClosestPack>, GlmIsLessThan>& queue);
		
		static bool FindSupportForEPA(std::vector<GJK_MinkowskiMap>& md_list, 
			std::vector<BodyWorldPair> const& AVertices,
			std::vector<BodyWorldPair> const& BVertices, ClosestPack const& closest,
			BodyWorldPair& support);

		static void EPATerminationRoutine(std::vector<GJK_MinkowskiMap> const& md_list,
			ClosestPack const& closest, int iterations,
			AuxMath::GJK_Manifold_V1& manifold);
	};


	//Method to, from OBB info, get a set of both OBB in world 
	//space and OBB in object space (which is an AABB really)
	void ObjWorldPairsFromOBB(OBB const& obb,
		std::vector<BodyWorldPair>& vertices);
}