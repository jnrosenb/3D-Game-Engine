///HEADER STUFF

#pragma once

///INCLUDES
#include "../External/Includes/glm/glm.hpp"
#include "Quaternion.h"
#include <string>
#include <vector>


struct PosKey
{
	glm::vec3 position;
	float time;
};

struct RotKey
{
	AuxMath::Quaternion quaternion;
	float time;
};

struct ScaKey
{
	glm::vec3 scale;
	float time;
};


struct AnimChannel
{
	std::string boneName;
	std::vector<PosKey> PositionKeys;
	std::vector<RotKey> RotationKeys;
	std::vector<ScaKey> ScalingKeys;
};


struct Animation
{
	std::string animName;
	float duration;
	float ticksPerSecond;

	std::vector<AnimChannel> channels;
};