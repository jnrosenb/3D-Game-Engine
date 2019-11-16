///HEADER STUFF

#pragma once

///INCLUDES
#include "BaseComponent.h"
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
	void scale(float val);
	void scale(float x, float y, float z);
	void scale(glm::vec3 const& scale);

	//Global modifiers
	void SetPosition(glm::vec3 const& newPos);
	glm::vec4 const& GetPosition() const;
	glm::vec4 GetForward() const;
	glm::vec4 GetRight() const;

	//Get the objects model
	glm::mat4 const & GetModel();
	glm::mat4 const & GetNormalModel();

private:
	//Flag for knowing if the object moved
	bool needToRecalculateModel;

	//Object data
	glm::vec4 m_position;
	glm::vec4 m_rotation;
	glm::vec4 m_scale;

	glm::mat4 T, R, H;
	glm::mat4 model;
	glm::mat4 normalsModel;
};