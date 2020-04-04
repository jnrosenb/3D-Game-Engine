///HEADER STUFF

#include <iostream>
#include "AABBTree.h"
#include "../GameObject.h"

//REMOVE LATER
#include "../Renderer.h"
#include "../Shader.h"
#include "../Meshes/DebugBox.h"
//REMOVE LATER


//TEMPORARY
glm::vec4 hierarchyColor[] = 
{
	glm::vec4(1, 0, 0, 0.25f),
	glm::vec4(0, 1, 0, 0.25f),
	glm::vec4(0, 0, 1, 0.25f),
	glm::vec4(0.4f, 0.4f, 0.4f, 0.25f),
	glm::vec4(1, 0, 1, 0.25f),
	glm::vec4(0, 1, 1, 0.25f),
	glm::vec4(1, 1, 1, 0.25f)
};



AABBTree::AABBTree() : root(nullptr), 
	count(0)
{
	//REMOVE LATER
	DebugDrawSetupStuff();
	//REMOVE LATER

	//ROTATION THRESSHOLD
	rotationThresshold = 0.2f;
}


AABBTree::~AABBTree() 
{
}


void AABBTree::Add(RigidbodyComponent *body)
{
	AABBNode *newNode = new AABBNode(body);
	Add(newNode);
}


//************************************************
void AABBTree::SwapNodes(AABBNode *A, AABBNode *B)
{
	AABBNode *temp = A->parent;
	
	//Rewire parents
	if (temp->left == A)
		temp->left = B;
	else if (temp->right == A)
		temp->right = B;
	if (B->parent->left == B)
		B->parent->left = A;
	else if (B->parent->right == B)
		B->parent->right = A;

	//Rewire nodes
	A->parent = B->parent;
	B->parent = temp;
}


//***********************************************************
void AABBTree::AppendBranch(AABBNode *branch, AABBNode *dest)
{
	//Set dest as the current node and start traversing
	AABBNode *current = dest;
	while (1)
	{
		if (current->isLeaf)
		{
			current = current->CreateInteriorNode(current->parent, current, branch);
			if (current->parent == nullptr)
				root = current;
			else
				//RefitAABBs(current->parent);*********

			break;
		}
		else
		{
			//compare heuristics and choose lowest volume to merge
			AABBNode rightMerge = AABBNode::Merge(*branch, *(current->right));
			AABBNode leftMerge = AABBNode::Merge(*branch, *(current->left));

			if (rightMerge.GetHeuristic() < leftMerge.GetHeuristic())
				current = current->right;
			else
				current = current->left;
		}
	}
}


void AABBTree::Add(AABBNode *n) 
{
	//Add to the dictionary
	m_nodeMap[n->body->GetOwner()->GetId()] = n;

	//First attemp to fill algorithm
	if (root == nullptr) 
	{
		//First element in tree is root. Leaf node
		///AABBNode *node = new AABBNode(n->body);
		///root = node;
		root = n;
	}
	else
	{
		//Set root as the current node and start traversing
		AABBNode *current = root;
		while (1)
		{
			if (current->isLeaf) 
			{
				current = current->CreateInteriorNode(current->parent, current, n);
				if (current->parent == nullptr)
					root = current;
				else
					RefitAABBs(current->parent);

				break;
			}
			else 
			{
				//compare heuristics and choose lowest volume to merge
				AABBNode rightMerge = AABBNode::Merge(*n, *(current->right));
				AABBNode leftMerge = AABBNode::Merge(*n, *(current->left));

				float rightMergeCost = rightMerge.GetHeuristic();
				float leftMergeCost = leftMerge.GetHeuristic();
				if (rightMergeCost < leftMergeCost)
					current = current->right;
				else 
					current = current->left;
			}
		}
	}
}


void AABBTree::RefitAABBs(AABBNode *current) 
{
	do
	{
		AABBNode merged = AABBNode::Merge(*(current->left), *(current->right));
		current->size = merged.size;
		current->center = merged.center;


		/*POTENTIAL FOR ROTATION CHECK-----------------
		float leftHeuristic = current->left->GetHeuristic();
		float rightHeuristic = current->right->GetHeuristic();
		if (leftHeuristic/rightHeuristic < rotationThresshold)
		{
			//ROTATE
			AABBNode *smallest = current->left;
			AABBNode *biggest = current->right;
			ApplyRotationToNodes(smallest, biggest);
		}
		else if (rightHeuristic/leftHeuristic < rotationThresshold)
		{
			//ROTATE
			AABBNode *smallest = current->right;
			AABBNode *biggest = current->left;
			ApplyRotationToNodes(smallest, biggest);
		}
		//---------------------------------------------*/

		
		current = current->parent;
	} 
	while (current != nullptr);
}


