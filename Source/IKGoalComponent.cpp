///HEADER STUFF

#include <iostream>
#include "IKGoalComponent.h"
#include "GameObject.h"

#include "Quaternion.h"
#include "Affine.h"

#include "TransformComponent.h"
#include "AnimationComponent.h"
#include "RendererComponent.h"
#include "Camera.h"
#include "Model.h"
#include "Quad.h"

// TODO Temporary (while no world exists)
#include "InputManager.h"
extern InputManager *inputMgr;

// TODO Temporary (while no world exists)
#include "DeferredRenderer.h"
extern Renderer *renderer;


#define EPSILON_IK 0.0000001f
#define TOLERANCE 7.5f
#define PI 3.14159f



IKGoalComponent::IKGoalComponent(GameObject *owner) :
	BaseComponent(owner, COMPONENT_TYPES::IK_GOAL)
{
	goalModel = nullptr;
}

IKGoalComponent::~IKGoalComponent()
{
	std::cout << "Destroying IKGoal Component" << std::endl;

	//Delete mesh used for drawing goal
	if (goalModel)
		delete goalModel;
}


void IKGoalComponent::DeserializeInit()
{
	// TODO - This is temporary, should be stuff the anim comp has access to
	Transform *T = this->m_owner->GetComponent<Transform>();
	if (T) 
	{
		this->Goal = T->GetPosition();
	}

	//Other vars setup
	resolvingIkSystem = false; 
	timeElapsed = 0.0f;

	//For drawing the goal
	this->goalModel = new Model(abs_path_prefix + "Sphere.fbx");

	YAXIS = 70.0f;
}


void IKGoalComponent::Update(float dt)
{
	//TODO temporal
	handleInput(dt);
}


void IKGoalComponent::Calculate_Angular_Velocities(std::vector<glm::vec3>& J, glm::vec3& V, std::vector<float>& w)
{
	// J is a jacobian in which every vec3 is a row. We can use it here as if each vec3 was a column
	// W is what we want to output, angular velocities

	for (int row = 0; row < J.size(); ++row) 
	{
		w[row] = glm::dot(J[row], V);
	}
}


