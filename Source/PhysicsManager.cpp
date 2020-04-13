///HEADER

//SEE IF THIS IS TOO MUCH


#define USE_GJK		1	//If this is zero, we use a OBB SAT for very simple test narrow phase
#define CONTACT_NUM 4


#include "Renderer.h"

#include "PhysicsManager.h" 
#include "RigidbodyComponent.h"
#include "Physics/AABBTree.h"

#include "TransformComponent.h"
#include "GameObject.h"

#include "Math/Intersections.h"

#include <iostream>
#include <sstream>
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
				bool intersects = CheckCollision(rbA, rbB);
			}
		}
	}

	//Clear the contact dictionary (so that later, only         //////////////********
	//contacts that happened this iteration will be reinserted)	//////////////********
	m_persistentContacts.clear();

	//Check if contacts are valid. Discard otherwise
	for (CollisionContact& contact : m_contacts)
		CheckContactValidity(contact);


	//Collision resolution
	for (CollisionContact& contact : m_contacts) 
		CollisionResolutionTest(contact);


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


	//Empty contact list									   //////////////*********
	//By this point, all information about persistent contacts //////////////*********
	//is already safely stored back in the dictionary		   //////////////*********
	m_contacts.clear();


	//TESTING PURPOSES (To being able to toggle the BVH, but 
	//only make it effective at the end of the frame)
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
	for (auto& node : m_persistentContacts)
	{
		AuxMath::GJK_Manifold_V1& manifold = node.second.manifold;

		for (int i = 0; i < manifold.ptsA.size(); ++i) 
		{
			DrawDebugData data2 = {};
			glm::mat4 model(0.5f);

			data2.diffuseColor = { 1, 0, 0, 1.0f };
			data2.mesh = debugPointMesh;
			data2.shader = debugShader;
			
			model[3] = manifold.ptsA[i].world;
			if (manifold.ptsA[i].world.w != 1)
				assert(false);
			data2.model = model;
			renderer->QueueForDebugDraw(data2);
		}

		for (int i = 0; i < manifold.ptsB.size(); ++i)
		{
			DrawDebugData data2 = {};
			glm::mat4 model(0.5f);

			data2.diffuseColor = { 0, 1, 0, 1.0f };
			data2.mesh = debugPointMesh;
			data2.shader = debugShader;

			model[3] = manifold.ptsB[i].world;
			if (manifold.ptsB[i].world.w != 1)
				assert(false);
			data2.model = model;
			renderer->QueueForDebugDraw(data2);
		}
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

	//WITH GJK + EPA
	std::vector<AuxMath::GJK_MinkowskiMap> md_list(0);
	std::vector<glm::vec4> simplex;
	bool intersects = AuxMath::GJKSolver::GJK_Intersects(md_list, A_OBB, B_OBB, simplex);
	if (intersects)
	{
		///Call EPA, giving the simplex we got from the
		///intersection and the manifold to fill
		AuxMath::GJK_Manifold_V1 manifoldInfo = {};
		AuxMath::GJKSolver::EPA(md_list, A_OBB, B_OBB, simplex, manifoldInfo);

		//REMOVE-LATER///////////////////////////////////////////////////////////
		//Experiment with SAT - Wanna compare what I get from EPA with SAT	 ////
		glm::vec4 restitution_SAT;											 ////
		AuxMath::TestOBB_OBB(A_OBB, B_OBB, restitution_SAT);				 ////
		restitution_SAT = glm::normalize(restitution_SAT);					 ////
		/////////////////////////////////////////////////////////////////////////

		///Set the restitution
		manifoldInfo.restitution = glm::normalize(manifoldInfo.ptsB[0].world - manifoldInfo.ptsA[0].world);
		//manifoldInfo.restitution = restitution_SAT;

		//Print info
		/// glm::vec3 test = manifoldInfo.restitution - static_cast<glm::vec3>(restitution_SAT);
		/// std::cout << "DIFFERENCE BETWEEN EPA AND SAT: " 
		/// 		<< test.x << ", " << test.y << ", " << test.z << std::endl;
		/// std::cout << "LEN: " << glm::length(test) << std::endl;
		/// std::cout << "----------------------------" << std::endl;

		///Push contact pair from EPA into list
		CollisionContact contact = {};
		contact.rbdyA = A;
		contact.rbdyB = B;
		assert(A != nullptr && B != nullptr);
		contact.manifold = manifoldInfo;



		//Persistent contact---------------------------------------------------------------------
		#pragma region MyRegion
		//*
		std::ostringstream stringStream;
		stringStream << A->GetOwner()->GetId() << B->GetOwner()->GetId();
		std::string key1 = stringStream.str();
		auto iter = m_persistentContacts.find(key1);
		if (iter == m_persistentContacts.end()) 
		{
			//Its not here, add to contacts
			m_persistentContacts[key1] = contact;
		}
		else
		{
			//It does exist, so we need to update it
			auto& node = *iter;
			CollisionContact& persistentCt = node.second;
		
			//FOR BOTH OBJ A AND B!!!
			//1- Compare new contact against stored contacts. If not too close, add it to them
			float thressholdSQR = 0.01f;
			bool isAlreadyPresent = false;
			assert(persistentCt.manifold.ptsA.size() == persistentCt.manifold.ptsB.size());
			for (int i = 0; i < persistentCt.manifold.ptsA.size(); ++i)
			{
				AuxMath::BodyWorldPair const& ptA = persistentCt.manifold.ptsA[i];
				AuxMath::BodyWorldPair const& ptB = persistentCt.manifold.ptsB[i];

				float distanceSQR_A = glm::dot(ptA.body - contact.manifold.ptsA[0].body, 
					ptA.body - contact.manifold.ptsA[0].body);
				float distanceSQR_B = glm::dot(ptB.body - contact.manifold.ptsB[0].body,
					ptB.body - contact.manifold.ptsB[0].body);
				
				if (distanceSQR_A < thressholdSQR || distanceSQR_B < thressholdSQR)
				{
					//Same point, do nothing (later change this, just for visualizing)
					isAlreadyPresent = true;
					break;
				}
			}
			
			//Here we choose between: 
			//		-  Adding all points in persistentManifold to contact 
			//		-  Replacing contact by persistent manifold
			if (isAlreadyPresent == true) 
			{
				contact.manifold.ptsA.clear();
				for (AuxMath::BodyWorldPair point_a : persistentCt.manifold.ptsA)
					if (point_a.IsValid())
						contact.manifold.ptsA.push_back(point_a);
			}
			else 
			{
				//If persistent already has 4 points, then choose the 4 with biggest area
				//Else - Add persistent points to current contact
				if (persistentCt.manifold.ptsA.size() >= CONTACT_NUM)
				{
					//TODO (replacing points, maximize area)
				}
				else if (persistentCt.manifold.ptsA.size() < CONTACT_NUM)
				{
					for (AuxMath::BodyWorldPair point_a : persistentCt.manifold.ptsA)
						if (point_a.IsValid())
							contact.manifold.ptsA.push_back(point_a);
				}
			}

			//Here we choose between: 
			//		-  Adding all points in persistentManifold to contact 
			//		-  Replacing contact by persistent manifold
			if (isAlreadyPresent == true) 
			{
				contact.manifold.ptsB.clear();
				for (AuxMath::BodyWorldPair point_b : persistentCt.manifold.ptsB)
					if (point_b.IsValid())
						contact.manifold.ptsB.push_back(point_b);
			}
			else
			{
				//If persistent already has 4 points, then we just replace for now
				if (persistentCt.manifold.ptsB.size() >= CONTACT_NUM) 
				{
					//TODO - Maximize area when replacing points
				}
				else if (persistentCt.manifold.ptsB.size() < CONTACT_NUM)
				{
					for (AuxMath::BodyWorldPair point_b : persistentCt.manifold.ptsB)
						if (point_b.IsValid())
							contact.manifold.ptsB.push_back(point_b);
				}
			}
		}
		//*/
		#pragma endregion


		//Finally push back the contact
		m_contacts.push_back(contact);
	}

	return intersects;
}


