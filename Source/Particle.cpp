///HEADER STUFF

#include "Particle.h"
#include "Affine.h"
#include <vector>


//OBSTACLE AVOIDANCE
#include "RigidbodyComponent.h"


#define PI 3.141592f


Particle::Particle() : timeToLive(-1.0f), 
	rotation(glm::vec3(0)), L(glm::vec3(0)), 
	accel(glm::vec3(0)), mass(1.0f), fov(60.0f), 
	size(glm::vec3(1, 1, 1))
{
	//FOR NOW
	needToRecalc = true;
}

Particle::~Particle()
{
}


void Particle::BaseInit(particleDescriptor const& descriptor,
	glm::vec3 const& velocity)
{
	//Set the particle multiple parameters
	this->spawnerPosition = descriptor.spawnerPosition;
	this->timeToLive = descriptor.timeToLive;
	this->mass = descriptor.mass;
	this->SetPosition(descriptor.position);
	this->axis = descriptor.axis;
	this->size = descriptor.size;
	this->avoidDist = descriptor.avoidDistance;
	this->avoidObstacles = descriptor.avoidObstacles;

	//Set initial velocity if any
	this->velocity = velocity;
	this->prevVel = velocity;
}


void Particle::Initialize(particleDescriptor const& descriptor,
	std::vector<Operator*> *m_advectors,
	std::vector<Operator*> *m_operators,
	glm::vec3 const& velocity)
{
	//Common initializer
	BaseInit(descriptor, velocity);

	//Operators
	this->m_advectorsPTR = m_advectors;
	this->m_operatorsPTR = m_operators;
}

void Particle::Update(float dt, std::vector<RigidbodyComponent*> const& rigidbodies)
{
	//Apply advections (base, overwrite velocities)
	int advectionCount = m_advectorsPTR->size();
	if (advectionCount)
	{
		SetVelocity(glm::vec3(0));
		for (Operator *op : *m_advectorsPTR)
			op->operator()(dt, *this);
	}


	//Operators to be added using operator splittting
	for (Operator *op : *m_operatorsPTR)
		op->operator()(dt, *this);


	//OBSTACLE AVOIDANCE
	glm::vec3 a0(0);
	if(avoidObstacles)
		ManageObstacleAvoidance(rigidbodies, a0, 30.0f);
	
	//Add accelerations, then use that to final velocity integration
	AddAcceleration(a0);
	this->velocity = velocity + accel*dt;


	//////////////////////////////////////
	////    ROTATION INTEGRATION      ////
	//////////////////////////////////////
	glm::vec3 forward = GetRotationMatrix()[2];
	float invMass = (mass == 0.0f) ? 0.0f : 1.0f / mass;
	
	float dot = glm::dot(forward, GetVelocity());
	glm::vec3 torque = glm::cross(forward, GetVelocity());
	/// if (dot < 0)
	/// 	torque = -torque;
	
	float torqueEpsilon = 0.0001f;// 0.00001f; -> this one works well enough
	if (torque.x*torque.x + torque.y*torque.y + torque.z*torque.z <= torqueEpsilon &&
		glm::dot(forward, GetVelocity()) < 0.0f)
		torque = GetUpVector();
	torqueDebug = torque;
	torque = torque * 30.0f;
	L = /*L +*/ torque * dt;
	/*Damp(L, 0.01f);*/
	AuxMath::Quaternion q0(GetRotationMatrix());
	AuxMath::Quaternion w(L*invMass); //Assuming Inertia tensor = identity scaled by mass
	AuxMath::Quaternion dqdt = 0.5f * q0 * w;
	AuxMath::Quaternion q = q0 + dqdt * dt;
	AuxMath::Quaternion dq = q * q0.Conjugate();
	dq = dq.Normalize();
	glm::vec3 deltaRot;
	AuxMath::Quaternion::QuaternionToEuler(dq, deltaRot);
	deltaRot = deltaRot * (180.0f / PI);
	this->rotate(deltaRot.x, deltaRot.y, deltaRot.z);


	//////////////////////////////////////
	////    TRANSLATION INTEGRATION   ////
	//////////////////////////////////////
	this->position = position + 0.5f * dt*(prevVel + velocity);

	//UPDATE TIME TO LIVE
	this->accel = glm::vec3(0);
	this->prevVel = velocity;
	this->timeToLive -= dt;
}

glm::vec3 const& Particle::GetRotation() const
{
	return rotation;
}


