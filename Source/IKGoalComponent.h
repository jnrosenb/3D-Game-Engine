///HEADER STUFF

#pragma once

///INCLUDES
#include "BaseComponent.h"
#include <vector>
#include <unordered_map>
#include <string>
#include "../External/Includes/glm/glm.hpp"
#include "Bone.h"

class Model;


//Had to create this in order to store the
//info that later was needed
struct IK_Joint 
{
	Bone *joint;
	glm::vec3 rootPos;

	IK_Joint(Bone *j) : joint(j)
	{}
};



class IKGoalComponent : public BaseComponent
{
public:
	friend class Factory;

//PUBLIC INTERFACE
public:
	IKGoalComponent(GameObject *owner);
	virtual ~IKGoalComponent();

	virtual IKGoalComponent* clone() override
	{
		return new IKGoalComponent(m_owner);
	}
	virtual void Update(float dt) override;
	virtual void LateUpdate(float dt) override;
	virtual void DeserializeInit() override;

	glm::vec3 const& GetWorldCoordGoal() const;

private:
	//Temporary TODO REMOVE
	void handleInput(float dt);

	void Calculate_Angular_Velocities(std::vector<glm::vec3>& J, 
		glm::vec3& P, std::vector<float>& w);
	void PassDrawDataToRenderer();

public:
	//Control points for the curve
	std::vector<std::string> endEffectorsKeys;
	std::unordered_map<std::string, Bone> *boneMap;
	std::string endEffector;
	int jointDepth;
	int steps;

	//This position will fake a transform
	glm::vec3 Goal;
	bool resolvingIkSystem;
	float timeElapsed;

	//For drawing the control point
	std::string const abs_path_prefix = "C:\\Users\\Jose\\Desktop\\OpenGl_Framework\\Assets\\Models\\";
	Model *goalModel;

	float YAXIS;

	//To compare when using templates
	static COMPONENT_TYPES const comp_class_type = IK_GOAL;
};