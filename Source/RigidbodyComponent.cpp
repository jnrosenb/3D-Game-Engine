///HEADER STUFF


#include "RigidbodyComponent.h"
#include "GameObject.h"
#include <limits>
#include <assert.h>

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
	//Start with zero forces
	ResetForces();

	//Rigidbody parameters
	invMass = (mass == 0) ? 0.0f : 1.0f / mass;
	L = glm::vec4(0);
}


void RigidbodyComponent::Begin()
{
	//Setup initial params (HERE BECAUSE IT DEPENDS ON OTHER COMPONENT)
	Transform *T = this->m_owner->GetComponent<Transform>();

	Params[0] = T->GetPosition();
	Params[1] = glm::vec4(0);
	Params[2] = glm::vec4(0);
	Params[3] = glm::vec4(0);

	dParams[0] = glm::vec4(0);
	dParams[1] = glm::vec4(0);
	dParams[2] = glm::vec4(0);
	dParams[3] = glm::vec4(0);

	//Starts at zero
	prevVel = glm::vec4(0);

	//Setup OBB stuff
	ColliderSetup();

	//Register this rgbdy on physics manager
	physicsMgr->RegisterRigidbody(this);
}


//Offset is already ContactWorldPos - COMWorldPos
void RigidbodyComponent::ApplyForce(glm::vec3 const& F, 
	glm::vec3 const& offset)
{
	assert(offset.x == offset.x && offset.y == offset.y && offset.z == offset.z);
	assert(F.x == F.x && F.y == F.y && F.z == F.z);

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
		float halfwidth = aabb.radius.x;  ///(aabb.max.x - aabb.min.x) * 0.5f;
		float halfheight = aabb.radius.y; ///(aabb.max.y - aabb.min.y) * 0.5f;
		float halfdepth = aabb.radius.z;  ///(aabb.max.z - aabb.min.z) * 0.5f;
		glm::mat4 I(1);
		I[0][0] = (1 / 12.0f) * mass * (halfheight*halfheight + halfdepth*halfdepth);
		I[1][1] = (1 / 12.0f) * mass * (halfwidth*halfwidth + halfdepth*halfdepth);
		I[2][2] = (1 / 12.0f) * mass * (halfheight*halfheight + halfwidth*halfwidth);
		IbodyInv = glm::inverse(I);


		//FOR NOW, FIXED SCALE!!!!------------------------------------------------------
																						
		//Construct OBB dimension vector (radius)										
		OBBRadius = glm::vec3(															
			halfwidth * transformComp->GetScale().x,									
			halfheight * transformComp->GetScale().y,									
			halfdepth * transformComp->GetScale().z										
		);																				
																						
		//For now I'm only gonna leave it as the offset from the origin in object space,
		//since I believe the addition of the world position is done later				
		OBBCenterOffsetScaled = glm::vec4(															
			aabb.center.x * transformComp->GetScale().x, 								
			aabb.center.y * transformComp->GetScale().y,								
			aabb.center.z * transformComp->GetScale().z,								
			0.0f																		
		);																				
		//------------------------------------------------------------------------------
	}
}