void Particle::ManageObstacleAvoidance(std::vector<RigidbodyComponent*> const& rigidbodies,
	glm::vec3& accel, float finalScaling)
{
	//Loop through all rigidbodies, add to the avoid acceleration
	for (RigidbodyComponent *body : rigidbodies)
	{
		//First check if this body is a valid obstacle
		if (body->HasParticleInteraction() == false)
			continue;

		//Calculate accel depending on the collider
		glm::vec3 anew(0);
		int result = 0;
		if (body->GetParticleCollider() == "sphere")
		{
			result = SphereColliderAvoidance(GetVelocity(), GetPosition(),
				body->GetPositionEstimate(), GetUpVector(),
				body->GetOOBBMaximumLength(), avoidDist, anew);
		}
		else if (body->GetParticleCollider() == "aabb")
		{
			result = AABBColliderAvoidance(GetVelocity(), GetPosition(),
				body->GetPositionEstimate(), GetUpVector(),
				body->GetOBBRadiusVector(), avoidDist, anew);
		}

		if (result != 1) continue;
		accel = accel + anew;
	}

	//Finally, apply the scaling and/or average it
	accel = accel * (finalScaling);
}


void Particle::AddAcceleration(glm::vec3 a)
{
	//Zero a, return
	if (a.x == 0 && a.y == 0 && a.z == 0)
		return;

	//If we are already on max accel, return
	float currentAccel = glm::length(accel);
	if (fabsf(currentAccel - maxAccel) < 0.001f)
		return;

	//If we are still below, add the new acceleration a
	if (currentAccel < maxAccel) 
	{
		accel += a;
		float newAccel = glm::length(accel);
		if (newAccel > maxAccel) 
		{
			//Go back on x magnitude
			float Ai2 = glm::dot(a, a);
			float AiAn = glm::dot(a, accel);
			float Amax2 = maxAccel * maxAccel;
			float An2 = glm::dot(accel, accel);
			float disc = Ai2 * Amax2 - An2 * Ai2 + AiAn * AiAn;
			float x = (disc > 0) ? (AiAn - sqrtf(disc)) / Ai2 : AiAn / Ai2;
			
			//Result
			accel = accel - a * x;
			assert(accel.x == accel.x && accel.y == accel.y && accel.z == accel.z);
		}
	}
	assert(accel.x == accel.x && accel.y == accel.y && accel.z == accel.z);
}

glm::vec3 const& Particle::GetDebugTorque() const
{
	return torqueDebug;
}

glm::mat3 const& Particle::GetRotationMatrix()
{
	if (needToRecalc)
	{
		R = static_cast<glm::mat3>(
			AuxMath::rotate(rotation.x, { 1,0,0 }) *
			AuxMath::rotate(rotation.y, { 0,1,0 }) *
			AuxMath::rotate(rotation.z, { 0,0,1 })
		);
		needToRecalc = false;
		return R;
	}
	return R;
}

glm::vec3 const& Particle::GetUpVector() 
{
	return GetRotationMatrix()[1];
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
	this->velocity = vel;
}

void Particle::AddVelocity(glm::vec3 const& vel)
{
	this->velocity += vel;
}

void Particle::rotate(float dx, float dy, float dz)
{
	needToRecalc = true;
	rotation.x += dx;
	rotation.y += dy;
	rotation.z += dz;
}


void Particle::Damp(glm::vec3& val, float damping)
{
	val = val * (1.0f - damping);
	val.x = (fabs(val.x) <= 0.001f) ? 0.0 : val.x;
	val.y = (fabs(val.y) <= 0.001f) ? 0.0 : val.y;
	val.z = (fabs(val.z) <= 0.001f) ? 0.0 : val.z;
}



//////////////////////////////////////////////
//////   INTERACTIVE PARTICLE           //////
//////////////////////////////////////////////

InteractingParticle::InteractingParticle()
{
	//SERIALIZE
	maxNumberOfNeighbours = 5;
}

InteractingParticle::~InteractingParticle()
{
}

void InteractingParticle::Initialize(particleDescriptor const& descriptor,
	flockingParams const& m_flockingParams, glm::vec3 const& velocity)
{
	//Set the particle multiple parameters
	BaseInit(descriptor, velocity);

	//Flocking params
	this->maxAccel = m_flockingParams.maxAccel;
	this->radius = m_flockingParams.radius;
	this->fov = m_flockingParams.fov;
	this->separationWeight = m_flockingParams.separationWeight;
	this->cohesionWeight = m_flockingParams.cohesionWeight;
	this->avoidanceWeight = m_flockingParams.avoidanceWeight;
	this->optimalSeparation = m_flockingParams.optimalSeparation;
}


