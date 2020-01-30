///HEADER

#include "Model.h"
#include <iostream>
#include <limits>

#include "Sphere.h"
#include "Quad.h"
#include "Polar.h"
#include "Cube.h"
#include "Plane.h"

#include "Mesh.h"
#include "LoadedMesh.h"

#include <unordered_map>

//Experiment
#include <string>
#include "FBXLoader.h"

#include "Paths.h"
extern EnginePaths *globalPaths;



Model::Model()
{}


Model::Model(std::string path)
{
	std::string const abs_path_prefix = globalPaths->AssetsDir + "Models\\";
	FBXLoader::ReadAssimpFile(abs_path_prefix + path, meshes, boneMap, animMap);

	//For now, real all vertices on the list of meshes, and find the AABB in body space
	CalculateAABB();
}


Model::Model(bool isPrimitive, std::string primitive)
{
	if (primitive == "quad")
		meshes.push_back(new Quad());
	else if (primitive == "sphere")
		meshes.push_back(new Sphere(32));
	else if (primitive == "polar")
		meshes.push_back(new PolarPlane(64));
	else if (primitive == "plane")
		meshes.push_back(new Plane(64));
	else if (primitive == "cube")
		meshes.push_back(new Cube());

	//For now, real all vertices on the list of meshes, and find the AABB in body space
	CalculateAABB();
}


Model::~Model()
{
	for (auto mesh : meshes)
	{
		delete mesh;
	}
	meshes.clear();
}


void Model::CalculateAABB()
{
	boundingBox.min.x = std::numeric_limits<float>::max();
	boundingBox.min.y = std::numeric_limits<float>::max();
	boundingBox.min.z = std::numeric_limits<float>::max();
	boundingBox.max.x = std::numeric_limits<float>::lowest();
	boundingBox.max.y = std::numeric_limits<float>::lowest();
	boundingBox.max.z = std::numeric_limits<float>::lowest();

	//For now, create a big AABB (in body space) around all the meshes
	for (Mesh *mesh : meshes)
	{
		for (int i = 0; i < mesh->GetVertexCount(); ++i)
		{
			glm::vec4& v = mesh->GetVertices()[i];

			if (v.x < boundingBox.min.x)
				boundingBox.min.x = v.x;
			if (v.y < boundingBox.min.y)
				boundingBox.min.y = v.y;
			if (v.z < boundingBox.min.z)
				boundingBox.min.z = v.z;
			if (v.x > boundingBox.max.x)
				boundingBox.max.x = v.x;
			if (v.y > boundingBox.max.y)
				boundingBox.max.y = v.y;
			if (v.z > boundingBox.max.z)
				boundingBox.max.z = v.z;
		}
	}

	//TODO - Later : Create a collection of AABB instead. This way, we get a more convex shape

	//TODO - Later : This would be a good place to test the quick-hull algorithm!
}


AABB Model::GetAABB() const
{
	return boundingBox;
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