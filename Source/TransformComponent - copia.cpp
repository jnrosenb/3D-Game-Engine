///HEADER STUFF

#include "TransformComponent.h"
#include "Affine.h"


//TODO - RIGHT NOW NOT DEALING WITH HIERARCHIES***


Transform::Transform(GameObject *owner) :
	BaseComponent(owner, COMPONENT_TYPES::TRANSFORM),
	needToRecalculateModel(0)
{
	this->m_position = glm::vec4(0, 0, 0, 1);
	this->m_rotation = glm::vec4(0, 0, 0, 1); //TODO - Unused right now
	this->m_scale = glm::vec4(1, 1, 1, 1);

	this->T = glm::mat4(1);
	this->R = glm::mat4(1);
	this->H = glm::mat4(1);
	this->model = glm::mat4(1);
	this->normalsModel = glm::mat4(1);
}

Transform::~Transform()
{
	std::cout << "Destroying transform component" << std::endl;
}


void Transform::DeserializeInit()
{
	needToRecalculateModel = true;


	//Set rotation based on rotate vector
	this->R = AuxMath::rotate(m_rotation.x, glm::vec4(1, 0, 0, 0)) *
		AuxMath::rotate(m_rotation.y, glm::vec4(0, 1, 0, 0)) *
		AuxMath::rotate(m_rotation.z, glm::vec4(0, 0, 1, 0));
}

void Transform::Update(float dt)
{
	if (needToRecalculateModel) 
	{
		//Translation
		this->T[3][0] = m_position.x;
		this->T[3][1] = m_position.y;
		this->T[3][2] = m_position.z;

		//Rotation
		this->R = AuxMath::rotate(m_rotation.z, glm::vec4(0, 0, 1, 0)) *
			AuxMath::rotate(m_rotation.y, glm::vec4(0, 1, 0, 0)) *
			AuxMath::rotate(m_rotation.x, glm::vec4(1, 0, 0, 0));

		//Scale
		this->H[0][0] = m_scale.x;
		this->H[1][1] = m_scale.y;
		this->H[2][2] = m_scale.z;

		//Model update
		this->model = T * R * H;
		//Normal's model update
		glm::mat4 invH = glm::mat4(1);
		invH[0][0] = 1.0f / H[0][0];
		invH[1][1] = 1.0f / H[1][1];
		invH[2][2] = 1.0f / H[2][2];
		this->normalsModel = R;// glm::transpose(R) * invH; // TODO - Which one to use??? 
	}

	needToRecalculateModel = 0;
}

void Transform::translate(glm::vec3 const& translation) 
{
	this->translate(translation.x, translation.y, translation.z);
}


void Transform::translate(float dx, float dy, float dz)
{
	this->m_position += glm::vec4(dx, dy, dz, 1.0f);

	needToRecalculateModel = 1;
}


void Transform::rotate(glm::vec3 const& euler)
{
	this->rotate(euler.x, euler.y, euler.z);
}


void Transform::rotate(float pitch, float yaw, float roll)
{
	this->m_rotation.x += pitch;
	this->m_rotation.y += yaw;
	this->m_rotation.z += roll;

	needToRecalculateModel = 1;
}

void Transform::scale(float val)
{
	this->scale(val, val, val);
}

void Transform::scale(glm::vec3 const& scale)
{
	this->scale(scale.x, scale.y, scale.z);
}

void Transform::scale(float x, float y, float z)
{
	this->m_scale = glm::vec4(x, y, z, 1.0f);

	needToRecalculateModel = 1;
}


glm::mat4 const& Transform::GetModel()
{
	return this->model;
}


glm::mat4 const& Transform::GetNormalModel()
{
	return this->normalsModel;
}