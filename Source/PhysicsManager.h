///HEADER

#pragma  once

#include <iostream>
#include "../External/Includes/glm/glm.hpp"
#include <string>
#include <vector>
#include "Physics/GJK.h"

#include <unordered_map>
#include <string>

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

	//Collision resolution
	void CollisionResolutionTest(CollisionContact& contact);
	void SequentialImpulseRoutine(std::vector<CollisionContact> const& contacts);

	//Contact related methods
	glm::vec4 BodyToWorldContact(glm::vec4 const& body, RigidbodyComponent *rgbdy);
	void CheckContactValidity(CollisionContact& contact);
	void AddPersistentContactToManifold(RigidbodyComponent *A, RigidbodyComponent *B, 
		CollisionContact& contact);
	void ChooseFourContacts(CollisionContact const& persistentCt,
		CollisionContact& contact);

private:
	//VBH structure
	AABBTree *vbh;


	//For testing manifold---------------------------------
	std::vector<AuxMath::GJK_Manifold_V1> m_gjk_manifolds;
	Mesh *debugPointMesh;
	Shader *debugShader;
	//For testing manifold---------------------------------


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

	//Attempts at building persistent manifolds (first is the usual vector)
	std::vector<CollisionContact> m_contacts;
	std::unordered_map<std::string, CollisionContact> m_persistentContacts;
};