AABBNode *AABBTree::RemoveLeaf(AABBNode *leaf)
{
	// Should remove internal parent node (delete it) 
	// Should replace that one by the sibling of the leaf
	// Could return the leaf so it can be reinserted or deleted

	if (leaf->parent == nullptr) 
	{
		//Root case, just set root to null and return it
		root = nullptr;
		return leaf;
	}

	AABBNode *internalParent = leaf->parent;
	AABBNode *newParent = internalParent->parent;

	//Get sibling
	AABBNode *sibling = internalParent->left;
	if (leaf == internalParent->left)
		sibling = internalParent->right;

	//Rewire the pointers for the new parent and sibling
	if (newParent && newParent->right == internalParent)
		newParent->right = sibling;
	else if (newParent && newParent->left == internalParent)
		newParent->left = sibling;
	sibling->parent = newParent;

	//Set new root in special case
	if (newParent == nullptr)
		root = sibling;

	//Rewire for leaf
	leaf->parent = nullptr;
	
	//Delete internal
	delete internalParent;

	//Return leaf
	return leaf;

}


void AABBTree::Update(RigidbodyComponent *body)
{
	if (root == nullptr)
		return;

	auto iter = m_nodeMap.find(body->GetOwner()->GetId());
	if (iter != m_nodeMap.end())
	{
		//Remove and reinsert
		iter->second->UpdateNode();
		AABBNode *leaf = RemoveLeaf(iter->second);
		Add(leaf);
	}
}


//***************************************************************
void AABBTree::MergeNodes(AABBNode *toInsert, AABBNode *receiver)
{
	///First, correct the links at the current position
	//Get current parent (should always exist)
	AABBNode *currentParent = toInsert->parent;

	//Promote sibling to your current parent
	AABBNode *sibling = currentParent->left;
	if (toInsert == currentParent->left)
		sibling = currentParent->right;
	
	//Sibling points to grandpa (SHOULD NEVER BE NULL)
	sibling->parent = currentParent->parent;
	
	//Rewire grandpa to adopt sibling (promoting)
	if (currentParent->parent->right == currentParent) 
		currentParent->parent->right = sibling;
	else if (currentParent->parent->left == currentParent)
		currentParent->parent->left = sibling;

	//Delete current parent
	toInsert->parent = nullptr;
	delete currentParent;

	///Now fix the toInsert node to the other branch
	AppendBranch(toInsert, receiver);
}



AABBNode *AABBTree::FindLeafNode(RigidbodyComponent *body)
{
	auto iter = m_nodeMap.find(body->GetOwner()->GetId());
	if (iter != m_nodeMap.end()) 
	{
		return iter->second;
	}
	return nullptr;
}


void AABBTree::ApplyRotationToNodes(AABBNode* smallest, AABBNode *biggest) 
{
	//We have to break apart the biggest child - FOR NOW, ONLY IF NOT A LEAF
	if (biggest->isLeaf == false)
	{
		AABBNode *tornedLeft = biggest->left;
		AABBNode *tornedRight = biggest->right;

		//Get the smallest child's smallest node, or it if its a leaf
		AABBNode *smallToSwap = smallest;
		if (smallest->isLeaf == false) 
			smallToSwap = (smallest->left->GetHeuristic() < smallest->right->GetHeuristic()) ? smallest->left : smallest->right;

		//Get the biggest child's biggest node
		AABBNode *bigToSwap = (biggest->left->GetHeuristic() > biggest->right->GetHeuristic()) ? biggest->left : biggest->right;

		//INCOMPLETE ANALYSIS
		SwapNodes(smallToSwap, bigToSwap);
	}
}



//////////////////////////////////////
//// SEND DATA TO GHX MGR TO DRAW  ///
//////////////////////////////////////
void AABBTree::Draw(Renderer *renderer)
{
	return;
	RecursiveDraw(renderer, root, 0);
}


void AABBTree::RecursiveDraw(Renderer *renderer, 
	AABBNode *current, int level)
{
	if (current == nullptr)
		return;

	RecursiveDraw(renderer, current->left, level + 1);
	RecursiveDraw(renderer, current->right, level + 1);

	//Draw the middle one
	glm::mat4 model(1);
	model[3] = glm::vec4(current->center.x, current->center.y, current->center.z, 1.0f);
	//Leafs are scaled by less than the internal nodes
	//Not sure why this scaling is needed here CHECK IT OUT
	model[0][0] = (current->isLeaf) ? current->size.x * 1.75f : current->size.x * 2.0f;
	model[1][1] = (current->isLeaf) ? current->size.y * 1.75f : current->size.y * 2.0f;
	model[2][2] = (current->isLeaf) ? current->size.z * 1.75f : current->size.z * 2.0f;
	DrawDebugData data = {};
	data.diffuseColor = hierarchyColor[level];
	data.model = model;
	data.mesh = (current->isLeaf) ? debugMeshLeaf : debugMesh;
	data.shader = debugShader;
	renderer->QueueForDebugDraw(data);
}


void AABBTree::DebugDrawSetupStuff()
{
	glm::vec3 size = {1, 1, 1};
	this->debugMesh = new DebugBox({ 1, 1, 1 });
	this->debugMeshLeaf = new DebugBoxFilled({ 1, 1, 1 });
	
	this->debugShader = new Shader("Solid.vert", "Solid.frag");
	this->debugShader->BindUniformBlock("test_gUBlock", 1);
}