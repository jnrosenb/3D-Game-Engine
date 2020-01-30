///HEADER STUFF


#include "RigidbodyComponent.h"
#include "GameObject.h"

//To get and set position for physic objects
#include "TransformComponent.h"
#include "AnimationComponent.h"

//To get model, and object space OBB
#include "RendererComponent.h"
#include "Model.h"
#include "Meshes/DebugBox.h"
#include "Meshes/DebugSphere.h"
#include "Meshes/DebugRay.h"
#include "Shader.h"

#include "Quaternion.h"

//FOR NOW (global accesor)
#include "PhysicsManager.h"
extern PhysicsManager *physicsMgr;
#include "Renderer.h"
#include "DeferredRenderer.h"
#include "Camera.h"
extern Renderer *renderer;
#include "InputManager.h"
extern InputManager *inputMgr;



RigidbodyComponent::RigidbodyComponent(GameObject *owner) :
	BaseComponent(owner, COMPONENT_TYPES::RIGIDBODY)
{
	//TEMPORARY FLAG; DELETE WHEN ADDING SCRIPTING COMPS
	isPlayer = false;

	//Initial setup of params
	Params  = std::vector<glm::vec4>(4);
	dParams = std::vector<glm::vec4>(4);
}


RigidbodyComponent::~RigidbodyComponent()
{
	std::cout << "Destroying RigidbodyComponent" << std::endl;

	if (debugMesh)
		delete debugMesh;
	if (debugPointMesh)
		delete debugPointMesh;
	if (debugRayMesh)
		delete debugRayMesh;

	delete debugShader;
	delete debugShaderLine;
}


void RigidbodyComponent::DeserializeInit()
{
	//Register this rgbdy on physics manager
	physicsMgr->RegisterRigidbody(this);

	//Setup initial params
	Transform *T = this->m_owner->GetComponent<Transform>();
	
	Params[0] = T->GetPosition();
	Params[1] = glm::vec4(0);
	Params[2] = glm::vec4(0);
	Params[3] = glm::vec4(0);

	dParams[0] = glm::vec4(0);
	dParams[1] = glm::vec4(0);
	dParams[2] = glm::vec4(0);
	dParams[3] = glm::vec4(0);

	//Start with zero forces
	ResetForces();

	//Rigidbody parameters
	invMass = (mass == 0) ? 0.0f : 1.0f / mass;
}


void RigidbodyComponent::Begin()
{
	//Setup OBB stuff
	ColliderSetup();
}


//Offset is already ContactWorldPos - COMWorldPos
void RigidbodyComponent::ApplyForce(glm::vec3 const& F, 
	glm::vec3 const& offset)
{
	this->Force += glm::vec4(F.x, F.y, F.z, 0);

	//For now, leave torque alone
	glm::vec3 t = glm::cross(offset, F);
	this->Torque += glm::vec4(t.x, t.y, t.z, 0.0f);
}


void RigidbodyComponent::ColliderSetup()
{
	Transform *transformComp = this->m_owner->GetComponent<Transform>();
	Render *rendererComp = this->m_owner->GetComponent<Render>();
	if (rendererComp && transformComp)
	{
		Model *model = rendererComp->GetModel();
		aabb = model->GetAABB();
		DebugDrawSetup(aabb);

		//More setup
		float halfwidth = (aabb.max.x - aabb.min.x) * 0.5f;
		float halfheight = (aabb.max.y - aabb.min.y) * 0.5f;
		float halfdepth = (aabb.max.z - aabb.min.z) * 0.5f;
		glm::mat4 I(1);
		I[0][0] = (1 / 12.0f) * mass * (halfheight*halfheight + halfdepth*halfdepth);
		I[1][1] = (1 / 12.0f) * mass * (halfwidth*halfwidth + halfdepth*halfdepth);
		I[2][2] = (1 / 12.0f) * mass * (halfheight*halfheight + halfwidth*halfwidth);
		IbodyInv = glm::inverse(I);

		//Based on AABB stuff and scaling of transform, construct OBB dimension vector (radius)
		OBBRadius = glm::vec3(
			halfwidth * transformComp->GetScale().x,
			halfheight * transformComp->GetScale().y,
			halfdepth * transformComp->GetScale().z
		);
	}
}


