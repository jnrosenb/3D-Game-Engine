///HEADER STUFF

#pragma once

///INCLUDES
#include <string>
#include "../External/Includes/glm/glm.hpp"
#include "Model.h"
#include "Affine.h"

class Operator;

//Obstacle avoidance
class RigidbodyComponent;


struct flockingParams 
{
	float maxAccel;
	float radius;
	float fov;

	float separationWeight;
	float cohesionWeight;
	float avoidanceWeight;
	float optimalSeparation;
};


struct particleDescriptor 
{
	float timeToLive;
	float avoidDistance;
	float mass;
	bool avoidObstacles;

	glm::vec3 spawnerPosition;
	glm::vec3 size;
	glm::vec3 position;
	glm::vec3 color;
	glm::vec3 axis;
};


class Particle
{
public:
	Particle();
	virtual ~Particle();


	virtual void Initialize(particleDescriptor const& descriptor,
		std::vector<Operator*> *m_advectors,
		std::vector<Operator*> *m_operators,
		glm::vec3 const& velocity = glm::vec3(0));
	//Needed so virtual calls it
	virtual void Initialize(particleDescriptor const& descriptor,
		flockingParams const& m_flockingParams, 
		glm::vec3 const& velocity = glm::vec3(0)) {}

	virtual void Update(float dt, std::vector<RigidbodyComponent*> const& rigidbodies);
	//Needed so virtual calls it
	virtual void Update(float dt, glm::vec3 const& target,
		std::vector<Particle*> const& particles,
		std::vector<RigidbodyComponent*> const& rigidbodies) {}

	//GETTERS
	virtual float GetMass() const;
	virtual float GetTimeToLive() const;
	virtual glm::vec3 const& GetPosition() const;
	virtual glm::vec3 const& GetVelocity() const;
	virtual glm::vec3 const& GetSize() const;
	virtual glm::vec3 const& GetColor() const;
	virtual glm::vec3 const& GetAxis() const;
	virtual glm::vec3 const& GetSpawnerPosition() const;
	virtual glm::vec3 const& GetDebugTorque() const;
	virtual glm::vec3 const& GetRotation() const;
	virtual glm::mat3 const& GetRotationMatrix();
	virtual glm::vec3 const& Particle::GetUpVector();

	//SETTERS
	virtual void SetPosition(glm::vec3 const& pos);
	virtual void SetVelocity(glm::vec3 const& vel);
	virtual void AddVelocity(glm::vec3 const& vel);

	//INTERFACE
	virtual void rotate(float dx, float dy, float dz); 
	
	//Add forces/accels
	void AddAcceleration(glm::vec3 a);
	void Damp(glm::vec3& val, float damping = 0.03f);


protected:
	//Base init method, called by every public initializer
	virtual void BaseInit(particleDescriptor const& descriptor,
		glm::vec3 const& velocity = glm::vec3(0));

	//Collision detection/avoidance
	virtual void ManageObstacleAvoidance(std::vector<RigidbodyComponent*> const& rigidbodies, 
		glm::vec3& accel, float finalScaling);
	
	//Different colliders
	virtual int SphereColliderAvoidance(glm::vec3 const& Va, glm::vec3 const& Pa, glm::vec3 const& Ctr,
		glm::vec3 const& planeNormal, float radius, float DMax, glm::vec3& outDir);
	virtual int AABBColliderAvoidance(glm::vec3 const& Va, glm::vec3 const& Pa, glm::vec3 const& Ctr,
		glm::vec3 const& planeNormal, glm::vec3 radiuses, float DMax, glm::vec3& outDir);


protected:
	float timeToLive;
	float mass;
	float fov;
	float avoidDist;
	bool avoidObstacles;
	bool needToRecalc;

	glm::vec3 spawnerPosition;
	
	glm::vec3 size;
	glm::vec3 position;
	glm::vec3 rotation;
	
	//Experiment
	glm::vec3 L;
	glm::vec3 accel;
	float maxAccel;

	//Just as visual cue for now
	glm::vec3 torqueDebug;

	glm::vec3 color;
	glm::vec3 velocity;
	glm::vec3 prevVel;
	glm::vec3 axis;

	glm::mat3 R;

private:
	//Operators vec
	std::vector<Operator*> *m_operatorsPTR;
	std::vector<Operator*> *m_advectorsPTR;
};



class InteractingParticle : public Particle 
{

public:
	InteractingParticle();
	virtual ~InteractingParticle();

