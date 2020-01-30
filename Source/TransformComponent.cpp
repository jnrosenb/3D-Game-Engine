///HEADER STUFF

#include "TransformComponent.h"
#include "Affine.h"


//TODO - RIGHT NOW NOT DEALING WITH HIERARCHIES***
#define PI			3.14159265359f


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
		this->R = AuxMath::rotate(m_rotation.z, glm::vec3(0, 0, 1)) *
			AuxMath::rotate(m_rotation.y, glm::vec3(0, 1, 0)) *
			AuxMath::rotate(m_rotation.x, glm::vec3(1, 0, 0));
		this->q = AuxMath::Quaternion(R);

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

	this->m_rotation.x = this->m_rotation.x - static_cast<int>(m_rotation.x / 360) * 360.0f;
	this->m_rotation.y = this->m_rotation.y - static_cast<int>(m_rotation.y / 360) * 360.0f;
	this->m_rotation.z = this->m_rotation.z - static_cast<int>(m_rotation.z / 360) * 360.0f;

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


glm::vec4 const& Transform::GetPosition() const
{
	return this->m_position;
}


glm::vec4 Transform::GetForward() const
{
	return glm::vec4(R[2].x, R[2].y, R[2].z, 0.0f);
}


glm::vec4 Transform::GetRight() const
{
	return glm::vec4(R[0].x, R[0].y, R[0].z, 0.0f);
}


void Transform::SetPosition(glm::vec3 const& newPos)
{
	this->m_position = glm::vec4(newPos.x, newPos.y, newPos.z, 1.0f);
	needToRecalculateModel = 1;
}


glm::mat4 const& Transform::GetModel() const
{
	return this->model;
}


glm::mat4 const& Transform::GetNormalModel() const 
{
	return this->normalsModel;
}


glm::mat4 const& Transform::GetRotationMatrix() const 
{
	return this->R;
}

AuxMath::Quaternion const& Transform::GetRotationQuaternion() const
{
	return this->q;
}

glm::vec4 const& Transform::GetScale() const
{
	return this->m_scale;
}