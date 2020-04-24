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


//This will hold two vec3
//So one of this correspond to six elements
//Being used on constraint solver, for holding a vec6 per body
struct LinearAngularPair
{
	glm::vec4 linear;
	glm::vec4 angular;

	LinearAngularPair operator+(LinearAngularPair const& rhs) const
	{
		LinearAngularPair result = {};
		result.linear = this->linear + rhs.linear;
		result.angular = this->angular + rhs.angular;
		return result;
	}

	LinearAngularPair& operator+=(LinearAngularPair const& rhs)
	{
		this->linear += rhs.linear;
		this->angular += rhs.angular;
		return *this;
	}

	float operator*(LinearAngularPair const& rhs) const
	{
		return glm::dot(linear, rhs.linear) +
			glm::dot(angular, rhs.angular);
	}

	LinearAngularPair operator*(float rhs) const
	{
		LinearAngularPair result = {};
		result.linear = this->linear * rhs;
		result.angular = this->angular * rhs;
		return result;
	}
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
	void SequentialImpulseRoutine(float dt,
		std::vector<CollisionContact> const& contacts);

	//Contact related methods
	///glm::vec4 BodyToWorldContact(glm::vec4 const& body, RigidbodyComponent *rgbdy);
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