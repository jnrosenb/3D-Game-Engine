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
	void translate(float x, float y, float z);
	void rotate(float angle_deg, glm::vec4 const& axis);
	void scale(float val);
	void scale(float x, float y, float z);

	//Get the objects model
	glm::mat4& GetModel();
	glm::mat4& GetNormalModel();

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