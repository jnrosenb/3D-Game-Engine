///HEADER

#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include "Mesh.h"

#include <../External/Includes/Assimp/Importer.hpp>
#include <../External/Includes/Assimp/scene.h>
#include <../External/Includes/Assimp/postprocess.h>

#include "../External/Includes/glm/glm.hpp"

#include "Bone.h"
#include "Animation.h"

class LoadedMesh;


class Model 
{
public:
	Model();
	Model(std::string path);
	~Model();

	//TEMP
	void ProcessRecursiveTransformationFromRoot(Bone& node, 
		glm::mat4 const& parentTransf, std::vector<glm::mat4>& BoneTransformations);

//TODO - switch to private
public:
	std::vector<Mesh*> meshes;
	std::unordered_map<std::string, Bone> boneMap;
	std::unordered_map<std::string, Animation> animMap;

	//DEBUG DRAW OF BONES
	///std::vector<glm::vec4> bonesDrawPositions;
};