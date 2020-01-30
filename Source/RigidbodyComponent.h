///HEADER STUFF

#pragma once

///INCLUDES
#include "BaseComponent.h"
#include "../External/Includes/glm/glm.hpp"
#include "Shapes.h"
#include <vector>

class Shader;
class Mesh;


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

	//These should be private
	virtual void DeserializeInit() override;
	virtual void Begin() override;

	//GETTERS
	float GetMass() const { return this->mass; }

	//OBB getter, for now
	glm::vec3 GetOBBRadiusVector() const;

private:
	void ColliderSetup();
	void DebugDrawSetup(AABB const& aabb);
	void handleInput(float dt);
	void ResetForces();

	//TEMPORARY while no friction
	void DampVelocity(glm::vec4& vel, float damping = 0.03f);

public:
	//To compare when using templates
	static COMPONENT_TYPES const comp_class_type = RIGIDBODY;

private:
	//Collider related info
	glm::mat4 IbodyInv;
	glm::vec3 OBBRadius;
	//Also collider info, but more on debug draw side of things
	AABB aabb;
	Mesh *debugMesh;
	Mesh *debugPointMesh;
	Mesh *debugRayMesh;
	Shader *debugShader;
	Shader *debugShaderLine;
	glm::vec4 DebugForce;

	//Integration parameters
	glm::vec4 Force;
	glm::vec4 Torque;

	//Vector of diferentiated params
	std::vector<glm::vec4> Params;  // x(t) - q       - P - L
	std::vector<glm::vec4> dParams; // v(t) - 0.5*w*q - F - T

	//TEMPORARY (ofr allowing controlling)
	bool isPlayer;

	//Rigidbody usual stuff
	float mass, invMass;
};