void IKGoalComponent::LateUpdate(float dt) 
{
	//Transform GOAL into root (obj) space vec G
	Transform *T = m_owner->GetComponent<Transform>();
	if (T == nullptr) return;
	glm::vec3 G = glm::inverse(T->GetModel()) * glm::vec4(Goal.x, Goal.y, Goal.z, 1);
	G *= 2;

	//TODO - Update code for the IKGoal
	if (resolvingIkSystem)
	{
		// TODO - Cache this so we dont have to get it every frame
		Render *renderComp = this->m_owner->GetComponent<Render>();
		if (renderComp == nullptr) return;
		boneMap = &renderComp->model->boneMap;

		//BEGIN ALGORITHM============================================
		Bone& EE = (*boneMap)[this->endEffector];
		glm::vec3 P = glm::vec3(EE.accumTransformation[3]); //We get P in root space

		//CHECK IF WE NEED TO EXIT THE IK CALCULATIONS
		if (glm::distance(G, P) <= TOLERANCE)
		{
			resolvingIkSystem = false;
			AnimationComponent *AnimComp = m_owner->GetComponent<AnimationComponent>();
			if (AnimComp == nullptr) return;
			AnimComp->animSpeed = 1.0f;
			std::cout << "IK SYSTEM INTEGRATION DONE -- BY REACHING POS" << std::endl;
		}

		//For now, jacobian will be a vector (column) of vec3 (rows)
		std::vector<glm::vec3> J;
		std::vector<IK_Joint> joints; //TODO - Check existance


		//  >> O(n)
		//Get the number of joints we are gonna affect 
		//(depends on wether root is closer than the jointDepth)
		Bone *currentBone = &EE;
		for (int i = 0; i < this->jointDepth; ++i)
		{
			if (currentBone->parent == "")
			{
				jointDepth = i;
				break;
			}
			else
			{
				//Get Parent of currJoint, and store (only joint not stored will be EE)
				Bone& parent = (*boneMap)[currentBone->parent];
				currentBone = &parent;

				//WE ARE IN ROOT SPACE (obj space)
				IK_Joint joint(currentBone);
				joint.rootPos = glm::vec3(currentBone->accumTransformation[3]);

				//Jacobian calcs. Per joint, we gen three vec3 and add them to J as columns		  
				glm::vec3 from_joint_to_EE = (P - joint.rootPos);
				glm::vec3 diff_cross_x = glm::cross(glm::vec3(1, 0, 0), from_joint_to_EE);
				glm::vec3 diff_cross_y = glm::cross(glm::vec3(0, 1, 0), from_joint_to_EE);
				glm::vec3 diff_cross_z = glm::cross(glm::vec3(0, 0, 1), from_joint_to_EE);
				J.push_back(diff_cross_x);
				J.push_back(diff_cross_y);
				J.push_back(diff_cross_z);

				joints.push_back(joint);
			}
		}


		//Now get the current EndEffector's velocity vector (in root space)
		glm::vec3 dP = (G - P);
		glm::vec3 V = dP;// / 0.016f; // TODO - CHANGE TO DT AS SOON AS FINISH DEBUGGING*************
		V = glm::vec3(V.x/steps, V.y / steps, V.z / steps);
		std::cout << ">>> Vector between EE and goal:  " << dP.x << ", " << dP.y << ", " << dP.z << "-." << std::endl;

		//These are the targets to fill
		int n = jointDepth * 3;
		std::vector<float> w(n);

		//Using our J transposed and V vector, get angular velocity
		Calculate_Angular_Velocities(J, V, w);

		//Write back to joint. Start from closest to Root, and move towards EE
		for (int i = 0; i < joints.size(); ++i)
		{
			//Get the joint
			std::string jointName = joints[i].joint->name;
			Bone& joint = (*boneMap)[jointName];

			//1- Get the three angles per joint (IN ROOT SPACE)
			///glm::mat3 rot = joint.nodeTransformation;
			glm::mat3 rot;
			if (joint.IKUseVQS)
				rot = joint.vqs;
			else 
				rot = joint.nodeTransformation;

			//2- Add the dq
			float qx = std::atan2f(rot[1][2], rot[2][2]);
			float qy = std::atan2f(-rot[0][2], std::sqrtf(rot[1][2] * rot[1][2] + rot[2][2] * rot[2][2]));
			float qz = std::atan2f(rot[0][1], rot[0][0]);
			qx += dt * w[i * 3];
			qy += dt * w[i * 3 + 1];
			qz += dt * w[i * 3 + 2];
			
			
			//These angles are not in each joint or parent's space. The are all respect to the root
			//We need to do some sort of transformation to, from the root up, make the right bones move
			float degreeX = glm::degrees(qx);
			float degreeY = glm::degrees(qy);
			float degreeZ = glm::degrees(qz);

			glm::mat4 Rx = glm::mat4(1);
			glm::mat4 Ry = glm::mat4(1);
			glm::mat4 Rz = glm::mat4(1);
			
			//temp------------------------------------
			/// if (degreeY <= YAXIS && degreeY >= -YAXIS) 
			/// {
			/// 	Ry = AuxMath::Quaternion::QuaternionFromAA(degreeY, glm::vec3(0, 1, 0)).GetRotationMatrix();
			/// 	glm::mat4 R = Rz * Ry * Rx;
			/// 
			/// 	if (joint.IKUseVQS)
			/// 	{
			/// 		R[3] = joint.vqs[3];
			/// 		joint.vqs = R;
			/// 		joint.IKUseVQS = false;
			/// 	}
			/// 	else
			/// 	{
			/// 		R[3] = joint.nodeTransformation[3];
			/// 		joint.nodeTransformation = R;
			/// 	}
			/// }

			//if (degreeX <= 30 && degreeX >= -30)
				Rx = AuxMath::Quaternion::QuaternionFromAA(degreeX, glm::vec3(1, 0, 0)).GetRotationMatrix();
			//if (degreeY <= YAXIS && degreeY >= -YAXIS)
				Ry = AuxMath::Quaternion::QuaternionFromAA(degreeY, glm::vec3(0, 1, 0)).GetRotationMatrix();
			//if (degreeZ <= 30 && degreeZ >= -30)
				Rz = AuxMath::Quaternion::QuaternionFromAA(degreeZ, glm::vec3(0, 0, 1)).GetRotationMatrix();
			glm::mat4 R = Rz * Ry * Rx;
			
			
			///R[3] = joint.nodeTransformation[3];
			///joint.nodeTransformation = R;
			if (joint.IKUseVQS)
			{
				R[3] = joint.vqs[3];
				joint.vqs = R;
				joint.IKUseVQS = false;
			}
			else
			{
				R[3] = joint.nodeTransformation[3];
				joint.nodeTransformation = R;
			}
		}
	}

	//Render the IKGoal
	PassDrawDataToRenderer();
}


