///HEADER

//SEE IF THIS IS TOO MUCH


#define USE_GJK		1	//If this is zero, we use a OBB SAT for very simple test narrow phase


#include "Renderer.h"

#include "PhysicsManager.h" 
#include "RigidbodyComponent.h"
#include "Physics/AABBTree.h"

#include "TransformComponent.h"
#include "GameObject.h"

#include "Math/Intersections.h"

#include <iostream>
#include "SAT.h"

//Manifold test
#include "Meshes/DebugSphere.h"
#include "Shader.h"
//Manifold test


PhysicsManager::PhysicsManager() : timeAccumulator(0.0f)
{
	std::cout << "Physics manager constructor." << std::endl;

	//Create new AABBTree
	vbh = new AABBTree();


	//SHADER AND MESH SETUP FOR DEBUG DRAW OF MANIFOLD (TEST)
	this->debugPointMesh = new DebugSphereOutline(8);
	this->debugShader = new Shader("Solid.vert", "Solid.frag");
	this->debugShader->BindUniformBlock("test_gUBlock", 1);
}


PhysicsManager::~PhysicsManager()
{
	std::cout << "Physics manager destructor." << std::endl;

	//Delete AABBTree
	delete vbh;
}


void PhysicsManager::PhysicsUpdate(float dt)
{
	//For now, update all rigidbodies using fixed timestep
	float fixedDT = 0.01666666666f;


	//Update VBH
	for (RigidbodyComponent *body : m_rigidbodies)
		vbh->Update(body);


	//BROADPHASE using VBH
	if (USEBVH) 
	{
		for (int i = 0; i < m_rigidbodies.size(); ++i)
		{
			RigidbodyComponent *rbA = m_rigidbodies[i];
			AABBNode *nodeA = vbh->FindLeafNode(rbA);
			RecursiveTreeCheck(nodeA, vbh->GetRoot());
		}
	}
	//BROADPHASE O(N^2)
	else if (!USEBVH)
	{
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

	//TESTING PURPOSES
	if (onEndOfFrameToggleVBH) 
	{
		toggleVBH();
		onEndOfFrameToggleVBH = false;
	}
}


void PhysicsManager::DebugDraw(Renderer *renderer)
{
	//BVH------------------------
	if (vbh) 
	{
		vbh->Draw(renderer);
	}

	//Manifold point pairs-------
	for (AuxMath::GJK_Manifold_V1& manifold : m_gjk_manifolds)
	{
		DrawDebugData data2 = {};
		data2.diffuseColor = { 1, 0, 0, 1.0f };
		glm::mat4 model(0.5f);
		model[3] = manifold.ptA;
		data2.model = model;
		data2.mesh = debugPointMesh;
		data2.shader = debugShader;
		renderer->QueueForDebugDraw(data2);

		model[3] = manifold.ptB;
		data2.model = model;
		renderer->QueueForDebugDraw(data2);
	}
	m_gjk_manifolds.clear();
}


void PhysicsManager::RegisterRigidbody(RigidbodyComponent* rgbdy) 
{
	//ADD, but first check if it exists maybe?
	//maybe change from vector to map?
	this->m_rigidbodies.push_back(rgbdy);


	//Once the rigidbody has been registered, add to the BVH
	vbh->Add(rgbdy);
}


std::vector<RigidbodyComponent*> const& PhysicsManager::GetRigidbodies() const
{
	return m_rigidbodies;
}


bool PhysicsManager::CheckCollision(RigidbodyComponent *A, RigidbodyComponent *B)
{
	Transform *TA = A->GetOwner()->GetComponent<Transform>();
	Transform *TB = B->GetOwner()->GetComponent<Transform>();

	AuxMath::OBB A_OBB(A->GetOBBCenterOffsetScaled() + TA->GetPosition(), A->GetOBBRadiusVector(), TA->GetRotationMatrix());
	AuxMath::OBB B_OBB(B->GetOBBCenterOffsetScaled() + TB->GetPosition(), B->GetOBBRadiusVector(), TB->GetRotationMatrix());

	glm::vec4 restitutionForce(0);


#if USE_GJK
	//WITH GJK
	std::vector<glm::vec4> simplex;
	bool intersects = AuxMath::GJKSolver::GJK_Intersects(A_OBB, B_OBB, simplex);
	if (intersects)
	{
		///Call EPA, giving the simplex we got from the 
		///intersection and the manifold to fill
		AuxMath::GJK_Manifold_V1 manifoldInfo = {};
		AuxMath::GJKSolver::EPA(simplex, manifoldInfo);

		///Push contact pair from EPA into list
		CollisionContact contact = {};
		contact.rbdyA = A;
		contact.rbdyA = B;
		contact.manifold = manifoldInfo;
		m_contacts.push_back(contact);

		///For now, for debug draw purposes
		m_gjk_manifolds.push_back(manifoldInfo);

		//std::cout << ">> INTERSECTION! - Simplex size: " << 
		//simplex.size() << ", >> " << rand() << std::endl;
	}
	else 
	{
		///For now, for debug draw purposes, current support when not intersecting
		AuxMath::GJK_Manifold_V1 manifoldInfo = {};
		manifoldInfo.ptA = simplex[0];
		manifoldInfo.ptB = simplex[1];
		m_gjk_manifolds.push_back(manifoldInfo);
	}

#else
	//SAT METHOD
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
#endif

	return intersects;
}


void PhysicsManager::RecursiveTreeCheck(AABBNode *current, AABBNode *starting)
{
	//Compare this rigidbody against the different levels of the tree.
	//Bad approach  - start at root, compare against both children, go down if necessary
	
	if (starting->isLeaf)
	{
		//CHECK if body collides against rbB. If so, store contact
		RigidbodyComponent *rbB = starting->body;
		RigidbodyComponent *rbA = current->body;
		if (CheckCollision(rbA, rbB))
		{
			CollisionContact contact = {};
			contact.rbdyA = rbA;
			contact.rbdyB = rbB;
			m_contacts.push_back(contact);
		}

		return;
	}
	else 
	{
		bool collidesLeft = AuxMath::AABBToAABBIntersection(current->center, current->size, starting->left->center, starting->left->size);
		bool collidesRight = AuxMath::AABBToAABBIntersection(current->center, current->size, starting->right->center, starting->right->size);
		if (collidesLeft)
			RecursiveTreeCheck(current, starting->left);
		if (collidesRight)
			RecursiveTreeCheck(current, starting->right);
	}
}