void RigidbodyComponent::DebugDrawSetup(AABB const& aabb)
{
	//USING OUTLINE
	/*
	std::vector<glm::vec4> points;
	points.push_back(glm::vec4(aabb.max.x, aabb.max.y, aabb.max.z, 1.0f)); // n_topRight
	points.push_back(glm::vec4(aabb.min.x, aabb.max.y, aabb.max.z, 1.0f)); // n_topLeft    
	points.push_back(glm::vec4(aabb.min.x, aabb.min.y, aabb.max.z, 1.0f)); // n_bottomLeft 
	points.push_back(glm::vec4(aabb.max.x, aabb.min.y, aabb.max.z, 1.0f)); // n_bottomRight
	points.push_back(glm::vec4(aabb.max.x, aabb.max.y, aabb.max.z, 1.0f)); // n_topRight   
	points.push_back(glm::vec4(aabb.max.x, aabb.max.y, aabb.min.z, 1.0f)); // f_topRight   
	points.push_back(glm::vec4(aabb.min.x, aabb.max.y, aabb.min.z, 1.0f)); // f_topLeft    
	points.push_back(glm::vec4(aabb.min.x, aabb.max.y, aabb.max.z, 1.0f)); // n_topLeft    
	points.push_back(glm::vec4(aabb.min.x, aabb.max.y, aabb.min.z, 1.0f)); // f_topLeft    
	points.push_back(glm::vec4(aabb.min.x, aabb.min.y, aabb.min.z, 1.0f)); // f_bottomLeft 
	points.push_back(glm::vec4(aabb.min.x, aabb.min.y, aabb.max.z, 1.0f)); // n_bottomLeft 
	points.push_back(glm::vec4(aabb.min.x, aabb.min.y, aabb.min.z, 1.0f)); // f_bottomLeft 
	points.push_back(glm::vec4(aabb.max.x, aabb.min.y, aabb.min.z, 1.0f)); // f_bottomRight
	points.push_back(glm::vec4(aabb.max.x, aabb.min.y, aabb.max.z, 1.0f)); // n_bottomRight
	points.push_back(glm::vec4(aabb.max.x, aabb.min.y, aabb.min.z, 1.0f)); // f_bottomRight
	points.push_back(glm::vec4(aabb.max.x, aabb.max.y, aabb.min.z, 1.0f)); // f_topRight
	
	//Create new mesh with these points
	this->debugMesh = new DebugBox(points);

	this->debugShader = new Shader("Line.vert", "Line.frag");
	this->debugShader->BindUniformBlock("test_gUBlock", 1);
	//*/

	//USING SEMI TRANSPARENT BLOCK
	glm::vec3 size = {aabb.max - aabb.min};
	size = glm::vec3(std::fabs(size.x), std::fabs(size.y), std::fabs(size.z));
	
	this->debugMesh = new DebugBoxFilled(size);
	this->debugPointMesh = new DebugSphereOutline(8);
	this->debugRayMesh = new DebugRay();
	
	this->debugShader = new Shader("Solid.vert", "Solid.frag");
	this->debugShader->BindUniformBlock("test_gUBlock", 1);
	this->debugShaderLine = new Shader("Line.vert", "Line.frag");
	this->debugShaderLine->BindUniformBlock("test_gUBlock", 1);
}


void RigidbodyComponent::Update(float dt)
{
	handleInput(dt);
}