void IKGoalComponent::PassDrawDataToRenderer()
{
	DeferredRenderer *defRenderer = static_cast<DeferredRenderer*>(renderer);
	if (defRenderer)
	{
		//Model for the controlpoint
		float radius = 0.5f;
		glm::mat4 model = glm::mat4(1);
		model[3] = glm::vec4(Goal, 1.0f);
		model[0][0] = radius;
		model[1][1] = radius;
		model[2][2] = radius;

		// Pack it maybe on a struct
		DrawData data = {};
		data.meshes = &goalModel->meshes;
		data.model = model;
		data.normalsModel = model;
		data.roughness = 0.2F;
		data.metallic = 0.5f;
		data.normalsModel = model;
		data.diffuseColor = glm::vec4(0, 1, 0, 1);
		data.specularColor = glm::vec4(1, 1, 1, 1);

		// Pass it to renderer's queue
		defRenderer->QueueForDraw(data);
	}
}


glm::vec3 const& IKGoalComponent::GetWorldCoordGoal() const
{
	return this->Goal;
}


////////////////////////////////
////	HANDLE INPUT	    ////
////////////////////////////////
void IKGoalComponent::handleInput(float dt)
{
	Transform *T = m_owner->GetComponent<Transform>();
	if (T == nullptr)
		return;

	if (inputMgr->getKeyTrigger(SDL_SCANCODE_RETURN))
	{
		AnimationComponent *AnimComp = m_owner->GetComponent<AnimationComponent>();
		if (AnimComp == nullptr) return;
		
		if (AnimComp->animSpeed == 0.0f) 
			AnimComp->animSpeed = 1.0f;
		else if (AnimComp->animSpeed > 0.0f) 
			AnimComp->animSpeed = 0.0f;

		resolvingIkSystem = !resolvingIkSystem;
		std::cout << "STARTING TO SOLVE IK SYSTEM" << std::endl;
	}


	if (inputMgr->getKeyPress(SDL_SCANCODE_Y))
	{
		--YAXIS;
	}
	if (inputMgr->getKeyPress(SDL_SCANCODE_U))
	{
		++YAXIS;
	}

	//Get camera forward and right vectors for movement
	DeferredRenderer *defRenderer = static_cast<DeferredRenderer*>(renderer);
	if (defRenderer) 
	{
		Camera *camera = defRenderer->GetCurrentCamera();
		glm::vec4 look = camera->getLook();
		glm::vec3 forward = glm::normalize(glm::vec3(look.x, 0.0f, look.z));
		glm::vec3 right = glm::cross(forward, glm::vec3(0, 1, 0));
		float speed = 15.0f;

		//Movement code for the object under this component
		if (inputMgr->getKeyPress(SDL_SCANCODE_UP))
		{
			Goal += speed * dt * forward;
		}
		if (inputMgr->getKeyPress(SDL_SCANCODE_DOWN))
		{
			Goal -= speed * dt * forward;
		}
		if (inputMgr->getKeyPress(SDL_SCANCODE_RIGHT))
		{
			Goal += speed * dt * right;
		}
		if (inputMgr->getKeyPress(SDL_SCANCODE_LEFT))
		{
			Goal -= speed * dt * right;
		}
		if (inputMgr->getKeyPress(SDL_SCANCODE_PAGEUP))
		{
			Goal += speed * dt * glm::vec3(0, 1, 0);
		}
		if (inputMgr->getKeyPress(SDL_SCANCODE_PAGEDOWN))
		{
			Goal -= speed * dt * glm::vec3(0, 1, 0);
		}
	}
}


