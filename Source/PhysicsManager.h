///HEADER

#pragma  once

#include <iostream>
#include "../External/Includes/glm/glm.hpp"
#include <string>
#include <vector>
#include "Physics/GJK.h"

class RigidbodyComponent;
class AABBTree;
class AABBNode;
class Renderer;
//Manifold test
class Mesh;
class Shader;


struct CollisionContact 
{
	RigidbodyComponent *rbdyA;
	RigidbodyComponent *rbdyB;
	glm::vec3 p0;
	//More stuff
	AuxMath::GJK_Manifold_V1 manifold;
};



class PhysicsManager 
{
public:
	PhysicsManager();
	virtual ~PhysicsManager();

	void PhysicsUpdate(float dt);
	void DebugDraw(Renderer *renderer);

	void RegisterRigidbody(RigidbodyComponent* rgbdy);
	std::vector<RigidbodyComponent*> const& GetRigidbodies() const;

	//FOR TESTING PURPOSES
	void publicToggleVBH() { onEndOfFrameToggleVBH = true; }
	//FOR TESTING PURPOSES

private:
	bool CheckCollision(RigidbodyComponent *A, RigidbodyComponent *B);
	void RecursiveTreeCheck(AABBNode *current, AABBNode *starting);

private:
	//VBH structure
	AABBTree *vbh;

	//For testing manifold
	std::vector<AuxMath::GJK_Manifold_V1> m_gjk_manifolds;
	Mesh *debugPointMesh;
	Shader *debugShader;

	//FOR TESTING PURPOSES
	bool USEBVH = false, onEndOfFrameToggleVBH = false;
	void toggleVBH() 
	{ 
		USEBVH = !USEBVH;
		std::cout << ">>> USING TREE: " << USEBVH << std::endl;
	}
	//FOR TESTING PURPOSES
	
	std::vector<RigidbodyComponent*> m_rigidbodies;
	float timeAccumulator;

	std::vector<CollisionContact> m_contacts;
};