//vector<vec4>  Params   x(t) - Rvo   - P - L
//vector<vec4> dParams   v(t) - w*Rvo - F - T
void RigidbodyComponent::PhysicsUpdate(float dt)
{
	//Initial stuff we need
	Transform *T = this->m_owner->GetComponent<Transform>();

	/////////////////////////////////
	////    LINEAR STUFF         ////
	/////////////////////////////////
	if (Force != glm::vec4(0) || dParams[0] != glm::vec4(0))
	{
		glm::vec4 accel = Force * invMass;

		//Integrate for velocity (of center of mass)
		glm::vec4& linearVel = dParams[0];
		linearVel = linearVel + accel * dt;

		//For now, damp the velocity to fake friction
		DampVelocity(linearVel);

		//Integrate for position
		glm::vec4 position = T->GetPosition();//this->Params[0];
		position = position + linearVel * dt;
		Params[0] = position;

		T->translate(linearVel * dt);
	}

	/////////////////////////////////
	////    ANGULAR STUFF        ////
	/////////////////////////////////
	if (Torque != glm::vec4(0) || Params[3] != glm::vec4(0))
	{
		//Get rotation matrix, rotation quaternion, and Iinv in world space
		glm::mat4 const& R = T->GetRotationMatrix();
		AuxMath::Quaternion const& q0 = T->GetRotationQuaternion();
		glm::mat4 Iinv = R * IbodyInv * glm::transpose(R);

		//Get previous angular momentum, integrate torque, get w
		///--Param[3] as w--
		glm::vec4 angAccel = Iinv * Torque;
		Params[3] = Params[3] + angAccel * dt;
		DampVelocity(Params[3], 0.005f);
		AuxMath::Quaternion w(Params[3]);
		w.s = 0.0f;
		//w.print("Angular velocity: ");

		///--Param[3] as L--
		//glm::vec4& L = Params[3];
		//L = L + Torque * dt;
		//AuxMath::Quaternion w(Iinv * L); //Get angular velocity
		//w.s = 0.0f;

		///Get quaternion velocity (slope vector), then integrate
		//q0.print("Q0 is           : ");
		AuxMath::Quaternion qvel = 0.5f * w * q0;
		//qvel.print("QVel            : ");
		AuxMath::Quaternion q = q0 + dt * qvel;

		//Find difference between this q and previous
		q = q0.Inverse() * q;
		q.Normalize();

		//DEDUCE THIS CODE (TAKEN ONLINE)-------------------------------
		float yaw, pitch, roll;
		// pitch (x-axis rotation)
		float sinr_cosp = 2 * (q.s * q.x + q.y * q.z);
		float cosr_cosp = 1 - 2 * (q.x * q.x + q.y * q.y);
		pitch = std::atan2(sinr_cosp, cosr_cosp);
		// yaw (y-axis rotation)
		float sinp = 2 * (q.s * q.y - q.z * q.x);
		if (std::abs(sinp) >= 1)
			yaw = std::copysign(M_PI / 2, sinp); // use 90 degrees if out of range
		// roll (z-axis rotation)
		float siny_cosp = 2 * (q.s * q.z + q.x * q.y);
		float cosy_cosp = 1 - 2 * (q.y * q.y + q.z * q.z);
		roll = std::atan2(siny_cosp, cosy_cosp);
		//--------------------------------------------------------------

		T->rotate(pitch, yaw, roll);
		//std::cout << ">> pitch: " << pitch << ", yaw: " << yaw << ", roll: " << roll << std::endl;
		//std::cout << "----------------------------" << std::endl;
	}

	/////////////////////////////////
	////  AT THE END OF ALL      ////
	/////////////////////////////////
	ResetForces();
}


//TEMPORARY AUX FUNCTION
void RigidbodyComponent::DampVelocity(glm::vec4& vel, float damping)
{
	vel = vel * (1.0f - damping);
	vel.x = (fabs(vel.x) <= 0.001f) ? 0.0 : vel.x;
	vel.y = (fabs(vel.y) <= 0.001f) ? 0.0 : vel.y;
	vel.z = (fabs(vel.z) <= 0.001f) ? 0.0 : vel.z;
}


void RigidbodyComponent::ResetForces() 
{
	//For debug draw purposes
	this->DebugForce = Force;

	this->Force = glm::vec4(0);
	this->Torque = glm::vec4(0);
}


