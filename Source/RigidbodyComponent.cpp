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
	prevAngAccel = glm::vec4(0);
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

	if (debugRay)
		delete debugRay;

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
	L = glm::vec4(0);
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

	this->debugRay = new Model("Vector.fbx");
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
	Params[0] = T->GetPosition();

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
		DampVelocity(linearVel, 0.01f);

		//Integrate for position
		glm::vec4& position = Params[0];
		position = position + linearVel * dt;

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

		///--Param[3] as w--
		//glm::vec4 angAccel = Iinv * Torque;
		//Params[3] = Params[3] + angAccel * dt;
		//DampVelocity(Params[3], 0.25f);
		//AuxMath::Quaternion w(Params[3]);
		
		///--Param[3] as L--
		L = L + Torque * dt;
		DampVelocity(L, 0.1f);
		Params[3] = Iinv * L;
		AuxMath::Quaternion w(Params[3]);

		///Get quaternion velocity (slope vector), then integrate
		AuxMath::Quaternion qvel = 0.5f * (w * q0);
		AuxMath::Quaternion q = q0 + (dt * qvel);
		q = q0.Conjugate() * q;
		q = q.Normalize();

		//T->rotateWorld(q);
		T->rotate(q);
	}
	else 
	{
		prevAngAccel = glm::vec4(0);
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

	////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////

	//FORCE DRAWING TEST
	glm::mat4 rayModel(1);
	///float f = glm::length(DebugForce);
	///glm::vec4 forceDir = DebugForce / f;
	glm::vec4 forceDir = glm::normalize(this->dParams[0]);
	//Rotation (gonna only rotate x axis)
	float scale = 3.0f;
	glm::vec3 fwd = glm::vec3(forceDir);
	glm::vec3 right = glm::cross(fwd, { 0,1,0 });
	glm::vec3 up = glm::cross(right, fwd);
	glm::mat3 R = { fwd, up, right };//{ right, up, fwd }; 
	R = R * glm::mat3(scale);
	rayModel[0] = glm::vec4(R[0].x, R[0].y, R[0].z, 0.0f);
	rayModel[1] = glm::vec4(R[1].x, R[1].y, R[1].z, 0.0f);
	rayModel[2] = glm::vec4(R[2].x, R[2].y, R[2].z, 0.0f);
	//Translation
	rayModel[3] = T->GetPosition();
	//Final diagonal
	rayModel[3][3] = 1.0f;
	//------------------
	DrawDebugData data3 = {};
	data3.diffuseColor = { 0, 1, 1, 1.0f };
	data3.model = rayModel;
	data3.mesh = debugRay->meshes[0];//debugRayMesh;
	data3.shader = debugShader;//debugShaderLine;
	renderer->QueueForDebugDraw(data3);

	////////////////////////////////////////////////
	////////////////////////////////////////////////


	//Local axis drawing test (FORWARD)
	glm::mat4 fwdModel(1);
	glm::vec4 fwdDir = T->GetRotationMatrix()[2];
	//Rotation (gonna only rotate x axis)
	fwd = glm::vec3(fwdDir);
	right = glm::cross(fwd, { 0,1,0 });
	up = glm::cross(right, fwd);
	glm::mat3 Rot = { fwd, up, right };
	Rot = Rot * glm::mat3(3.0f);
	fwdModel[0] = glm::vec4(Rot[0].x, Rot[0].y, Rot[0].z, 0.0f);
	fwdModel[1] = glm::vec4(Rot[1].x, Rot[1].y, Rot[1].z, 0.0f);
	fwdModel[2] = glm::vec4(Rot[2].x, Rot[2].y, Rot[2].z, 0.0f);
	//Translation
	fwdModel[3] = T->GetPosition();
	fwdModel[3][3] = 1.0f;
	//------------------
	data3 = {};
	data3.diffuseColor = { 0, 0, 1, 1.0f };
	data3.model = fwdModel;
	data3.mesh = debugRay->meshes[0];;
	data3.shader = debugShader;;
	renderer->QueueForDebugDraw(data3);

	//Local axis drawing test (UP)
	fwdModel = glm::mat4(1);
	fwdDir = T->GetRotationMatrix()[1];
	//Rotation (gonna only rotate x axis)
	fwd = glm::vec3(fwdDir);
	right = T->GetRotationMatrix()[0]; //glm::cross(fwd, { 0,1,0 });
	up = T->GetRotationMatrix()[2];    //glm::cross(right, fwd);
	Rot = { fwd, up, right };
	Rot = Rot * glm::mat3(3.0f);
	fwdModel[0] = glm::vec4(Rot[0].x, Rot[0].y, Rot[0].z, 0.0f);
	fwdModel[1] = glm::vec4(Rot[1].x, Rot[1].y, Rot[1].z, 0.0f);
	fwdModel[2] = glm::vec4(Rot[2].x, Rot[2].y, Rot[2].z, 0.0f);
	//Translation
	fwdModel[3] = T->GetPosition();
	fwdModel[3][3] = 1.0f;
	//------------------
	data3 = {};
	data3.diffuseColor = { 1, 0, 0, 1.0f };
	data3.model = fwdModel;
	data3.mesh = debugRay->meshes[0];;
	data3.shader = debugShader;;
	renderer->QueueForDebugDraw(data3);

	//Local axis drawing test (RIGHT)
	fwdModel = glm::mat4(1);
	fwdDir = T->GetRotationMatrix()[0];
	//Rotation (gonna only rotate x axis)
	fwd = glm::vec3(fwdDir);
	right = glm::cross(fwd, { 0,1,0 });
	up = glm::cross(right, fwd);
	Rot = { fwd, up, right };
	Rot = Rot * glm::mat3(3.0f);
	fwdModel[0] = glm::vec4(Rot[0].x, Rot[0].y, Rot[0].z, 0.0f);
	fwdModel[1] = glm::vec4(Rot[1].x, Rot[1].y, Rot[1].z, 0.0f);
	fwdModel[2] = glm::vec4(Rot[2].x, Rot[2].y, Rot[2].z, 0.0f);
	//Translation
	fwdModel[3] = T->GetPosition();
	fwdModel[3][3] = 1.0f;
	//------------------
	data3 = {};
	data3.diffuseColor = { 0, 1, 0, 1.0f };
	data3.model = fwdModel;
	data3.mesh = debugRay->meshes[0];;
	data3.shader = debugShader;;
	renderer->QueueForDebugDraw(data3);
}


