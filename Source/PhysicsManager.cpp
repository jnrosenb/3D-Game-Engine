///HEADER

#include "PhysicsManager.h" 
#include "RigidbodyComponent.h"

#include "TransformComponent.h"
#include "GameObject.h"

#include <iostream>
#include "SAT.h"


PhysicsManager::PhysicsManager() : timeAccumulator(0.0f)
{
	std::cout << "Physics manager constructor." << std::endl;
}

PhysicsManager::~PhysicsManager()
{
	std::cout << "Physics manager destructor." << std::endl;
}

void PhysicsManager::PhysicsUpdate(float dt)
{
	//For now, update all rigidbodies using fixed timestep
	float fixedDT = 0.01666666666f;

	//Check collision maybe? (broadphase)
		//Check narrowphase?
	for (int i = 0; i < m_rigidbodies.size(); ++i) 
	{
		RigidbodyComponent *rbA = m_rigidbodies[i];
		for (int j = i + 1; j < m_rigidbodies.size(); ++j)
		{
			RigidbodyComponent *rbB = m_rigidbodies[j];

			//CHECK IF rbA collides against rbB. If so, store in contact
			if (CheckCollision(rbA, rbB)) 
			{
				CollisionContact contact = {};
				contact.rbdyA = rbA;
				contact.rbdyB = rbB;
				m_contacts.push_back(contact);
			}
		}
	}

	//Collision resolution
	for (CollisionContact& contact : m_contacts) 
	{
		//Resolution
	}

	//JUST TEMPORARILY PUTTING IT HERE TO SEE CONTACT FORCES
	for (RigidbodyComponent *rgbdy : m_rigidbodies)
	{
		timeAccumulator += dt;
		if (timeAccumulator >= fixedDT)
		{
			rgbdy->PhysicsUpdate(fixedDT);
			timeAccumulator -= fixedDT;
		}
	}

	//Empty contact list
	m_contacts.clear();
}

void PhysicsManager::RegisterRigidbody(RigidbodyComponent* rgbdy) 
{
	//ADD, but first check if it exists maybe?
	//maybe change from vector to map?
	this->m_rigidbodies.push_back(rgbdy);
}


std::vector<RigidbodyComponent*> const& PhysicsManager::GetRigidbodies() const
{
	return m_rigidbodies;
}


bool PhysicsManager::CheckCollision(RigidbodyComponent *A, RigidbodyComponent *B)
{
	Transform *TA = A->GetOwner()->GetComponent<Transform>();
	Transform *TB = B->GetOwner()->GetComponent<Transform>();
	AuxMath::OBB A_OBB(TA->GetPosition(), A->GetOBBRadiusVector(), TA->GetRotationMatrix());
	AuxMath::OBB B_OBB(TB->GetPosition(), B->GetOBBRadiusVector(), TB->GetRotationMatrix());

	glm::vec4 restitutionForce(0);
	bool intersects = AuxMath::TestOBB_OBB(A_OBB, B_OBB, restitutionForce);
	
	if (A->GetMass() > 0.0f && B->GetMass() > 0.0f) 
	{
		//Restitute
		TA->translate(restitutionForce * 0.5f);
		TB->translate(restitutionForce * -0.5f);

		//Impulse code (for now, just apply a force)
		///A->ApplyForce(restitutionForce * 0.5f, {0, 0, 0});
		///B->ApplyForce(restitutionForce * -0.5f, { 0, 0, 0 });
	}
	else if (A->GetMass() > 0.0f)
	{
		//Restitute
		TA->translate(restitutionForce);

		//Impulse code (for now, just apply a force)
		///A->ApplyForce(restitutionForce, { 0, 0, 0 });
	}
	else if (B->GetMass() > 0.0f)
	{
		//Restitute
		TB->translate(restitutionForce * -1.0f);

		//Impulse code (for now, just apply a force)
		///B->ApplyForce(restitutionForce * -1.0f, { 0, 0, 0 });
	}
	
	return intersects;
}