void InteractingParticle::Update(float dt, glm::vec3 const& target,
	std::vector<Particle*> const& particles,
	std::vector<RigidbodyComponent*> const& rigidbodies)
{
	unsigned int NeighboursFound = 0;
	std::vector<InteractingParticle*> neighbours;
	glm::vec3 forward = GetRotationMatrix()[2];

	//FIND NEIGHBOURS
	for (Particle *p : particles) 
	{
		//Avoid self
		if (p == this)
			continue;

		//Distance check
		float distSqr = std::powf(p->GetPosition().x - GetPosition().x, 2) + 
			std::powf(p->GetPosition().y - GetPosition().y, 2) + 
			std::powf(p->GetPosition().z - GetPosition().z, 2);
		if (distSqr > radius*radius)
			continue;

		//FOV check
		glm::vec3 AtoBUnit = glm::normalize(p->GetPosition() - GetPosition());
		float halfFOVRadian = 0.5*fov*(PI / 180.0f);
		if (glm::dot(forward, AtoBUnit) < std::cosf(halfFOVRadian))
			continue;

		//If all is good, add to list of neighbours we will use
		neighbours.push_back(static_cast<InteractingParticle*>(p));
		if (++NeighboursFound == maxNumberOfNeighbours)
			break;
	}

	//NOW, LOOP NEIGHBOURS AND GET THE RULES ACCELERATIONS
	glm::vec3 a0 = glm::vec3(0), a1 = glm::vec3(0), 
		a2 = glm::vec3(0), a3 = glm::vec3(0), a4 = glm::vec3(0);
	for (InteractingParticle *p : neighbours) 
	{
		float dist = glm::distance(GetPosition(), p->GetPosition());
		if (dist <= 0.0f)
			continue;
		
		//RULE 1 & 3
		float invSqr = std::powf((optimalSeparation / dist), 2) - 1.0f;
		float separationForce = separationWeight * maxAccel;
		float cohesionForce = cohesionWeight * maxAccel;
		if (invSqr >= 0.0f) 
			a3 = a3 - separationForce * invSqr * glm::normalize(p->GetPosition() - GetPosition());
		else 
			a1 = a1 - cohesionForce * invSqr * glm::normalize(p->GetPosition() - GetPosition());

		//RULE 2
		a2 = a2 + p->GetVelocity();
	}
	
	//RULE 2, final step
	float invT = 0.5f;
	float overN = (neighbours.size()) ? 1.0f / neighbours.size() : 0.0f;
	a2 = ( (a2 * overN) - GetVelocity() )*invT;
	assert(a2.x == a2.x && a2.y == a2.y && a2.z == a2.z);

	//RULE 4
	invT = 0.5f;
	float chaseSpeed = 0.5f * maxAccel; //SERIALIZE
	a4 = invT * (chaseSpeed * glm::normalize(target - GetPosition()) - GetVelocity());

	//OBSTACLE AVOIDANCE RULE
	if (avoidObstacles)
		ManageObstacleAvoidance(rigidbodies, a0, 0.8f * maxAccel);

	//Allocate the accelerations
	AddAcceleration(a0);
	AddAcceleration(a1);
	AddAcceleration(a2);
	AddAcceleration(a3);
	AddAcceleration(a4);


	//By this point, the final acceleration on obj should be defined. 
	//Get force in order to get torque
	float invMass = (mass == 0.0f) ? 0.0f : (1.0f / mass);
	glm::vec3 force = mass * accel;

	//////////////////////////////////////
	////    ROTATION INTEGRATION      ////
	//////////////////////////////////////

	float dot = glm::dot(forward, GetVelocity());
	glm::vec3 torque = glm::cross(forward, GetVelocity());
	/// if (dot < 0)
	/// 	torque = -torque;
	
	float torqueEpsilon = 0.0001f;// 0.00001f; -> this one works well enough
	if (torque.x*torque.x + torque.y*torque.y + torque.z*torque.z <= torqueEpsilon && 
		glm::dot(forward, GetVelocity()) < 0.0f)
		torque = GetUpVector();
	torqueDebug = torque;
	torque = torque * 30.0f;
	L = /*L +*/ torque * dt;
	/*Damp(L, 0.01f);*/
	AuxMath::Quaternion q0(GetRotationMatrix());
	AuxMath::Quaternion w(L*invMass); //Assuming Inertia tensor = identity scaled by mass
	AuxMath::Quaternion dqdt = 0.5f * q0 * w;
	AuxMath::Quaternion q = q0 + dqdt *dt;
	AuxMath::Quaternion dq = q * q0.Conjugate();
	dq = dq.Normalize();
	glm::vec3 deltaRot;
	AuxMath::Quaternion::QuaternionToEuler(dq, deltaRot);
	deltaRot = deltaRot * (180.0f/PI);
	this->rotate(deltaRot.x, deltaRot.y, deltaRot.z);

	
	//////////////////////////////////////
	////    TRANSLATION INTEGRATION   ////
	//////////////////////////////////////
	this->velocity = velocity + dt * accel;
	this->position = position + 0.5f * dt*(prevVel + velocity);

	
	//////////////////////////////////////
	////    AT THE END OF UPDATE      ////
	//////////////////////////////////////
	this->accel = glm::vec3(0);
	this->prevVel = velocity;
	this->timeToLive -= dt;
}


