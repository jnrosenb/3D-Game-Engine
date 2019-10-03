///HEADER STUFF

#pragma once

///INCLUDES
#include "BaseComponent.h"

#include "Bone.h"
#include "Animation.h"
#include <vector>
#include <unordered_map>

class Model;

#define EPSILON		0.000001f


class AnimationComponent : public BaseComponent
{
public:
	friend class Factory;

//PUBLIC INTERFACE
public:
	AnimationComponent(GameObject *owner);
	virtual ~AnimationComponent();

	virtual AnimationComponent* clone() override
	{
		return new AnimationComponent(m_owner);
	}
	virtual void Update(float dt) override;
	virtual void DeserializeInit() override;

	void PassAnimationInfo();

private:
	glm::vec3 CalculateInterpPos(float AnimationTime, AnimChannel const& animChannel);
	AuxMath::Quaternion CalculateInterpRot(float AnimationTime, AnimChannel const& animChannel);
	glm::vec3 CalculateInterpScale(float AnimationTime, AnimChannel const& animChannel);

public:
	//To compare when using templates
	static COMPONENT_TYPES const comp_class_type = ANIMATION;

	//Vector that, for now, will be used to pass info to shader as uniform
	std::vector<glm::mat4> BoneTransformations;


private:
	//Animation parameters
	std::string currentAnimation;
	float AnimationTime;
	float duration;
	float currentTPS;
	bool looping;
};