glm::vec3 RigidbodyComponent::GetOBBRadiusVector() const 
{
	return this->OBBRadius;
}

//THIS WILL ONLY WORK IF POSITION IS INTEGRATED CORRECTLY
glm::vec3 RigidbodyComponent::GetPositionEstimate() const
{
	return static_cast<glm::vec3>(Params[0]);
}


float RigidbodyComponent::GetOOBBMaximumLength() const
{
	return std::fmax(std::fmax(OBBRadius.y, OBBRadius.z), OBBRadius.x);
}


////////////////////////////////////
////   INPUT MANAGEMENT         ////
////////////////////////////////////
void RigidbodyComponent::handleInput(float dt)
{
	/// if (inputMgr->getKeyPress(SDL_SCANCODE_TAB))
	/// {
	/// 	isPlayer = !isPlayer;
	/// }

	if (!isPlayer) 
		return;

	Transform *T = this->m_owner->GetComponent<Transform>();
	float moveSpeed = 2000.0f * dt;
	DeferredRenderer *deferred = static_cast<DeferredRenderer*>(renderer);
	glm::vec3 fwd = deferred->GetCurrentCamera()->getLook();
	fwd.y = 0.0f;
	glm::vec3 rup = {0, 1, 0};
	glm::vec3 right = glm::cross(fwd, rup);

	
	///Camera
	//deferred->GetCurrentCamera()->setEye(glm::vec3(T->GetPosition()) + glm::vec3(0.0f, 25.0f, 50.0f));

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
}