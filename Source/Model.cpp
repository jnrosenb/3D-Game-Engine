///HEADER

#include "Model.h"
#include <iostream>
#include <limits>

#include "Mesh.h"
#include "LoadedMesh.h"

#include <unordered_map>

//Experiment
#include <string>
#include "FBXLoader.h"



Model::Model()
{}


Model::Model(std::string path)
{
	//OLD WAY----------------------------------------------
	///load_model(path);

	//NEW WAY----------------------------------------------
	FBXLoader::ReadAssimpFile(path, meshes, boneMap, animMap);
}


Model::~Model()
{
	for (auto mesh : meshes)
	{
		delete mesh;
	}
	meshes.clear();
}


void Model::ProcessRecursiveTransformationFromRoot(Bone& node, 
	glm::mat4 const& parentTransf, std::vector<glm::mat4>& BoneTransformations)
{

	//If bone had VQS calculated, we use that instead of nodeTransformation
	if (node.updatedVQS) 
	{
		node.updatedVQS = false;
		node.accumTransformation = parentTransf * node.vqs;
	}
	else 
	{
		//does nodeTransformation otherwise hold accum data?
		node.accumTransformation = parentTransf * node.nodeTransformation;
	}

	//Continue with children
	for (int i = 0; i < node.children.size(); ++i)
	{
		Bone& child = this->boneMap[node.children[i]];
		ProcessRecursiveTransformationFromRoot(child, node.accumTransformation, BoneTransformations);
	}

	//Update stuff using offset matrix
	glm::mat4 const result = node.accumTransformation * node.offsetMatrix;
	BoneTransformations[node.index] = result;
}