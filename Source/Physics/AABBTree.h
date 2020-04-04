///HEADER STUFF

#pragma once

///INCLUDES
#include "../../External/Includes/glm/glm.hpp"
#include <vector>
#include <unordered_map>
#include <assert.h>
#include "../RigidbodyComponent.h"


//REMOVE LATER
class Renderer;
class Shader;
class Mesh;
//REMOVE LATER


struct AABBNode
{
	//Size data
	glm::vec3 center;
	glm::vec3 size;
	bool isLeaf;

	//Later use union or something
	RigidbodyComponent *body;
	AABBNode *parent;
	AABBNode *left;
	AABBNode *right;


	//CTOR
	AABBNode() 
	{
		//TODO
	}


	//Create a leaf node from a single rigidbody
	AABBNode(RigidbodyComponent *rhs)
	{
		//Get AABB data and set as leaf
		center = rhs->GetPositionEstimate() + glm::vec3(rhs->GetOBBCenterOffsetScaled());
		size = rhs->GetAABBRadiusFromOBB();
		isLeaf = true;

		//In case of leafs, left will contain the pointer to the body
		body = rhs;
		left = nullptr;
		right = nullptr;
		parent = nullptr;
	}


	void UpdateNode()
	{
		if (isLeaf)
		{
			//Update AABB data and set as leaf
			center = body->GetPositionEstimate() + glm::vec3(body->GetOBBCenterOffsetScaled());
			size = body->GetAABBRadiusFromOBB(); 
		}
	}
	

	//Merge operation overrides addition
	static AABBNode Merge(AABBNode const& lhs, AABBNode const& rhs)
	{
		AABBNode result;

		float minX = std::fminf(lhs.center.x - lhs.size.x, rhs.center.x - rhs.size.x);
		float maxX = std::fmaxf(lhs.center.x + lhs.size.x, rhs.center.x + rhs.size.x);
		result.center.x = 0.5f * (minX + maxX);
		result.size.x = 0.5f * std::fabsf(maxX - minX);

		float minY = std::fminf(lhs.center.y - lhs.size.y, rhs.center.y - rhs.size.y);
		float maxY = std::fmaxf(lhs.center.y + lhs.size.y, rhs.center.y + rhs.size.y);
		result.center.y = 0.5f * (minY + maxY);
		result.size.y = 0.5f * std::fabsf(maxY - minY);

		float minZ = std::fminf(lhs.center.z - lhs.size.z, rhs.center.z - rhs.size.z);
		float maxZ = std::fmaxf(lhs.center.z + lhs.size.z, rhs.center.z + rhs.size.z);
		result.center.z = 0.5f * (minZ + maxZ);
		result.size.z = 0.5f * std::fabsf(maxZ - minZ);

		return result;
	}


	//Create an internal node parent of two nodes
	AABBNode *CreateInteriorNode(AABBNode *parent, AABBNode *originalNode, 
		AABBNode *newSibling)
	{
		AABBNode *newNode = new AABBNode();

		//Make originalNode's parent point to newNode
		if (originalNode->parent) 
		{
			if (originalNode->parent->right == originalNode)
				originalNode->parent->right = newNode;
			else if (originalNode->parent->left == originalNode)
				originalNode->parent->left = newNode;
			else assert(false);
		}

		originalNode->parent = newNode;
		newSibling->parent = newNode;

		newNode->parent = parent;
		newNode->right = newSibling;
		newNode->left = originalNode;

		newNode->isLeaf = false;
		newNode->body = nullptr;

		//TEMPORARY MEASURE (inefficient)
		AABBNode merged = AABBNode::Merge(*originalNode, *newSibling);
		newNode->size = merged.size;
		newNode->center = merged.center;

		return newNode;
	}


	//Gets node heuristic value to compare
	float GetHeuristic()
	{
		return (2.0f * size.x) * (2.0f * size.y) * (2.0f * size.z);
	}
};


class AABBTree 
{

public:
	AABBTree();
	virtual ~AABBTree();

	AABBNode *GetRoot() const { return root; }
	AABBNode *FindLeafNode(RigidbodyComponent *body);

	void Add(AABBNode *n);
	void AppendBranch(AABBNode *branch, AABBNode *dest);
	void Add(RigidbodyComponent *body);

	AABBNode *RemoveLeaf(AABBNode *leaf);

	void RefitAABBs(AABBNode *current);

	void Update(RigidbodyComponent *body);
	

	//REMOVE LATER
	void Draw(Renderer *renderer);
	void RecursiveDraw(Renderer *renderer,
			AABBNode *current, int level);
	//REMOVE LATER

private:
	//REMOVE LATER
	void DebugDrawSetupStuff();
	//REMOVE LATER

	void MergeNodes(AABBNode *toInsert, AABBNode *receiver);
	
	void SwapNodes(AABBNode *A, AABBNode *B);
	void ApplyRotationToNodes(AABBNode* smallest, AABBNode *biggest);

private:
	float rotationThresshold;
	AABBNode *root;
	size_t count;

	//Experiment
	std::unordered_map<int, AABBNode*> m_nodeMap;

	//REMOVE LATER
	Mesh *debugMesh;
	Mesh *debugMeshLeaf;
	Shader *debugShader;
	//REMOVE LATER
};