//OLD WAY OF IK JACOBIAN CALC
/*
//TODO - Update code for the IKGoal
	if (resolvingIkSystem)
	{
		//Temporary, to exit the thing after two seconds******************************************EXIT
		//////////////////////////////////////////////////////////////////////////////////////////////////timeElapsed += dt;
		if (timeElapsed >= 4.0f)
		{
			resolvingIkSystem = false;
			timeElapsed = 0.0f;
			std::cout << "IK SYSTEM INTEGRATION DONE" << std::endl;
		}

		// TODO - Cache this so we dont have to get it every frame
		Render *renderComp = this->m_owner->GetComponent<Render>();
		if (renderComp == nullptr) return;
		boneMap = &renderComp->model->boneMap;


		//BEGIN ALGORITHM============================================
		Bone& EE = (*boneMap)[this->endEffector];
		glm::vec3 P = glm::vec3(EE.nodeTransformation[3]);

		//For now, jacobian will be a vector (column) of vec3 (rows)
		std::vector<glm::vec3> J;
		std::vector<IK_Joint> joints;

		//Get the number of joints we are gonna affect
		//(depends on wether root is closer than the jointDepth)
		Bone *currentBone = &EE;
		int i = 0;
		for (; i < this->jointDepth; ++i)
		{
			if (currentBone->parent == "")
				break;
			else
			{
				//Get the parent of the current bone
				Bone& parent = (*boneMap)[currentBone->parent];
				currentBone = &parent;

				//Push back. This will later be used to transverse the joints other way around and get their world position from root up
				joints.push_back(IK_Joint(currentBone));

				//Accumulate the world pos for end effector
				P += glm::vec3(parent.nodeTransformation[3]);
			}
		}


		//Second way of exiting, if the distance between
		//goal and End Effector's position is less than tolerance*********************************EXIT
		if (glm::distance(G, P) <= EPSILON)
		{
			resolvingIkSystem = false;
			timeElapsed = 0.0f;
			std::cout << "IK SYSTEM INTEGRATION DONE -- BY REACHING POS" << std::endl;
		}


		//Save the number of joints and angles we are storing
		this->jointDepth = i;
		int n = jointDepth * 3;


		//Now get the current EndEffector's velocity vector
		glm::vec3 dP = G - P;
		glm::vec3 V = dP / 0.016f; // TODO - CHANGE TO DT AS SOON AS FINISH DEBUGGING*************


		//Now we can go from current to top to get the world positions and calculate jacobian
		glm::vec3 worldPosAccumulator(0);
		for (int i = joints.size() - 1; i >= 0; --i)
		{
			//Get the joint
			Bone *currJoint = joints[i].joint;

			//Let's say the origin of the world is the origin of the final joint we
			//affect (true if go through to the root)
			glm::vec3 jointRelativePos = glm::vec3(currJoint->nodeTransformation[3]);

			worldPosAccumulator += jointRelativePos;
			joints[i].rootPos = worldPosAccumulator;

			//Jacobian shit. Per joint, we generate three vec3 and push them to jacobian
			glm::vec3 from_joint_to_EE = (P - worldPosAccumulator);
			glm::vec3 diff_cross_x = glm::cross(glm::vec3(1, 0, 0), from_joint_to_EE);
			glm::vec3 diff_cross_y = glm::cross(glm::vec3(0, 1, 0), from_joint_to_EE);
			glm::vec3 diff_cross_z = glm::cross(glm::vec3(0, 0, 1), from_joint_to_EE);
			J.push_back(diff_cross_x);
			J.push_back(diff_cross_y);
			J.push_back(diff_cross_z);
		}

		//These are the targets to fill
		std::vector<float> dq(n);
		std::vector<float> w(n);

		//Finally calculate stuff using this jacobian
		//STEPS WE NEED TO DO
		// 1 - TRANSPOSE THE JACOBIAN (gonna turn into nx3, so use vector of vector)
		//			- Not really necessary, since we can just make an algorithm that treats J as if it was already transposed
		// 2 - Multiply it against the V to get a n vector of w
		Calculate_Angular_Velocities(J, P, w);


		// 3 - From there, get vector of q
		for (int i = 0; i < w.size(); ++i)
			dq[i] = w[i] * 0.016f; // TODO - CHANGE TO DT AS SOON AS FINISH DEBUGGING*************


		// 4 - Now, SOMEHOW write this value back to the bones transformation matrices to get an actual effect
		//     (you need to affect the bones in the map, since others may be copies by value)
		for (int i = joints.size() - 1; i >= 0; --i)
		{
			//Get the joint
			std::string jointName = joints[i].joint->name;
			Bone& joint = (*boneMap)[jointName];

			//We have to now
			//1- Get the three angles per joint
			glm::mat3 rot = joint.nodeTransformation;
			float qx = std::atan2f( rot[1][2], rot[2][2]);
			float qy = std::atan2f(-rot[0][2], std::sqrtf(rot[1][2]*rot[1][2] + rot[2][2]*rot[2][2]));
			float qz = std::atan2f( rot[0][1], rot[0][0]);
			//2- Add the dq
			qx += dq[i * 3];
			qy += dq[i * 3 + 1];
			qz += dq[i * 3 + 2];
			//3- Re write them into the matrix transformation of the boneMap
			glm::mat4 Rx = AuxMath::Quaternion::QuaternionFromAA(qx, glm::vec3(1, 0, 0)).GetRotationMatrix();
			glm::mat4 Ry = AuxMath::Quaternion::QuaternionFromAA(qy, glm::vec3(0, 1, 0)).GetRotationMatrix();
			glm::mat4 Rz = AuxMath::Quaternion::QuaternionFromAA(qz, glm::vec3(0, 0, 1)).GetRotationMatrix();
			glm::mat4 FinalRot = Rx * Ry * Rz;
			joint.nodeTransformation = FinalRot;
			int a = 123;
		}

		//Supposing we have W and V, now we need to multiply jacobian and shit
		int a = 123;
	}
*/


