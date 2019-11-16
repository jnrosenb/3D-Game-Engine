///HEADER STUFF

#pragma once

///INCLUDES
#include "BaseComponent.h"
#include <vector>
#include <map>
#include <tuple>
#include <list>
#include "../External/Includes/glm/glm.hpp"


class PathFollowComponent : public BaseComponent
{
public:
	friend class Factory;

//PUBLIC INTERFACE
public:
	PathFollowComponent(GameObject *owner);
	virtual ~PathFollowComponent();

	virtual PathFollowComponent* clone() override
	{
		return new PathFollowComponent(m_owner);
	}
	virtual void Update(float dt) override;
	virtual void LateUpdate(float dt) override;
	virtual void DeserializeInit() override;

private:
	//Temporary TODO REMOVE
	void handleInput(float dt);

	void generateArcLenTable();

public:
	//Control points for the curve
	std::vector<glm::vec4> m_points;
	std::vector<glm::vec4> m_curve_vector;
	std::map<float, glm::vec4> m_curve;

	//Arclen table
	std::vector<std::tuple<float,float>> arclenTable;

	//Path following parameters
	float speed;

	//animT goes from 0 to globalT (global to the whole curve)
	float globalT;
	float t1;
	float t2;

	//Accum values
	float animTime;
	float animDist;

	//Bools
	bool isRunning;
	bool drawPath;

	//To compare when using templates
	static COMPONENT_TYPES const comp_class_type = PATH_FOLLOW;
};