/////////////////////////////////////////////////////////
//////		COLLISION DETECTION/AVOIDANCE          //////
/////////////////////////////////////////////////////////
int Particle::SphereColliderAvoidance(glm::vec3 const& Va, glm::vec3 const& Pa, glm::vec3 const& Ctr,
	glm::vec3 const& planeNormal, float radius, float DMax, glm::vec3& outDir)
{
	//1st - Check if our path collides against bounding sphere
	float A = glm::dot(Va, Va);
	float B = 2 * glm::dot(Va, Pa - Ctr);
	float C = glm::dot(Pa - Ctr, Pa - Ctr) - powf(radius, 2);
	float disc = B*B - 4*A*C;

	//If disc < 0.0f, early out
	if (disc < 0.0f) return -4;
	
	//Get the time of intersection
	float sqrtDisc = sqrtf(disc);
	float num = -1.0f;
	float t = 0.0f;
	float t1 = (-B - sqrtDisc) / (2 * A);
	float t2 = (-B + sqrtDisc) / (2 * A);
	if (t1 > 0.0f) 
	{
		t = t1;
		num = -B - sqrtDisc;
	}
	else if (t2 > 0.0f) 
	{
		//If inside circle, just apply any force and return. Not done yet
		t = t2;
		num = -B + sqrtDisc;
		return -1;
	}

	//dSee represents if the obstacle is "visible" given the particle's direction
	float dSee = (-num <= 0.0f) ? 1.0f : 0.0f;
	if (dSee == 0.0f) return -3;

	//dRange represents if we are within range of the obstable to care about it
	float dRange = (glm::length(Va) * t <= DMax) ? 1.0f : 0.0f;
	if (dRange == 0.0f) return -2;

	//Get important data. 'Cp' is the center of the intersected circle, 's' is its radius
	float CtrToCpLength = glm::dot((Ctr - Pa), planeNormal);
	float aux = std::powf(radius, 2) - std::powf(CtrToCpLength, 2);
	if (aux < 0.0f) return -1;
	float s = sqrtf(aux);
	glm::vec3 CtrToCp = planeNormal * (-CtrToCpLength);
	glm::vec3 Cp = Ctr + CtrToCp;

	//To get u
	float CpToPaLength = glm::length(Pa - Cp);
	glm::vec3 CpToPaUnit = (Pa - Cp) / CpToPaLength;
	glm::vec3 NaOrtho = glm::cross(planeNormal, CpToPaUnit);
	
	//Final direction
	outDir = NaOrtho;
	return 1;
}



int Particle::AABBColliderAvoidance(glm::vec3 const& Va, glm::vec3 const& Pa, glm::vec3 const& Ctr,
	glm::vec3 const& planeNormal, glm::vec3 radiuses, float DMax, glm::vec3& outDir)
{
	//Ray plane intersecion
	//For now, only test for yup direction
	glm::vec3 Np = {0, 1, 0};
	glm::vec3 Po = Ctr + glm::vec3(0, radiuses.z, 0); //Ask for z cause its rotated only
	float num = glm::dot(Np, Po - Pa);
	float denom = glm::dot(Va, Np);
	float t = num / denom;

	//dSee represents if the obstacle is "visible" given the particle's direction
	float dSee = (t > 0.0f) ? 1.0f : 0.0f;
	if (dSee == 0.0f) return -3;

	//dRange represents if we are within range of the obstable to care about it
	float len = glm::length(Va * t);
	float dRange = (len <= DMax) ? 1.0f : 0.0f;
	if (dRange == 0.0f) return -2;

	//Get important data. 'Cp' is the center of the intersected circle, 's' is its radius
	outDir = {1, 1, 0};
	assert(outDir.x == outDir.x && outDir.y == outDir.y && outDir.z == outDir.z);
	return 1;
}