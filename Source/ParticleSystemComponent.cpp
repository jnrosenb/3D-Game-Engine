///HEADER STUFF

#include "ParticleSystemComponent.h"
#include "Affine.h"

#include "GameObject.h"
#include "Model.h"
#include "Sphere.h"
#include "Quad.h"
#include "Polar.h"
#include "Plane.h"
#include "Shader.h"

//Coupling bit
#include "TransformComponent.h"

//Temporary (while no world exists)
#include "ResourceManager.h"
extern ResourceManager *resMgr;

//For now, camera will handle all input
#include "InputManager.h"
extern InputManager *inputMgr;

//Temporary---------------
#include "PhysicsManager.h"
extern PhysicsManager *physicsMgr;

//Temporary (while no world exists)
#include "Renderer.h"
extern Renderer *renderer;


#define PI 3.14159265359f


ParticleSystemComponent::ParticleSystemComponent(GameObject *owner) :
	BaseComponent(owner, COMPONENT_TYPES::PARTICLE_SYSTEM)
{
	//For some vars which may or may not be defined in json
	xTiling = 1;
	yTiling = 1;
	useDiffuseTexture = false;
}

ParticleSystemComponent::~ParticleSystemComponent()
{
	std::cout << "Destroying ParticleSystem component" << std::endl;

	if (model)
		delete model;

	//TEMPORARY MEASURE - DELETE
	glDeleteBuffers(1, &instanceBuffer);

	//Delete all particles
	for (Particle *p : m_particles) 
	{
		delete p;
	}
	m_particles.clear();

	//Delete the particle advectors
	for (Operator *operation : m_advectors)
	{
		delete operation;
	}
	m_advectors.clear();

	//Delete the particle operators
	for (Operator *operation : m_operators)
	{
		delete operation;
	}
	m_operators.clear();
}


void ParticleSystemComponent::Update(float dt)
{
	handleInput(dt);

	//TEMPORARY IF
	if (interactive) 
	{
		//Interacting particles
		for (Particle *p : m_particles)
		{
			if (p->GetTimeToLive() > 0.0f)
				p->Update(dt, target, m_particles, physicsMgr->GetRigidbodies());
		}
	}
	else 
	{
		//Non interacting particles
		for (Particle *p : m_particles)
		{
			if (p->GetTimeToLive() > 0.0f)
				p->Update(dt);
		}
	}
}


void ParticleSystemComponent::Draw()
{
	int count = 20;
	DrawInstanceData data = {};

	int index = 0;
	for (Particle *p : m_particles)
	{
		if (p->GetTimeToLive() <= 0.0f)
			continue;

		glm::mat4 modelMatrix(1);
		
		//Old bad way
		///modelMatrix[0][0] = 0.25f;
		///modelMatrix[1][1] = 0.25f;
		///modelMatrix[2][2] = 0.25f;
 		///modelMatrix[3] = glm::vec4(p->GetPosition(), 1.0f);
		//New good way
		glm::mat3 pH(1);
		pH[0][0] = p->GetSize().x;
		pH[1][1] = p->GetSize().y;
		pH[2][2] = p->GetSize().z;
		glm::mat3 const& pR = p->GetRotationMatrix();
		modelMatrix = pR * pH;
		modelMatrix[3] = glm::vec4(p->GetPosition(), 1.0f);

		modelMatrices[index] = modelMatrix;
		++index;
		
		data.instanceCount += 1;
	}

	//TEMPORARY MEASURE
	data.TempVBO = instanceBuffer;
	data.useDiffuseTexture = useDiffuseTexture;
	data.diffuseTexture = renderer->GetTexture(this->diffuseTexture);
	data.diffuseColor = glm::vec4(1, 0, 0, 1);
	data.meshes = &model->meshes;
	data.modelMatrices = &modelMatrices;
	data.xTiling = xTiling;
	data.yTiling = yTiling;

	//Send instance drawData to graphics manager
	renderer->QueueForDrawInstanced(data);
}


void ParticleSystemComponent::DeserializeInit()
{
	//init mesh data
	initModel();

	//TEMPORARY MEASURE - GEN BUFFER
	glGenBuffers(1, &instanceBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, instanceBuffer);
	glBufferData(GL_ARRAY_BUFFER, this->count * sizeof(glm::mat4), NULL, GL_DYNAMIC_DRAW);

	//reserve size for model matrices
	modelMatrices.resize(this->count);

	//Create the initial number of particles to later pool
	for (int i = 0; i < count; ++i)
	{
		Particle *p = interactive ? new InteractingParticle() : new Particle();
		particleDescriptor desc = {};
		desc.timeToLive = -1.0f;
		
		//TEMPORARY IF
		if (interactive)
			p->Initialize(desc, m_flockingParams);
		else
			p->Initialize(desc, &m_advectors, &m_operators);

		m_particles.push_back(p);
	}

	//CHANGE THIS FOR A DYNAMIC TARGET
	target = glm::vec3(-25, 4, -3);
}


