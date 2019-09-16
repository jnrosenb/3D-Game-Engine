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


/*
Transform* clone() 
{
	return new Transform();
}
//*/

void Transform::Update(float dt)
{
	if (needToRecalculateModel) 
	{
		this->model = T * R * H;
		this->normalsModel = R;
	}

	needToRecalculateModel = 0;
}


void Transform::translate(float x, float y, float z)
{
	this->m_position += glm::vec4(x, y, z, 0.0f);

	this->T[3][0] = m_position.x;
	this->T[3][1] = m_position.y;
	this->T[3][2] = m_position.z;

	needToRecalculateModel = 1;
}

void Transform::rotate(float angle_deg, glm::vec4 const& axis)
{
	this->R = AuxMath::rotate(angle_deg, axis);

	needToRecalculateModel = 1;
}

void Transform::scale(float val)
{
	scale(val, val, val);
}

void Transform::scale(float x, float y, float z)
{
	this->m_scale = glm::vec4(x, y, z, 1.0f);

	this->H[0][0] = m_scale.x;
	this->H[1][1] = m_scale.y;
	this->H[2][2] = m_scale.z;

	needToRecalculateModel = 1;
}


glm::mat4& Transform::GetModel()
{
	return this->model;
}


glm::mat4& Transform::GetNormalModel()
{
	return this->normalsModel;
}