//First collision response, just random impulse being applied
void PhysicsManager::CollisionResolutionTest(CollisionContact const& contact)
{	
	RigidbodyComponent *A = contact.rbdyA;
	RigidbodyComponent *B = contact.rbdyB;
	AuxMath::GJK_Manifold_V1 manifoldInfo = contact.manifold;
	float mgtd = 1.0f;
	
	//Resolution for first body
	for (int i = 0; i < manifoldInfo.ptsA.size(); ++i)
	{
		//Get the points for the first contact
		glm::vec4 pointA_i = manifoldInfo.ptsA[i].world;
		glm::vec3 restitutionForce = static_cast<glm::vec3>(manifoldInfo.restitution);

		if (A->GetMass() > 0.0f)
			A->ApplyForce(restitutionForce * (mgtd/ manifoldInfo.ptsA.size()), static_cast<glm::vec3>(pointA_i) - A->GetPositionEstimate());
	}

	//Resolution for second body
	for (int i = 0; i < manifoldInfo.ptsB.size(); ++i)
	{
		//Get the points for the first contact
		glm::vec4 pointB_i = manifoldInfo.ptsB[i].world;
		glm::vec3 restitutionForce = static_cast<glm::vec3>(-manifoldInfo.restitution);

		if (B->GetMass() > 0.0f)
			B->ApplyForce(restitutionForce * (mgtd / manifoldInfo.ptsB.size()), static_cast<glm::vec3>(pointB_i) - B->GetPositionEstimate());
	}


	//Re insert CONTACT into dictionary
	std::ostringstream stringStream;
	stringStream << contact.rbdyA->GetOwner()->GetId() << contact.rbdyB->GetOwner()->GetId();
	std::string key = stringStream.str();
	m_persistentContacts[key] = contact;
}


