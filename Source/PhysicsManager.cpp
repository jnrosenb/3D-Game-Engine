///HEADER

//SEE IF THIS IS TOO MUCH


#define USE_GJK						1	//If this is zero, we use a OBB SAT for very simple test narrow phase
#define CONTACT_NUM					4
#define SEQ_IMP_MAX_ITERATIONS		5


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

#include "Physics/ConstraintBased.h"


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



	//PHYSICS UPDATE - NON FIXED TIME STEP
	for (RigidbodyComponent *rgbdy : m_rigidbodies)
	{
		//Fixed timestep
		/// timeAccumulator += dt;
		/// if (timeAccumulator >= fixedDT)
		/// {
		/// 	rgbdy->PhysicsUpdate(fixedDT);
		/// 	timeAccumulator -= fixedDT;
		/// }
		rgbdy->PhysicsUpdate(dt);
	}
	//PHYSICS UPDATE - NON FIXED TIME STEP



	//Check if contacts are valid. Discard otherwise
	for (auto& node : m_persistentContacts)
		CheckContactValidity(node.second);

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


	//Clear the contact dictionary (so that later, only        
	//contacts that happened this iteration will be reinserted)
	m_persistentContacts.clear();




	//Collision resolution----------------------------------////////////////////////////////****************
	if (m_contacts.size() > 0)								////////////////////////////////****************
		SequentialImpulseRoutine(dt, m_contacts);			////////////////////////////////****************
	//Collision resolution----------------------------------////////////////////////////////****************




	//Re insert CONTACT info into dictionary (for persistence)
	//Then empty the frame's contact list
	for (ContactSet& contact : m_contacts)
	{		
		std::ostringstream stringStream;
		stringStream << contact.rbdyA->GetOwner()->GetId() << contact.rbdyB->GetOwner()->GetId();
		std::string key = stringStream.str();
		m_persistentContacts[key] = contact;
	}
	m_contacts.clear();		  


	//RESET FORCES
	for (RigidbodyComponent *body : m_rigidbodies)
		body->ResetForces();


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

			assert(manifold.ptsA[i].IsValid());
			assert(manifold.ptsA[i].world.w == 1);
			
			model[3] = manifold.ptsA[i].world;
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

			assert(manifold.ptsB[i].IsValid());
			assert(manifold.ptsB[i].world.w == 1);

			model[3] = manifold.ptsB[i].world;
			data2.model = model;
			renderer->QueueForDebugDraw(data2);
		}
	}
	m_gjk_manifolds.clear();
}


void PhysicsManager::RegisterRigidbody(RigidbodyComponent* rgbdy) 
{
	//Make this more efficient - Right now its On
	//ADD, but first check if it exists maybe?
	for (RigidbodyComponent *body : m_rigidbodies)
		if (rgbdy == body)
			return;

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

		assert(manifoldInfo.ptsA.size() == 1);

		//REMOVE-LATER///////////////////////////////////////////////////////////
		//Experiment with SAT - Wanna compare what I get from EPA with SAT	 ////
		///glm::vec4 restitution_SAT;										 ////
		///AuxMath::TestOBB_OBB(A_OBB, B_OBB, restitution_SAT);				 ////
		///restitution_SAT = glm::normalize(restitution_SAT);				 ////
		/////////////////////////////////////////////////////////////////////////


		///Set the restitution
		manifoldInfo.restitution = glm::normalize(manifoldInfo.ptsB[0].world - manifoldInfo.ptsA[0].world);
		//manifoldInfo.restitution = restitution_SAT;
		/// glm::vec3 test = manifoldInfo.restitution - static_cast<glm::vec3>(restitution_SAT);
		/// std::cout << "DIFFERENCE BETWEEN EPA AND SAT: " 
		/// 		<< test.x << ", " << test.y << ", " << test.z << std::endl;
		/// std::cout << "LEN: " << glm::length(test) << std::endl;
		/// assert(glm::length(test) < 0.1f);
		/// std::cout << "----------------------------" << std::endl;

		///Push contact pair from EPA into list
		ContactSet contact = {};
		contact.rbdyA = A;
		contact.rbdyB = B;
		assert(A != nullptr && B != nullptr);
		contact.manifold = manifoldInfo;

		//Persistent contact------------------------***
		//CHANGE NAME TO BETTER REFLECT WHAT IT DOES***
		//This will add a new contact - First check if its too close to existing ones
		//Then, if more than 4 contacts exists, it will select the ones that maximize the area
		AddPersistentContactToManifold(A, B, contact);


		//Finally push back the contact
		m_contacts.push_back(contact);
	}

	return intersects;
}