glm::vec3 RigidbodyComponent::GetAABBRadiusFromOBB()
{
	Transform *transformComp = this->m_owner->GetComponent<Transform>();
	Render *rendererComp = this->m_owner->GetComponent<Render>();
	if (rendererComp && transformComp)
	{
		glm::mat4 const& R = transformComp->GetRotationMatrix();
		float xMax = std::numeric_limits<float>::lowest(), yMax = xMax, zMax = xMax;
		float xMin = std::numeric_limits<float>::max(), yMin = xMin, zMin = xMin;

		glm::vec3 xLocalDir = R[0] * OBBRadius.x;
		glm::vec3 yLocalDir = R[1] * OBBRadius.y;
		glm::vec3 zLocalDir = R[2] * OBBRadius.z;

		for (int i = -1; i <= 1; i += 2)
		{
			for (int j = -1; j <= 1; j += 2)
			{
				for (int k = -1; k <= 1; k += 2)
				{
					glm::vec3 vertex = glm::vec3(
						xLocalDir.x*i + yLocalDir.x*j + zLocalDir.x*k,
						xLocalDir.y*i + yLocalDir.y*j + zLocalDir.y*k,
						xLocalDir.z*i + yLocalDir.z*j + zLocalDir.z*k);
					xMax = (vertex.x > xMax) ? vertex.x : xMax;
					xMin = (vertex.x < xMin) ? vertex.x : xMin;
					yMax = (vertex.y > yMax) ? vertex.y : yMax;
					yMin = (vertex.y < yMin) ? vertex.y : yMin;
					zMax = (vertex.z > zMax) ? vertex.z : zMax;
					zMin = (vertex.z < zMin) ? vertex.z : zMin;
				}
			}
		}

		glm::vec3 result(0.5f * fabsf(xMax - xMin), 0.5f * fabsf(yMax - yMin), 0.5f * fabsf(zMax - zMin));
		return result;
	}
	else 
	{
		assert(false);
		return glm::vec3(-1);
	}
}


