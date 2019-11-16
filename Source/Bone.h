///HEADER STUFF

#pragma once

///INCLUDES
#include <vector>
#include <string>
#include "../External/Includes/glm/glm.hpp"


struct Bone 
{
	//FOR NOW PUBLIC
	int index = -1;
	std::string name;
	glm::mat4 nodeTransformation;
	glm::mat4 offsetMatrix;

	glm::mat4 accumTransformation;
	glm::mat4 vqs;

	//Probably dont need both
	bool updatedVQS;
	bool IKUseVQS;

	std::string parent;
	std::vector<std::string> children;
};