//First collision response, just random impulse being applied
void PhysicsManager::CheckContactValidity(ContactSet& contact)
{	
	///std::cout << "---Contact-Validation----------------" << std::endl;
	///std::cout << "\t>> Size of contact: " << contact.manifold.ptsA.size() << std::endl;

	//If the dot product abs is less than this, contact is valid
	float AcceptableThresshold = 1.5f; // -> 0.1f for 2 contacts - 0.75f for 3 contacts - 1.5f for 4?

	//Get auxiliar info
	RigidbodyComponent *A = contact.rbdyA;
	RigidbodyComponent *B = contact.rbdyB;
	
	//For this to work, I need both to have same number of contacts
	assert(contact.manifold.ptsA.size() == contact.manifold.ptsB.size());
	
	//For each pair of contacts
	int size = contact.manifold.ptsA.size();
	for (int i = 0; i < size; ++i)
	{
		//Normal from obj A to B (FOR NOW)
		glm::vec3 nAB = static_cast<glm::vec3>(contact.manifold.restitution);
	
		//Get the points for the first contact
		glm::vec4 nonValidated_World_A = contact.manifold.ptsA[i].world;
		glm::vec4 bodyPoint_A = contact.manifold.ptsA[i].body;
		glm::vec4 validated_world_A = A->BodyToWorldContact(bodyPoint_A);
		float lenSqrA = glm::dot(validated_world_A- nonValidated_World_A, validated_world_A - nonValidated_World_A);

		//Get the points for the second contact
		glm::vec4 nonValidated_World_B = contact.manifold.ptsB[i].world;
		glm::vec4 bodyPoint_B = contact.manifold.ptsB[i].body;
		glm::vec4 validated_world_B = B->BodyToWorldContact(bodyPoint_B);
		float lenSqrB = glm::dot(validated_world_B - nonValidated_World_B, validated_world_B - nonValidated_World_B);

		//If this dot product is greater than thresshold, we delete the contacts
		glm::vec3 CtAToCtB = static_cast<glm::vec3>(validated_world_B - validated_world_A);
		float dot = glm::dot(CtAToCtB, nAB);
		//if (dot <= 0.0f || lenSqrA > AcceptableThresshold || lenSqrB > AcceptableThresshold)
		if (lenSqrA > AcceptableThresshold || lenSqrB > AcceptableThresshold)
		{
			contact.manifold.ptsA[i].MarkAsInvalid();
			contact.manifold.ptsB[i].MarkAsInvalid();
			
			///std::cout << "\t>> INVALIDATED CONTACT" << rand() << std::endl;
		}
	}

	//This is so no invalid contact is left on the vector
	ContactSet validContacts = {};
	for (int i = 0; i < contact.manifold.ptsA.size(); ++i)
	{
		if (contact.manifold.ptsA[i].IsValid() && contact.manifold.ptsB[i].IsValid())
		{
			validContacts.manifold.ptsA.push_back(contact.manifold.ptsA[i]);
			validContacts.manifold.ptsB.push_back(contact.manifold.ptsB[i]);
		}
	}
	contact.manifold.ptsA.clear();
	contact.manifold.ptsB.clear();
	for (int i = 0; i < validContacts.manifold.ptsA.size(); ++i)
	{
		contact.manifold.ptsA.push_back(validContacts.manifold.ptsA[i]);
		contact.manifold.ptsB.push_back(validContacts.manifold.ptsB[i]);
	}

	///std::cout << "\t>> Size of contact: " << contact.manifold.ptsA.size() << std::endl;
	///std::cout << "------------------------------------------------------" << std::endl;
}



