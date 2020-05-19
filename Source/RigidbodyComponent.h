///HEADER STUFF

#pragma once

///INCLUDES
#include "BaseComponent.h"
#include "../External/Includes/glm/glm.hpp"
#include "Shapes.h"
#include <vector>

class Shader;
class Mesh;
class Model;


//Rigidbody component
class RigidbodyComponent : public BaseComponent
{
public:
	friend class Factory;

//PUBLIC INTERFACE
public:
	RigidbodyComponent(GameObject *owner);
	virtual ~RigidbodyComponent();

	virtual RigidbodyComponent* clone() override
	{
		return new RigidbodyComponent(m_owner);
	}
	virtual void Update(float dt) override;
	virtual void PhysicsUpdate(float dt) override;
	virtual void Draw() override;
	
	//For now, contact will be on center of mass
	void ApplyForce(glm::vec3 const& F, 
		glm::vec3 const& offset);
	void ApplyForce(glm::vec3 const& F);
	void ApplyTorque(glm::vec3 const& T);
	void ApplyImpulse(float dt, glm::vec4 const& Linear, 
		glm::vec4 const& Angular);

	//These should be private
	virtual void DeserializeInit() override;
	virtual void Begin() override;
	
	//Getting data regarding interaction with particles
	bool HasParticleInteraction() const { return hasParticleInteraction; }
	std::string const& GetParticleCollider() const { return ParticleCollider; }

	//GETTERS
	float GetMass() const { return this->mass; }
	bool IsStatic() const { return mass == 0.0f; }

	//OBB getter, for now
	glm::vec3 GetOBBRadiusVector() const;
	glm::vec4 GetOBBCenterOffsetScaled() const;
	float GetOOBBMaximumLength() const;
	glm::vec3 GetPositionEstimate() const;
	glm::vec3 GetAABBRadiusFromOBB();


	//For a contact pair, gives you the world position based on the body position (of the collider)
	glm::vec4 BodyToWorldContact(glm::vec4 const& bodyCoord);


	//For constraint solver
	glm::vec4 const& GetVelocity() const;
	glm::vec4 const& GetAngularVelocity() const;
	float getMassInv() const;
	glm::mat4 GetInertiaTensorWorldInv() const;

	//Experiment with setters
	void SetLinearVelocity(glm::vec4 const& v) 
	{
		this->V = v;
	}
	void SetAngularVelocity(glm::vec4 const& w)
	{
		this->W = w;
	}
	//Experiment with setters

	//FOR NOW PUBLIC
	glm::vec4 Force;
	glm::vec4 Torque;
	void ResetForces();

private:
	void ColliderSetup();
	void DebugDrawSetup(AABB const& aabb);
	void handleInput(float dt);

	//TEMPORARY while no friction
	void DampVelocity(glm::vec4& vel, float damping = 0.03f);

	//TEMPORARY - Move to a debug class later
	void DrawMeshWithOrientation(Mesh *mesh, Shader *shader,
		glm::vec4 const& dir, glm::vec4 const& worldPos,
		float scale, glm::vec4 const& color);

public:
	//To compare when using templates
	static COMPONENT_TYPES const comp_class_type = RIGIDBODY;

private:
	//Collider related info
	glm::mat4 IbodyInv;
	glm::vec3 OBBRadius;
	glm::vec4 OBBCenterOffsetScaled;

	//Particle stuff
	bool hasParticleInteraction;
	std::string ParticleCollider;

	//Also collider info, but more on debug draw side of things
	AABB aabb;
	Mesh *debugMesh;
	Mesh *debugPointMesh;
	Mesh *debugRayMesh;//////////////**
	Model *debugRay;/////////////////**
	Shader *debugShader;
	Shader *debugShaderLine;
	glm::vec4 DebugForce;

	//Integration parameters
	glm::vec4 prevAngAccel;
	glm::vec4 prevVel;
	glm::vec4 L;
	glm::vec4 W;
	glm::vec4 P;
	glm::vec4 V;

	//Vector of diferentiated params
	std::vector<glm::vec4> Params;  // x(t) - q       - P - L
	std::vector<glm::vec4> dParams; // v(t) - 0.5*w*q - F - T

	//TEMPORARY (ofr allowing controlling)
	bool isPlayer;

	//Gravity
	bool affectedByGravity;

	//Rigidbody usual stuff
	float mass, invMass;
};