void RigidbodyComponent::DebugDrawSetup(AABB const& aabb)
{
	//USING SEMI TRANSPARENT BLOCK
	glm::vec3 size = 2.0f * aabb.radius;//{aabb.max - aabb.min};
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
	//Gravity fake
	if (affectedByGravity)
		this->ApplyForce({ 0, -(1.0f * mass), 0 }, glm::vec3(0));

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
		position = position + linearVel*dt;

		T->translate((0.5f*dt)*(linearVel+prevVel));
		prevVel = linearVel;
	}

	/////////////////////////////////
	////    ANGULAR STUFF        ////
	/////////////////////////////////
	if (Torque != glm::vec4(0) || L != glm::vec4(0))
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
		assert(Torque.x == Torque.x && Torque.y == Torque.y && Torque.z == Torque.z);
		L = L + Torque * dt;
		Params[3] = Iinv * L;

		/// assert(Params[0].x == Params[0].x && Params[0].y == Params[0].y && Params[0].z == Params[0].z);
		/// assert(Params[1].x == Params[1].x && Params[1].y == Params[1].y && Params[1].z == Params[1].z);
		/// assert(Params[2].x == Params[2].x && Params[2].y == Params[2].y && Params[2].z == Params[2].z);
		/// assert(Params[3].x == Params[3].x && Params[3].y == Params[3].y && Params[3].z == Params[3].z);

		AuxMath::Quaternion w(Params[3]);
		DampVelocity(L, 0.1f);

		///Get quaternion velocity (slope vector), then integrate
		AuxMath::Quaternion qvel = 0.5f * (q0 * w);
		AuxMath::Quaternion q = q0 + (dt * qvel);
		q = q0.Conjugate() * q;
		q = q.Normalize();

		//T->rotateWorld(q);
		T->rotate(q);
		//T->rotate(dt * qvel);
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

	////////////////////////////////////////////////
	return;
	////////////////////////////////////////////////

	//ARROWS
	//Local axis drawing test (FORWARD)
	DrawMeshWithOrientation(debugRay->meshes[0], debugShader, T->GetRotationMatrix()[2],
		T->GetPosition(), 3.0f, { 0, 0, 1, 1 });

	//Local axis drawing test (UP)
	DrawMeshWithOrientation(debugRay->meshes[0], debugShader, T->GetRotationMatrix()[1],
		T->GetPosition(), 3.0f, { 0, 1, 0, 1 });

	//Local axis drawing test (RIGHT)
	DrawMeshWithOrientation(debugRay->meshes[0], debugShader, T->GetRotationMatrix()[0],
		T->GetPosition(), 3.0f, { 1, 0, 0, 1 });

	//FORCE
	DrawMeshWithOrientation(debugRay->meshes[0], debugShader, this->dParams[0],
		T->GetPosition(), 3.0f, { 0, 1, 1, 1 });

	////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////

	//CONTACT POINT TEST (all corners)
	/// glm::mat4 const& R123 = T->GetRotationMatrix();
	/// glm::mat4 dotModel(0.2f);
	/// glm::vec3 xLocalDir = R123[0] * OBBRadius.x;
	/// glm::vec3 yLocalDir = R123[1] * OBBRadius.y;
	/// glm::vec3 zLocalDir = R123[2] * OBBRadius.z;
	/// for (int i = -1; i <= 1; i += 2)
	/// {
	/// 	for (int j = -1; j <= 1; j += 2)
	/// 	{
	/// 		for (int k = -1; k <= 1; k += 2)
	/// 		{
	/// 			glm::vec3 vertex = glm::vec3(xLocalDir.x*i + yLocalDir.x*j + zLocalDir.x*k, 
	/// 				xLocalDir.y*i + yLocalDir.y*j + zLocalDir.y*k, 
	/// 				xLocalDir.z*i + yLocalDir.z*j + zLocalDir.z*k);
	/// 			DrawDebugData data2 = {};
	/// 			data2.diffuseColor = { 0, 1, 0, 1.0f };
	/// 			
	/// 			glm::mat4 const& R44 = T->GetRotationMatrix();
	/// 			dotModel[3] = (R44*OBBCenterOffsetScaled) + T->GetPosition() + glm::vec4(vertex.x, vertex.y, vertex.z, 0.0f);
	/// 			
	/// 			data2.model = dotModel;
	/// 			data2.mesh = debugPointMesh;
	/// 			data2.shader = debugShader;
	/// 			renderer->QueueForDebugDraw(data2);
	/// 		}
	/// 	}
	/// }

	////////////////////////////////////////////////
	////////////////////////////////////////////////
	
	//OBB
	DrawDebugData data = {};
	data.diffuseColor = { 0, 0, 1, 0.5f };

	data.model = T->GetModel(); 
	glm::mat4 const& R44 = T->GetRotationMatrix();
	data.model[3] = (R44*OBBCenterOffsetScaled) + T->GetPosition();
	
	data.mesh = debugMesh;
	data.shader = debugShader;
	//Bones experiment SHITTY WAY - NOT USING BONES FOR THIS
	data.BoneTransformations = (animComp == 0) ? 0 : &(animComp->BoneTransformations);
	renderer->QueueForDebugDraw(data);


	////////////////////////////////////////////////
	////////////////////////////////////////////////

	//AABB
	/// glm::vec3 sizeAABB = GetAABBRadiusFromOBB();
	/// data.diffuseColor = { 0, 1, 1, 0.25f };
	/// glm::mat4 tempMod(1);
	/// tempMod[0][0] = sizeAABB.x;
	/// tempMod[1][1] = sizeAABB.y;
	/// tempMod[2][2] = sizeAABB.z;
	/// tempMod[3] = T->GetPosition(); //Here we should be adding the displacement in order for it to work I think
	/// data.model = tempMod;
	/// data.mesh = debugMesh;
	/// data.shader = debugShader;
	/// data.BoneTransformations = 0;
	/// renderer->QueueForDebugDraw(data);
}


glm::vec3 RigidbodyComponent::GetOBBRadiusVector() const 
{
	return this->OBBRadius;
}