//For a contact pair, gives you the world position based on the body position (of the collider)
/// glm::vec4 PhysicsManager::BodyToWorldContact(glm::vec4 const& body, RigidbodyComponent *rgbdy)
/// {
/// 	//Get auxiliar data
/// 	Transform *T = rgbdy->GetOwner()->GetComponent<Transform>();
/// 	glm::mat4 const& R = T->GetRotationMatrix();
/// 	glm::vec3 const& radius = rgbdy->GetOBBRadiusVector();
/// 
/// 	//This should be center of mass, not position!!! - Wont work with objects whose COM differs from pivot
/// 	glm::vec4 const& COM = T->GetPosition();
/// 
/// 	return COM + (R[0] * body.x * radius.x) + (R[1] * body.y * radius.y) +
/// 		(R[2] * body.z * radius.z);
/// }



//Adds a new contact to the contact list
void PhysicsManager::AddPersistentContactToManifold(RigidbodyComponent *A,
	RigidbodyComponent *B, ContactSet& contact)
{
	//Find key to access the contact from the hash
	std::ostringstream stringStream;
	stringStream << A->GetOwner()->GetId() << B->GetOwner()->GetId();
	std::string key1 = stringStream.str();
	auto iter = m_persistentContacts.find(key1);

	assert(contact.manifold.ptsA[0].IsValid() && contact.manifold.ptsA[0].IsValid());
	assert(contact.manifold.ptsA.size() == 1);

	//Add persistentInfo into current contact (since persistent info will be cleared afterwards)
	if (iter != m_persistentContacts.end())
	{
		//It does exist, so we need to update it
		auto& node = *iter;
		ContactSet& persistentCt = node.second;

		//DEBUG ---------------------
		assert(persistentCt.manifold.ptsA.size() == persistentCt.manifold.ptsB.size());
		assert(contact.manifold.ptsA.size() == contact.manifold.ptsB.size());

		/////////////////////////////////////////////
		//////   MANAGING CLOSE CONTACTS       //////
		/////////////////////////////////////////////
		float thressholdSQR = 0.001f;
		bool isAlreadyPresent = false;
		int persistentSize = persistentCt.manifold.ptsA.size();
		for (int i = 0; i < persistentSize; ++i)
		{
			AuxMath::BodyWorldPair const& ptA = persistentCt.manifold.ptsA[i];
			AuxMath::BodyWorldPair const& ptB = persistentCt.manifold.ptsB[i];

			float distanceSQR_A = glm::dot(ptA.body - contact.manifold.ptsA[0].body,
				ptA.body - contact.manifold.ptsA[0].body);
			float distanceSQR_B = glm::dot(ptB.body - contact.manifold.ptsB[0].body,
				ptB.body - contact.manifold.ptsB[0].body);

			if (distanceSQR_A < thressholdSQR || distanceSQR_B < thressholdSQR)
			{
				isAlreadyPresent = true;
				break;
			}
		}

		//If we have three points and are adding a fourth, dont unless its outside of the triangle formed by the other 2

		//Here we choose between: 
		//		-  Adding all points in persistentManifold to contact 
		//		-  Replacing contact by persistent manifold
		if (isAlreadyPresent == true)
			contact.manifold = persistentCt.manifold;
		else
		{
			//If persistent already has 4 points, then choose the 4 with biggest area
			if (persistentSize >= CONTACT_NUM)
			{
				ChooseFourContacts(persistentCt, contact);
			}
			else
			{
				/*If it has three, check its not inside the triangle
				if (persistentSize == 3) 
				{
					AuxMath::BodyWorldPair const& P = contact.manifold.ptsA[0];
				
					AuxMath::BodyWorldPair const& firstA = persistentCt.manifold.ptsA[0];
					AuxMath::BodyWorldPair const& secondA = persistentCt.manifold.ptsA[1];
					AuxMath::BodyWorldPair const& thirdA = persistentCt.manifold.ptsA[2];
						
					glm::vec4 NmlA = AuxMath::Simplex::vec4Cross(
						secondA.world - firstA.world, thirdA.world - firstA.world);
				
					glm::vec4 R = P.world - NmlA * glm::dot((P.world - firstA.world), NmlA);
					R.w = 1.0f;
					float Area_AB = glm::dot(NmlA, AuxMath::Simplex::vec4Cross(R - secondA.world, firstA.world - secondA.world));
					float Area_BC = glm::dot(NmlA, AuxMath::Simplex::vec4Cross(R - thirdA.world, secondA.world - thirdA.world));
					float Area_AC = glm::dot(NmlA, AuxMath::Simplex::vec4Cross(R - firstA.world, thirdA.world - firstA.world));
					float totalArea = Area_AB + Area_AC + Area_BC;
					float gamma = Area_AB / totalArea;
					float beta = Area_AC / totalArea;
					float alpha = 1.0f - gamma - beta;
				
					if (alpha > 1.0f || beta > 1.0f || gamma > 1.0f)
				    {
				    	//What we did originally 
				    	//(later make code concise and eliminate repeated code)
				    	for (int i = 0; i < persistentSize; ++i)
				    	{
				    		AuxMath::BodyWorldPair& point_a = persistentCt.manifold.ptsA[i];
				    		AuxMath::BodyWorldPair& point_b = persistentCt.manifold.ptsB[i];
				    
				    		assert(point_a.IsValid() && point_b.IsValid());
				    
				    		contact.manifold.ptsA.push_back(point_a);
				    		contact.manifold.ptsB.push_back(point_b);
				    	}
				    }
					else
						contact.manifold = persistentCt.manifold;
				}
				//*/

				/*If it has two, it should check it is not colinear 
				else if (persistentSize == 2) 
				{
					AuxMath::BodyWorldPair const& P = contact.manifold.ptsA[0];
					AuxMath::BodyWorldPair const& firstA = persistentCt.manifold.ptsA[0];
					AuxMath::BodyWorldPair const& secondA = persistentCt.manifold.ptsA[1];

					glm::vec4 test = AuxMath::Simplex::vec4Cross(
						P.world - firstA.world, secondA.world - firstA.world);

					//If cross is not zero, add
					float epsilonTest = 1.1f;
					float val = glm::length(test);
					if (val >= epsilonTest)
					{
						AuxMath::BodyWorldPair& point_a0 = persistentCt.manifold.ptsA[0];
						AuxMath::BodyWorldPair& point_a1 = persistentCt.manifold.ptsA[1];
						AuxMath::BodyWorldPair& point_b0 = persistentCt.manifold.ptsB[0];
						AuxMath::BodyWorldPair& point_b1 = persistentCt.manifold.ptsB[1];

						assert(point_a0.IsValid() && point_b0.IsValid() && 
							point_a1.IsValid() && point_b1.IsValid());

						contact.manifold.ptsA.push_back(point_a0);
						contact.manifold.ptsA.push_back(point_a1);
						contact.manifold.ptsB.push_back(point_b0);
						contact.manifold.ptsB.push_back(point_b1);
					}
					else //If its colinear, add two 
					{
						contact.manifold = persistentCt.manifold;
					}
				}
				//*/

				//If it has one, just add the second
				///else 
				///{
					for (int i = 0; i < persistentSize; ++i)
					{
						AuxMath::BodyWorldPair& point_a = persistentCt.manifold.ptsA[i];
						AuxMath::BodyWorldPair& point_b = persistentCt.manifold.ptsB[i];

						assert(point_a.IsValid() && point_b.IsValid());

						contact.manifold.ptsA.push_back(point_a);
						contact.manifold.ptsB.push_back(point_b);
					}
				///}
			}
		}
	}

	assert(contact.manifold.ptsA.size() == contact.manifold.ptsB.size());
}



