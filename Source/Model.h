///HEADER

#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include <../External/Includes/Assimp/Importer.hpp>
#include <../External/Includes/Assimp/scene.h>
#include <../External/Includes/Assimp/postprocess.h>

#include "../External/Includes/glm/glm.hpp"

#include "Bone.h"
#include "Animation.h"
#include "Shapes.h"

class LoadedMesh;
class Mesh;


class Model 
{
public:
	Model();
	Model(std::string path);
	Model(bool primitive, std::string path);
	~Model();

	AABB GetAABB() const;

	//TEMP
	void ProcessRecursiveTransformationFromRoot(Bone& node, 
		glm::mat4 const& parentTransf, std::vector<glm::mat4>& BoneTransformations);

private:
	void CalculateAABB();

//TODO - switch to private
public:
	std::vector<Mesh*> meshes;
	std::unordered_map<std::string, Bone> boneMap;
	std::unordered_map<std::string, Animation> animMap;

private:
	AABB boundingBox;
};