glm::vec4 RigidbodyComponent::GetOBBCenterOffsetScaled() const
{
	//TODO - FIND CLEANER ALTERNATIVE
	Transform *T = this->m_owner->GetComponent<Transform>();
	glm::mat4 R = T->GetRotationMatrix();
	glm::vec4 OrientedOffset = R * OBBCenterOffsetScaled;
	//glm::vec4 OrientedOffset = (R[0] * OBBCenterOffsetScaled.x) + (R[1] * OBBCenterOffsetScaled.y) + (R[2] * OBBCenterOffsetScaled.z);
	return OrientedOffset;
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
	float moveSpeed = 1400.0f * dt;
	DeferredRenderer *deferred = static_cast<DeferredRenderer*>(renderer);
	glm::vec3 fwd = deferred->GetCurrentCamera()->getLook();
	fwd.y = 0.0f;
	glm::vec3 rup = {0, 1, 0};
	glm::vec3 right = glm::cross(fwd, rup);

	
	///Camera follow player
	//deferred->GetCurrentCamera()->setEye(glm::vec3(T->GetPosition()) + glm::vec3(0.0f, 50.0f, 85.0f));


	if (inputMgr->getKeyPress(SDL_SCANCODE_RIGHT))
	{
		ApplyForce(moveSpeed * right, { 0 ,0.0f, 0 });
	}
	if (inputMgr->getKeyPress(SDL_SCANCODE_LEFT))
	{
		ApplyForce( -moveSpeed * right, { 0, 0.0f, 0 });
	}
	if (inputMgr->getKeyPress(SDL_SCANCODE_UP))
	{
		ApplyForce( moveSpeed * fwd, { 0, 0.0f, 0 });
	}
	if (inputMgr->getKeyPress(SDL_SCANCODE_DOWN))
	{
		ApplyForce( -moveSpeed * fwd, { 0, 0.0f, 0 });
	}
	if (inputMgr->getKeyPress(SDL_SCANCODE_PAGEUP))
	{
		ApplyForce(moveSpeed * rup, { 0, 0.0f, 0 });
	}
	if (inputMgr->getKeyPress(SDL_SCANCODE_PAGEDOWN))
	{
		ApplyForce(-moveSpeed * rup, { 0, 0.0f, 0 });
	}


	//TOGGLE TREE
	if (inputMgr->getKeyTrigger(SDL_SCANCODE_SPACE))
	{
		ApplyForce(rup * (moveSpeed * 10.0f), { 0, 0.0f, 0 });
 		//physicsMgr->publicToggleVBH();
	}
}


//////////////////////////////////////////////////////////
//  DEBUG DRAW OF MESHES  //  TEMPORAL - MOVE TO DEBUG  //
//////////////////////////////////////////////////////////

void RigidbodyComponent::DrawMeshWithOrientation(Mesh *mesh, Shader *shader,
	glm::vec4 const& dir, glm::vec4 const& worldPos, 
	float scale, glm::vec4 const& color) 
{
	//Model matrix we'll give to the object
	glm::mat4 rayModel(1);
	
	//Dealing with the direction vector used (forward)
	///float f = glm::length(dir);
	///glm::vec4 forceDir = dir / f;
	glm::vec4 forceDir = glm::normalize(dir);
	
	//If dir is parallel to up, we need to use another to get right
	glm::vec3 fwd = glm::vec3(forceDir);
	glm::vec3 right = glm::cross(fwd, { 0,1,0 });
	if (right == glm::vec3(0))
		right = glm::cross(fwd, { 0,0,1 });
	glm::vec3 up = glm::cross(right, fwd);
	
	//Rotation - Scale
	glm::mat3 R = { fwd, up, right }; 
	R = R * glm::mat3(scale);
	rayModel[0] = glm::vec4(R[0].x, R[0].y, R[0].z, 0.0f);
	rayModel[1] = glm::vec4(R[1].x, R[1].y, R[1].z, 0.0f);
	rayModel[2] = glm::vec4(R[2].x, R[2].y, R[2].z, 0.0f);
	
	//Translation
	rayModel[3] = worldPos;
	
	//Final diagonal
	rayModel[3][3] = 1.0f;
	
	//Pass to graphic's manager
	DrawDebugData data = {};
	data.diffuseColor = color;
	data.model = rayModel;
	data.mesh = mesh;
	data.shader = shader;
	renderer->QueueForDebugDraw(data);
}