//First collision response, just random impulse being applied
void PhysicsManager::CheckContactValidity(CollisionContact const& contact)
{	
	//If the dot product abs is less than this, contact is valid
	float AcceptableThresshold = 0.0001f;

	//Get auxiliar info
	RigidbodyComponent *A = contact.rbdyA;
	RigidbodyComponent *B = contact.rbdyB;
	AuxMath::GJK_Manifold_V1 manifoldInfo = contact.manifold;
	
	//For this to work, I need both to have same number of contacts
	///assert(manifoldInfo.ptsA.size() == manifoldInfo.ptsB.size());
	float size = std::min(manifoldInfo.ptsA.size(), manifoldInfo.ptsB.size());

	//For each pair of contacts
	for (int i = 0; i < size; ++i)
	{
		//Normal from obj A to B (FOR NOW)
		glm::vec3 nAB = static_cast<glm::vec3>(manifoldInfo.restitution);
	
		//Get the points for the first contact
		glm::vec4 nonValidated_World_A = manifoldInfo.ptsA[i].world;
		glm::vec4 bodyPoint_A = manifoldInfo.ptsA[i].body;
		glm::vec4 validated_world_A = BodyToWorldContact(bodyPoint_A, A);
		float lenSqrA = glm::dot(validated_world_A- nonValidated_World_A, validated_world_A - nonValidated_World_A);

		//Get the points for the second contact
		glm::vec4 nonValidated_World_B = manifoldInfo.ptsB[i].world;
		glm::vec4 bodyPoint_B = manifoldInfo.ptsB[i].body;
		glm::vec4 validated_world_B = BodyToWorldContact(bodyPoint_B, B);
		float lenSqrB = glm::dot(validated_world_A - nonValidated_World_A, validated_world_A - nonValidated_World_A);

		//If this dot product is greater than thresshold, we delete the contacts
		glm::vec3 CtAToCtB = static_cast<glm::vec3>(validated_world_B - validated_world_A);
		float dot = glm::dot(CtAToCtB, nAB);
		if (dot > 0.0f || lenSqrA > AcceptableThresshold || lenSqrB > AcceptableThresshold)
		{
			manifoldInfo.ptsA[i].MarkAsInvalid();
			manifoldInfo.ptsB[i].MarkAsInvalid();
			
			std::cout << "INVALIDATED CONTACT - " << rand() << std::endl;
		}
	}
}



glm::vec4 PhysicsManager::BodyToWorldContact(glm::vec4 const& body, RigidbodyComponent *rgbdy) 
{
	//Get auxiliar data
	Transform *T = rgbdy->GetOwner()->GetComponent<Transform>();
	glm::mat4 const& R = T->GetRotationMatrix();
	glm::vec3 const& radius = rgbdy->GetOBBRadiusVector();

	//This should be center of mass, not position!!! - Wont work with objects whose COM differs from pivot
	glm::vec4 const& COM = T->GetPosition();

	return COM + (R[0] * body.x * radius.x) + (R[1] * body.y * radius.y) + 
		(R[2] * body.z * radius.z);
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

		//Make sure two same objects are not being intersected twice
		//TODO++++++++++++++++++++++++++++++

		bool intersects = CheckCollision(rbA, rbB);

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