//Later add use of a particle descriptor struct
void ParticleSystemComponent::EmitOnce(int num, float ttl, EMISSION_SHAPE shape)
{
	//For now - Cache trasformComp to get pos
	Transform *T = this->m_owner->GetComponent<Transform>();
	
	//Cap the number of particles to this emitter's maximum (while no mem mgr)
	num = (num <= count) ? num : count;

	//LOOP THROUGH PARTICLES
	int i = 0;
	for (Particle *p : m_particles)
	{
		//Skip particles that are already alive
		if (p->GetTimeToLive() > 0.0f) continue;

		//Don't emit more than num
		if (i == num) return;

		//Initial position
		glm::vec3 pos = T->GetPosition();
		SampleSpawnPosition(shape, pos);

		//Initial velocity
		glm::vec3 v0 = glm::vec3(0);//pos - glm::vec3(T->GetPosition().x, T->GetPosition().y, T->GetPosition().z);

		particleDescriptor desc = {};
		desc.spawnerPosition = glm::vec3(T->GetPosition());
		desc.timeToLive = ttl;
		desc.position = pos;
		desc.axis = glm::vec3(0, 1, 0);
		desc.size = size;

		//TEMPORARY IF
		if (interactive)
			p->Initialize(desc, m_flockingParams, v0);
		else
			p->Initialize(desc, &m_advectors, &m_operators, v0);

		++i;
	}
}


void ParticleSystemComponent::SampleSpawnPosition(EMISSION_SHAPE shape, 
	glm::vec3& pos)
{
	if (shape == EMISSION_SHAPE::SPHERE) 
	{
		//HARDCODED FOR NOW
		float radius = 6.0f;

		//Shitty way of sampling uniform cube and discarding
		float x, y, z;
		while (1)
		{
			x = 2*radius * static_cast<float>(rand()) / static_cast<float>(RAND_MAX) - radius;
			y = 2*radius * static_cast<float>(rand()) / static_cast<float>(RAND_MAX) - radius;
			z = 2*radius * static_cast<float>(rand()) / static_cast<float>(RAND_MAX) - radius;
			if (x*x + y*y + z*z > radius*radius)
				continue;
			break;
		}
		pos = glm::vec3(pos.x + x, pos.y + y, pos.z + z);
	}

	//END ASSERTION
	assert(pos.x == pos.x && pos.y == pos.y && pos.z == pos.z);
}


void ParticleSystemComponent::initModel()
{
	if (use_loaded_mesh)
	{
		this->model = new Model(modelPath);
	}
	else
	{
		this->model = new Model();
		if (this->primitive == "quad")
			this->model->meshes.push_back(new Quad());
		else if (this->primitive == "sphere")
			this->model->meshes.push_back(new Sphere(32));
		else if (this->primitive == "polar")
			this->model->meshes.push_back(new PolarPlane(64));
		else if (this->primitive == "plane")
			this->model->meshes.push_back(new Plane(64));
	}
}


//Pushes back an operator which later will be referenced by the particles
void ParticleSystemComponent::RegisterParticleAdvector(Operator *op)
{
	if (op != nullptr)
		m_advectors.push_back(op);
}


//Pushes back an operator which later will be referenced by the particles
void ParticleSystemComponent::RegisterParticleOperator(Operator *op)
{
	if (op != nullptr)
		m_operators.push_back(op);
}




////////////////////////////////
////		HANDLE INPUT	////
////////////////////////////////
void ParticleSystemComponent::handleInput(float dt)
{
	if (inputMgr->getKeyTrigger(SDL_SCANCODE_RETURN))
	{
		std::cout << "SPAWNING DEAD PARTICLES" << std::endl;
		EmitOnce(500, 50.0f, EMISSION_SHAPE::SPHERE);
	}

	if (inputMgr->getKeyPress(SDL_SCANCODE_LEFT))
	{
		target = target - glm::vec3(1, 0, 0);
	}
	if (inputMgr->getKeyPress(SDL_SCANCODE_RIGHT))
	{
		target = target + glm::vec3(1, 0, 0);
	}
}