///HEADER STUFF

#pragma once

///INCLUDES
#include <string>
#include "../External/Includes/glm/glm.hpp"
#include "Model.h"

class Operator;


struct particleDescriptor 
{
	float timeToLive;
	
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
	~Particle();

	void Initialize(particleDescriptor const& descriptor,
		std::vector<Operator*> *m_advectors,
		std::vector<Operator*> *m_operators,
		glm::vec3 const& velocity = glm::vec3(0));
	void Update(float dt);

	//GETTERS
	float GetMass() const;
	float GetTimeToLive() const;
	glm::vec3 const& GetPosition() const;
	glm::vec3 const& GetVelocity() const;
	glm::vec3 const& GetSize() const;
	glm::vec3 const& GetColor() const;
	glm::vec3 const& GetAxis() const;
	glm::vec3 const& GetSpawnerPosition() const;

	//SETTERS
	void SetPosition(glm::vec3 const& pos);
	void SetVelocity(glm::vec3 const& vel);
	void AddVelocity(glm::vec3 const& vel);

private:
	float timeToLive;
	float mass;

	glm::vec3 spawnerPosition;
	glm::vec3 size;
	glm::vec3 position;
	glm::vec3 color;
	glm::vec3 velocity;
	glm::vec3 newVel;
	glm::vec3 axis;

	//Operators vec
	std::vector<Operator*> *m_operatorsPTR;
	std::vector<Operator*> *m_advectorsPTR;
};


///////////////////////////////
////	OPERATIONS		   ////
///////////////////////////////

class Operator 
{
public:
	virtual ~Operator() {}
	virtual void operator()(float dt, Particle& p) = 0;
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
		glm::vec4 particlePos = glm::vec4(p.GetPosition(), 1);

		//SPIRAL - As rotation equation
		///float degree = rotationRate * dt;
		///glm::mat4 R = AuxMath::rotate(degree, p.GetAxis());
		///glm::vec4 vel = R* glm::vec4(p.GetVelocity(), 1);

		//SPIRAL - As magnetic field
		glm::vec3 a = rotationRate * glm::cross(p.GetAxis(), p.GetVelocity());
		glm::vec3 vel = p.GetVelocity() + dt * a;
		
		//Finally, set velocity
		p.AddVelocity(vel);
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

		//Calculate gravitational force between objects
		float G = 9.8f;
		glm::vec3 dir = glm::normalize(p.GetPosition() - CenterOfGravitation);
		float term = (particleMass * mass) / glm::length(p.GetPosition() - CenterOfGravitation);
		glm::vec3 F = dir * std::powf(term, 3) * -G;
		glm::vec3 vel = p.GetVelocity() + dt * (F/p.GetMass());

		//Finally, set velocity
		p.SetVelocity(vel);
	}
};