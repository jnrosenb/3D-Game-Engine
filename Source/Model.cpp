///HEADER

#include "Model.h"
#include <iostream>
#include <limits>

#include "Sphere.h"
#include "Quad.h"
#include "Polar.h"
#include "Cube.h"
#include "Meshes/DynamicGrid.h"
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
	else if (primitive == "dynamic_plane")
		meshes.push_back(new DynamicGridMesh());

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
	//Old AABB (min max)
	///boundingBox.min.x = std::numeric_limits<float>::max();
	///boundingBox.min.y = std::numeric_limits<float>::max();
	///boundingBox.min.z = std::numeric_limits<float>::max();
	///boundingBox.max.x = std::numeric_limits<float>::lowest();
	///boundingBox.max.y = std::numeric_limits<float>::lowest();
	///boundingBox.max.z = std::numeric_limits<float>::lowest();

	float min_x = std::numeric_limits<float>::max();
	float min_y = std::numeric_limits<float>::max();
	float min_z = std::numeric_limits<float>::max();
	float max_x = std::numeric_limits<float>::lowest();
	float max_y = std::numeric_limits<float>::lowest();
	float max_z = std::numeric_limits<float>::lowest();

	//For now, create a big AABB (in body space) around all the meshes
	for (Mesh *mesh : meshes)
	{
		for (int i = 0; i < mesh->GetVertexCount(); ++i)
		{
			glm::vec4& v = mesh->GetVertices()[i];

			//Old AABB (min-max)
			/// if (v.x < boundingBox.min.x)
			/// 	boundingBox.min.x = v.x;
			/// if (v.y < boundingBox.min.y)
			/// 	boundingBox.min.y = v.y;
			/// if (v.z < boundingBox.min.z)
			/// 	boundingBox.min.z = v.z;
			/// if (v.x > boundingBox.max.x)
			/// 	boundingBox.max.x = v.x;
			/// if (v.y > boundingBox.max.y)
			/// 	boundingBox.max.y = v.y;
			/// if (v.z > boundingBox.max.z)
			/// 	boundingBox.max.z = v.z;

			min_x = (v.x < min_x) ? v.x : min_x;
			min_y = (v.y < min_y) ? v.y : min_y;
			min_z = (v.z < min_z) ? v.z : min_z;
			max_x = (v.x > max_x) ? v.x : max_x;
			max_y = (v.y > max_y) ? v.y : max_y;
			max_z = (v.z > max_z) ? v.z : max_z;
		}
	}

	// Now, use mins and max to build the AABB
	// -> Center/Radius configuration
	// -> CENTER IS NOT GONNA BE SAME AS GO'S PIVOT FOR AABB (necesarily)
	boundingBox.center = {
		0.5f * (max_x + min_x),
		0.5f * (max_y + min_y),
		0.5f * (max_z + min_z),
	};
	boundingBox.radius = {
		0.5f * std::fabsf(max_x - min_x),
		0.5f * std::fabsf(max_y - min_y),
		0.5f * std::fabsf(max_z - min_z),
	};

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