//Desc
void PhysicsManager::ChooseFourContacts(ContactSet const& persistentCt, 
	ContactSet& contact)
{
	//Check if this doesnt happen, and why
	assert(contact.manifold.ptsA.size() == 1);
	assert(persistentCt.manifold.ptsA.size() == CONTACT_NUM);

	//For easy of handling, mix all in a single 
	//contact, and iterate through that one
	ContactSet auxCt = {};
	auxCt.manifold = persistentCt.manifold;
	auxCt.manifold.ptsA.push_back(contact.manifold.ptsA[0]);
	auxCt.manifold.ptsB.push_back(contact.manifold.ptsB[0]);

	//More asserts
	assert(persistentCt.manifold.ptsA.size() == persistentCt.manifold.ptsB.size());

	///For first point, choose the one with the highest penetration value
	AuxMath::BodyWorldPair firstA;
	AuxMath::BodyWorldPair firstB;
	float max = std::numeric_limits<float>().lowest();
	for (int i = 0; i < auxCt.manifold.ptsA.size(); ++i)
	{
		float penetrationSQR = glm::dot(auxCt.manifold.ptsB[i].world - auxCt.manifold.ptsA[i].world,
			auxCt.manifold.ptsB[i].world - auxCt.manifold.ptsA[i].world);
		if (penetrationSQR > max)
		{
			max = penetrationSQR;
			firstA = auxCt.manifold.ptsA[i];
			firstB = auxCt.manifold.ptsB[i];
		}
	}

	///For second point, choose the one furthest from the first
	AuxMath::BodyWorldPair secondA;
	max = std::numeric_limits<float>().lowest();
	for (AuxMath::BodyWorldPair const& pt : auxCt.manifold.ptsA) 
	{
		float sqrLen = glm::dot(firstA.world - pt.world, 
			firstA.world - pt.world);
		if (sqrLen > max) 
		{
			max = sqrLen;
			secondA = pt;
		}
	}
	AuxMath::BodyWorldPair secondB;
	max = std::numeric_limits<float>().lowest();
	for (AuxMath::BodyWorldPair const& pt : auxCt.manifold.ptsB)
	{
		float sqrLen = glm::dot(firstB.world - pt.world, 
			firstB.world - pt.world);
		if (sqrLen > max)
		{
			max = sqrLen;
			secondB = pt;
		}
	}

	///For third, choose the furthest from first-second edge
	AuxMath::BodyWorldPair thirdA;
	max = std::numeric_limits<float>().lowest();
	for (AuxMath::BodyWorldPair const& pt : auxCt.manifold.ptsA)
	{
		//Compute sqr of double the area of triangle First-Second-Pt
		glm::vec4 cross = AuxMath::Simplex::vec4Cross(secondA.world - firstA.world, pt.world - firstA.world);
		float maxDoubleAreaSqr = glm::dot(cross, cross);
		if (maxDoubleAreaSqr > max) 
		{
			max = maxDoubleAreaSqr;
			thirdA = pt;
		}
	}
	AuxMath::BodyWorldPair thirdB;
	max = std::numeric_limits<float>().lowest();
	for (AuxMath::BodyWorldPair const& pt : auxCt.manifold.ptsB)
	{
		//Compute sqr of double the area of triangle First-Second-Pt
		glm::vec4 cross = AuxMath::Simplex::vec4Cross(secondB.world - firstB.world, pt.world - firstB.world);
		float maxDoubleAreaSqr = glm::dot(cross, cross);
		if (maxDoubleAreaSqr > max)
		{
			max = maxDoubleAreaSqr;
			thirdB = pt;
		}
	}

	///For fourth, get barycentric coords of reminding two points 
	///from points, and choose whichever does not belong on the triangle
	AuxMath::BodyWorldPair fourthA;
	max = std::numeric_limits<float>().lowest();
	glm::vec4 NmlA = AuxMath::Simplex::vec4Cross(secondA.world - firstA.world, thirdA.world - firstA.world);
	bool found4A = false;
	for (AuxMath::BodyWorldPair const& P : auxCt.manifold.ptsA)
	{
		glm::vec4 R = P.world - NmlA * glm::dot((P.world - firstA.world), NmlA);
		R.w = 1.0f;
		float Area_AB = glm::dot(NmlA, AuxMath::Simplex::vec4Cross(R - secondA.world, firstA.world - secondA.world));
		float Area_BC = glm::dot(NmlA, AuxMath::Simplex::vec4Cross(R - thirdA.world, secondA.world - thirdA.world));
		float Area_AC = glm::dot(NmlA, AuxMath::Simplex::vec4Cross(R - firstA.world, thirdA.world - firstA.world));
		float totalArea = Area_AB + Area_AC + Area_BC;
		float gamma = Area_AB / totalArea;
		float beta = Area_AC / totalArea;
		float alpha = 1.0f - gamma - beta;

		if (alpha > 1.0f || beta > 1.0f || gamma > 1.0f) 
		{
			found4A = true;
			fourthA = P;
		}
	}
	AuxMath::BodyWorldPair fourthB;
	max = std::numeric_limits<float>().lowest();
	glm::vec4 NmlB = AuxMath::Simplex::vec4Cross(secondB.world - firstB.world, thirdB.world - firstB.world);
	bool found4B = false;
	for (AuxMath::BodyWorldPair const& P : auxCt.manifold.ptsB)
	{
		glm::vec4 R = P.world - NmlB * glm::dot((P.world - firstB.world), NmlB);
		R.w = 1.0f;
		float Area_AB = glm::dot(NmlB, AuxMath::Simplex::vec4Cross(R - secondB.world, firstB.world - secondB.world));
		float Area_BC = glm::dot(NmlB, AuxMath::Simplex::vec4Cross(R - thirdB.world, secondB.world - thirdB.world));
		float Area_AC = glm::dot(NmlB, AuxMath::Simplex::vec4Cross(R - firstB.world, thirdB.world - firstB.world));
		float totalArea = Area_AB + Area_AC + Area_BC;
		float gamma = Area_AB / totalArea;
		float beta = Area_AC / totalArea;
		float alpha = 1.0f - gamma - beta;

		if (alpha > 1.0f || beta > 1.0f || gamma > 1.0f)
		{
			found4B = true;
			fourthB = P;
		}
	}

	//Add the four pairs into the manifold
	contact.manifold.ptsA.clear();
	contact.manifold.ptsB.clear();
	contact.manifold.ptsA.push_back(firstA);
	contact.manifold.ptsB.push_back(firstB);
	contact.manifold.ptsA.push_back(secondA);
	contact.manifold.ptsB.push_back(secondB);
	contact.manifold.ptsA.push_back(thirdA);
	contact.manifold.ptsB.push_back(thirdB);
	if (found4A && found4B) // ||
	{
		contact.manifold.ptsA.push_back(fourthA);
		contact.manifold.ptsB.push_back(fourthB);
	}

	//After this, contact needs to have the max num of contacts, no more and no less
	assert(contact.manifold.ptsA.size() == contact.manifold.ptsB.size());
	
	/// //For now, weird experiment with normals
	/// if (contact.manifold.ptsA.size() == 3) 
	/// {
	/// 
	/// }
	/// else if (contact.manifold.ptsA.size() == 4)
	/// {
	/// 
	/// }
}



