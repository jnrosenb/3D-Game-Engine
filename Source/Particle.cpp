///HEADER STUFF

#include "Particle.h"
#include "Affine.h"
#include <vector>



Particle::Particle() : timeToLive(-1.0f)
{
	//TODO
}

Particle::~Particle()
{
}

void Particle::Initialize(particleDescriptor const& descriptor,
	std::vector<Operator*> *m_advectors,
	std::vector<Operator*> *m_operators,
	glm::vec3 const& velocity)
{
	//Set the particle multiple parameters
	this->spawnerPosition = descriptor.spawnerPosition;
	this->timeToLive = descriptor.timeToLive;
	this->mass = 1.0f;//descriptor.mass;
	this->position = descriptor.position;
	this->axis = descriptor.axis;

	//Set initial velocity if any
	this->velocity = velocity;

	//Operators
	this->m_advectorsPTR = m_advectors;
	this->m_operatorsPTR = m_operators;
}

void Particle::Update(float dt) 
{
	//Apply advections (base, overwrite velocities)
	int advectionCount = m_advectorsPTR->size();
	if (advectionCount)
	{
		SetVelocity(glm::vec3(0));
		this->velocity = glm::vec3(0);
		for (Operator *op : *m_advectorsPTR)
			op->operator()(dt, *this);
	}

	//Operators to be added using operator splittting
	for (Operator *op : *m_operatorsPTR)
		op->operator()(dt, *this);

	//INTEGRATE AFTER ALL THE OPERATOR SPLITTING
	this->position = position + 0.5f * dt*(velocity + newVel);
	this->velocity = newVel;

	//UPDATE TIME TO LIVE
	this->timeToLive -= dt;
}

float Particle::GetMass() const
{
	return this->mass;
}

float Particle::GetTimeToLive() const
{
	return timeToLive;
}

glm::vec3 const& Particle::GetPosition() const
{
	return this->position;
}

glm::vec3 const& Particle::GetVelocity() const
{
	return this->velocity;
}

glm::vec3 const& Particle::GetSize() const
{
	return this->size;
}

glm::vec3 const& Particle::GetColor() const
{
	return this->color;
}

glm::vec3 const& Particle::GetAxis() const
{
	return this->axis;
}

glm::vec3 const& Particle::GetSpawnerPosition() const
{
	return this->spawnerPosition;
}


///////////////////////////////////
////       SETTERS             ////
///////////////////////////////////
void Particle::SetPosition(glm::vec3 const& pos)
{
	this->position = pos;
}

void Particle::SetVelocity(glm::vec3 const& vel)
{
	this->newVel = vel;
}

void Particle::AddVelocity(glm::vec3 const& vel)
{
	this->newVel += vel;
}