	virtual void Initialize(particleDescriptor const& descriptor, 
		flockingParams const& m_flockingParams, glm::vec3 const& velocity = glm::vec3(0));
	virtual void Update(float dt, glm::vec3 const& target,
		std::vector<Particle*> const& particles,
		std::vector<RigidbodyComponent*> const& rigidbodies);


private:
	//Experiment
	glm::vec3 target;

	int maxNumberOfNeighbours;
	float radius;
	float colliderRadius;
	float separationWeight;
	float cohesionWeight;
	float avoidanceWeight;
	float optimalSeparation;
};




///////////////////////////////
////	OPERATIONS		   ////
///////////////////////////////

class Operator 
{
public:
	virtual ~Operator() {}
	virtual void operator()(float dt, Particle& p) {}
	virtual void operator()(float dt, Particle& p,
		std::vector<Particle*> const& particles) {}
};


class VortexOperation : public Operator
{
private:
	float tightness, rotationRate;

public:
	friend class Factory;

	VortexOperation(float tight = 1.5f, float rotRate = 30.0f) : Operator(),
		tightness(tight), rotationRate(rotRate) 
	{}
	
	virtual void operator()(float dt, Particle& p) override
	{
		glm::vec4 particlePos = glm::vec4(p.GetPosition(), 1);

		//VORTEX - As rotation equation (WRONG, ORBITS INSTEAD)
		glm::vec3 ParticleToCenter = glm::vec3(particlePos) - p.GetSpawnerPosition();
		float denom = std::powf(glm::dot(ParticleToCenter, ParticleToCenter) - std::powf(glm::dot(p.GetAxis(), ParticleToCenter), 2), tightness / 2.0f);
		///float degree = rotationRate * dt / denom;
		///glm::mat4 R = AuxMath::rotate(degree, p.GetAxis());
		///particlePos = glm::vec4(p.GetSpawnerPosition(), 1) + R* glm::vec4(ParticleToCenter, 0);
		///p.SetPosition(glm::vec3(particlePos.x, particlePos.y, particlePos.z));

		//VORTEX - As velocity field
		glm::vec3 numerator = rotationRate * glm::cross(p.GetAxis(), ParticleToCenter);
		glm::vec3 v1 = numerator / denom;
		
		//Finally, set velocity
		p.AddVelocity(v1);
	}
};


class SpiralOperation : public Operator
{
private:
	float rotationRate;

public:
	friend class Factory;

	SpiralOperation(float rotRate = 30.0f) : Operator(),
		rotationRate(rotRate)
	{}

	virtual void operator()(float dt, Particle& p) override
	{
		//SPIRAL - As rotation equation
		///float degree = rotationRate * dt;
		///glm::mat4 R = AuxMath::rotate(degree, p.GetAxis());
		///glm::vec4 vel = R* glm::vec4(p.GetVelocity(), 0);

		//SPIRAL - As magnetic field
		glm::vec3 a = rotationRate * glm::cross(p.GetVelocity(), p.GetAxis()) + (p.GetAxis() * 500.0f);
		glm::vec3 vel = p.GetVelocity() + (a*dt);
		
		//Finally, set velocity
		p.SetVelocity(vel);
	}
};


class GravitationalOperator : public Operator
{
private:
	glm::vec3 CenterOfGravitation;
	float mass;

public:
	friend class Factory;

	GravitationalOperator(float mass = 100.0f) : Operator(),
		mass(mass)
	{}

	virtual void operator()(float dt, Particle& p) override
	{
		CenterOfGravitation = glm::vec3(p.GetSpawnerPosition());
		float particleMass = p.GetMass();
		glm::vec3 GravitationCenterToPosition = p.GetPosition() - CenterOfGravitation;
		
		//TEMPORAL
		if (fabsf(GravitationCenterToPosition.x) < 0.0001f &&
			fabsf(GravitationCenterToPosition.y) < 0.0001f &&
			fabsf(GravitationCenterToPosition.z) < 0.0001f)
			return;

		//Calculate gravitational force between objects
		float G = 9.8f;
		glm::vec3 dir = glm::normalize(GravitationCenterToPosition);
		float term = (particleMass * mass) / glm::length(GravitationCenterToPosition);
		glm::vec3 F = dir * std::powf(term, 3) * -G;
		glm::vec3 vel = p.GetVelocity() + dt * (F/p.GetMass());

		//Finally, set velocity
		p.SetVelocity(vel);
	}
};