//desc
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
		//CHEAP WAY (true way is to change traversal down)
		if (rbB == rbA)
			return;
		std::ostringstream stringStream;
		stringStream << rbA->GetOwner()->GetId() << rbB->GetOwner()->GetId();
		std::string key1 = stringStream.str();
		std::ostringstream stringStream2;
		stringStream2 << rbB->GetOwner()->GetId() << rbA->GetOwner()->GetId();
		std::string key2 = stringStream2.str();
		auto iter1 = m_persistentContacts.find(key1);
		auto iter2 = m_persistentContacts.find(key2);
		
		if (iter1 == m_persistentContacts.end() && iter2 == iter1)
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



/////////////////////////////////
////   COLISION RESOLUTION   ////
/////////////////////////////////
void PhysicsManager::SequentialImpulseRoutine(float dt,
	std::vector<ContactSet>& contacts) 
{
	float correctedDt = dt / (SEQ_IMP_MAX_ITERATIONS);

	//Declare the structures
	std::vector<std::vector<LinearAngularPair>> Jsp;
	std::vector<std::vector<unsigned>> Jmap;
	std::vector<LinearAngularPair> Vcurr;
	
	//Fill the jacobian information //////////////////***********CHECK IF USING VALID CONTACT INFO
	AuxMath::JacobianSetup(contacts, Jsp);
	Jmap.resize(Jsp.size());
	for (int i = 0; i < Jmap.size(); ++i)
		Jmap[i].resize(2);
	
	//Setup rigidbody vector and the indices on jmap
	std::unordered_map<RigidbodyComponent*, unsigned> insertMap;
	std::vector<RigidbodyComponent*> bodies;
	int constraintIndex = 0, i = 0;
	for (ContactSet const& contact : contacts) 
	{
		for (int j = 0; j < contact.manifold.ptsA.size(); ++j)
		{
			//For first contact, this is done
			if (j == 0) 
			{
				auto& iter = insertMap.find(contact.rbdyA);
				if (iter == insertMap.end())
				{
					insertMap[contact.rbdyA] = i++;
					bodies.push_back(contact.rbdyA);
				}
				Jmap[constraintIndex + j][0] = insertMap[contact.rbdyA];

				iter = insertMap.find(contact.rbdyB); 
				if (iter == insertMap.end())
				{
					insertMap[contact.rbdyB] = i++;
					bodies.push_back(contact.rbdyB);
				}
				Jmap[constraintIndex + j][1] = insertMap[contact.rbdyB];
			}
			//For else cases, only update Jmap, not the rigidbody container
			else 
			{
				auto& iter = insertMap.find(contact.rbdyA);
				assert(iter != insertMap.end());
				Jmap[constraintIndex + j][0] = insertMap[contact.rbdyA];

				iter = insertMap.find(contact.rbdyB);
				assert(iter != insertMap.end());
				Jmap[constraintIndex + j][1] = insertMap[contact.rbdyB];
			}
			
			//Update contraintIndex
			if (j == contact.manifold.ptsA.size() - 1)
				constraintIndex += contact.manifold.ptsA.size();
		}
	}

	//1- Setup lambda
	//Lambda holds S floats
	std::vector<float> Lambda;
	for (ContactSet const& contact : contacts)
	{
		for (int j = 0; j < contact.manifold.ptsA.size(); ++j)
		{
			if (contact.manifold.Lambdas.size() == contact.manifold.ptsA.size())
				Lambda.push_back(contact.manifold.Lambdas[j]);
			else 
				Lambda.push_back(0.0f);
		}
	}

	//Setup the velocity vector
	AuxMath::VelocityVectorSetup(bodies, Vcurr);


	//-WARM-START-EXPERIMENT---
	/// std::vector<LinearAngularPair> Fc0;
	/// AuxMath::ImpulseSolver(Fc0, Lambda, bodies, Jsp, Jmap);
	/// unsigned s = Jsp.size();
	/// for (int i = 0; i < s; ++i)
	/// {
	/// 	unsigned b1 = Jmap[i][0];
	/// 	unsigned b2 = Jmap[i][1];
	/// 	RigidbodyComponent *A = bodies[b1];
	/// 	RigidbodyComponent *B = bodies[b2];
	/// 
	/// 	if (A->GetMass() > 0.0f && (Fc0[b1].linear != glm::vec4(0) || Fc0[b1].angular != glm::vec4(0)))
	/// 		A->ApplyImpulse(correctedDt, Fc0[b1].linear * correctedDt, Fc0[b1].angular * correctedDt);
	/// 	if (B->GetMass() > 0.0f && (Fc0[b2].linear != glm::vec4(0) || Fc0[b2].angular != glm::vec4(0)))
	/// 		B->ApplyImpulse(correctedDt, Fc0[b2].linear * correctedDt, Fc0[b2].angular * correctedDt);
	/// }
	//-------------------------


	//Start iterations
	unsigned iters = 0;
	while (iters++ < SEQ_IMP_MAX_ITERATIONS)
	{
		///Update velocity vector //POTENTIALLY USELESS
		AuxMath::VelocityVectorUpdate(bodies, Vcurr);

		//3- Setup Cpos and Cvel
		std::vector<float> Cpos;
		std::vector<float> Cvel;
		AuxMath::PositionConstraintSolver(Cpos, contacts);			//USING -N because of not yet having a normal for the contacts
		AuxMath::VelocityConstraintSolver(correctedDt, Cvel, Cpos,	//USING -N because of not yet having a normal for the contacts 
			Vcurr, bodies, Jsp, Jmap);

		//Run Gaus-Seidel to get the multipliers
		AuxMath::LagrangeMultipliersSolver( correctedDt, Lambda,
			bodies, Vcurr, Jsp, Jmap, Cpos, Cvel);

		//Calculate constraint forces
		std::vector<LinearAngularPair> Fc;
		AuxMath::ImpulseSolver(Fc, Lambda, bodies, Jsp, Jmap);

		//Calculate impulses based on forces
		unsigned s = Jsp.size();
		for (int i = 0; i < s; ++i)
		{
			unsigned b1 = Jmap[i][0];
			unsigned b2 = Jmap[i][1];
			RigidbodyComponent *A = bodies[b1];
			RigidbodyComponent *B = bodies[b2];

			if (A->GetMass() > 0.0f && (Fc[b1].linear != glm::vec4(0) || Fc[b1].angular != glm::vec4(0)))
			{
				A->ApplyImpulse(correctedDt, Fc[b1].linear * correctedDt, Fc[b1].angular  * correctedDt);
			}
			if (B->GetMass() > 0.0f && (Fc[b2].linear != glm::vec4(0) || Fc[b2].angular != glm::vec4(0)))
			{
				B->ApplyImpulse(correctedDt, Fc[b2].linear * correctedDt, Fc[b2].angular * correctedDt);
			}
		}
	}

	//Store the lambda info on the contacts
	CacheImpulsesOnContacts(contacts, Lambda);
}



void PhysicsManager::CacheImpulsesOnContacts(
	std::vector<ContactSet>& contacts,
	std::vector<float> const& Lambda) 
{
	int constraintIndex = 0;
	for (ContactSet& contact : contacts)
	{
		contact.manifold.Lambdas.clear();

		for (int i = 0; i < contact.manifold.ptsA.size(); ++i)
		{
			//Update the lambda for the constraint
			contact.manifold.Lambdas.push_back(Lambda[constraintIndex]);

			//Update contraintIndex
			if (i == contact.manifold.ptsA.size() - 1)
				constraintIndex += contact.manifold.ptsA.size();
		}
	}
}