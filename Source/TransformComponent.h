///HEADER STUFF

#pragma once

///INCLUDES
#include "BaseComponent.h"
#include "Quaternion.h"
#include "../External/Includes/glm/glm.hpp"


class Transform : public BaseComponent
{
public:
	friend class Factory;

//PUBLIC INTERFACE
public:
	Transform(GameObject *owner);
	virtual ~Transform();

	virtual Transform* clone() override 
	{
		return new Transform(m_owner);
	}
	virtual void Update(float dt) override;
	virtual void DeserializeInit() override;

	//To compare when using templates
	static COMPONENT_TYPES const comp_class_type = TRANSFORM;

	//To modify the object
	void translate(float dx, float dy, float dz);
	void translate(glm::vec3 const& translation);
	void rotate(float pitch, float yaw, float roll);
	void rotate(glm::vec3 const& euler);
	void rotate(AuxMath::Quaternion const& dq);
	void rotateWorld(float pitch, float yaw, float roll);
	void rotateWorld(AuxMath::Quaternion const& dq);
	void scale(float val);
	void scale(float x, float y, float z);
	void scale(glm::vec3 const& scale);

	//Global modifiers
	void SetPosition(glm::vec3 const& newPos);
	void SetRotation(float ex, float ey, float ez);
	void SetRotation(glm::vec3 const& eulerAngles);
	void SetRotation(AuxMath::Quaternion const& q);
	glm::vec4 const& GetPosition() const;
	glm::vec4 GetForward() const;
	glm::vec4 GetRight() const;

	//GETTERS
	glm::vec4 const& GetScale() const;

	//Get the objects model
	glm::mat4 const& GetModel() const;
	glm::mat4 const& GetNormalModel() const;
	glm::mat4 const& GetRotationMatrix() const;
	AuxMath::Quaternion const& GetRotationQuaternion() const;

private:
	//Flag for knowing if the object moved
	bool needToRecalculateModel;
	bool needToRecalculateModel_world;

	//Object data
	glm::vec4 m_position;
	glm::vec4 m_rotation;
	glm::vec4 m_rotationWorld;
	glm::vec4 m_scale;

	glm::mat4 T, R, H;
	AuxMath::Quaternion q;
	glm::mat4 model;
	glm::mat4 normalsModel;
};