void RigidbodyComponent::Draw()
{
	AnimationComponent *animComp = this->m_owner->GetComponent<AnimationComponent>();
	Transform *T = this->m_owner->GetComponent<Transform>();
	
	DrawDebugData data = {};
	data.diffuseColor = { 0, 0, 1, 0.5f };
	data.model = T->GetModel();
	data.mesh = debugMesh;
	data.shader = debugShader;
	//Bones experiment SHITTY WAY - NOT USING BONES FOR THIS
	data.BoneTransformations = (animComp == 0) ? 0 : &(animComp->BoneTransformations);
	renderer->QueueForDebugDraw(data);


	//CONTACT POINT TEST
	DrawDebugData data2 = {};
	data2.diffuseColor = { 0, 1, 0, 1.0f };
	data2.model = T->GetModel();
	data2.mesh = debugPointMesh;
	data2.shader = debugShader;
	///renderer->QueueForDebugDraw(data2);


	//FORCE DRAWING TEST
	if (DebugForce.x != 0.0f && DebugForce.z != 0.0f)
		int a = 123;
	glm::mat4 rayModel(1);
	float f = glm::length(DebugForce);
	glm::vec4 forceDir = DebugForce / f;
	//Rotation (gonna only rotate x axis)
	rayModel[0] = forceDir * 5.0f;
	//Translation
	rayModel[3] = T->GetPosition();
	rayModel[3][3] = 1.0f;
	//------------------
	DrawDebugData data3 = {};
	data3.diffuseColor = { 0, 1, 0, 1.0f };
	data3.model = rayModel;
	data3.mesh = debugRayMesh;
	data3.shader = debugShaderLine;
	renderer->QueueForDebugDraw(data3);
}


glm::vec3 RigidbodyComponent::GetOBBRadiusVector() const 
{
	return this->OBBRadius;
}


////////////////////////////////////
////   INPUT MANAGEMENT         ////
////////////////////////////////////
void RigidbodyComponent::handleInput(float dt)
{
	if (!isPlayer) 
		return;

	Transform *T = this->m_owner->GetComponent<Transform>();
	float moveSpeed = 2000.0f * dt;
	DeferredRenderer *deferred = static_cast<DeferredRenderer*>(renderer);
	glm::vec3 fwd = deferred->GetCurrentCamera()->getLook();
	fwd.y = 0.0f;
	glm::vec3 rup = {0, 1, 0};
	glm::vec3 right = glm::cross(fwd, rup);

	if (inputMgr->getKeyPress(SDL_SCANCODE_RIGHT))
	{
		ApplyForce(moveSpeed * right, { 0,1,0 });
	}
	if (inputMgr->getKeyPress(SDL_SCANCODE_LEFT))
	{
		ApplyForce( -moveSpeed * right, { 0,1,0 });
	}
	if (inputMgr->getKeyPress(SDL_SCANCODE_UP))
	{
		ApplyForce( moveSpeed * fwd, { 0,1,0 });
	}
	if (inputMgr->getKeyPress(SDL_SCANCODE_DOWN))
	{
		ApplyForce( -moveSpeed * fwd, { 0,1,0 });
	}

	//ROTATION TESTS
	if (inputMgr->getKeyPress(SDL_SCANCODE_J))
	{
		ApplyForce(moveSpeed * right, { 0,1,0 });
		ApplyForce(moveSpeed * right, { 0,-1,0 });
	}
	if (inputMgr->getKeyPress(SDL_SCANCODE_F))
	{
		ApplyForce(moveSpeed * right, { 0,1,0 });
	}
	if (inputMgr->getKeyPress(SDL_SCANCODE_V))
	{
		ApplyForce(-moveSpeed * right, { 0,1,0 });
	}
	if (inputMgr->getKeyPress(SDL_SCANCODE_G))
	{
		ApplyForce(moveSpeed * fwd, { 0,1,0 });
	}
	if (inputMgr->getKeyPress(SDL_SCANCODE_H))
	{
		ApplyForce(moveSpeed * right, { 0,0,1 });
	}
}