//////////////////////////////////////////////////////////////////////////////
//// Uses the ASSIMP library to read an animation file and print most of  ////
//// the relevant information.                                            ////
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>

#include <../External/Includes/Assimp/Importer.hpp>
#include <../External/Includes/Assimp/scene.h>
#include <../External/Includes/Assimp/postprocess.h>

#include "../External/Includes/glm/glm.hpp"

#include "Bone.h"
#include "Animation.h"
class Skeleton;
class Mesh;


//GLOBALS AND MACROS
#define MAX				3
#define MAX_BONES		200


namespace FBXLoader
{
	// Reads the assimp FBX and starts the unwrapping process
	void ReadAssimpFile(std::string& path, std::vector<Mesh*>& meshes,
		std::unordered_map<std::string, Bone>& boneMap,
		std::unordered_map<std::string, Animation>& animMap);


	// Prints a mesh's info; A mesh contains vertices, faces, normals and
	// more as needed for graphics.  Vertices are tied to bones with
	// weights.
	void ProcessMesh(aiMesh* mesh,
		std::unordered_map<std::string, glm::mat4>& BoneOffsetMap, 
		std::vector<Mesh*>& meshes,
		std::unordered_map<std::string, int> const& BoneNameIdMap);


	// Prints an animation.  An animation contains a few timing parameters
	// and then channels for a number of animated bones.  Each channel
	// contains a V, Q, and S keyframe sequences.
	void ProcessAnimation(aiAnimation* anim, std::unordered_map<std::string,Animation>& myAnimMap);


	// Process the nodes and get some bone information from here
	void ProcessBoneHierarchy(const aiScene* scene, const aiNode* node, 
		std::string const& parentName, 
		std::unordered_map<std::string, Bone>& myBoneMap,
		std::unordered_map<std::string, int>& BoneNameIdMap, int boneIndex);


	// TODO - check if the convertion is actually done correctly
	glm::mat4 aiMat_To_glmMat(aiMatrix4x4 const& inMat);


	// TODO - check if the convertion is actually done correctly
	aiMatrix4x4 glmMat_to_aiMat(glm::mat4 const& inMat);


	//Calculates total amount of vertices through all meshes in scene
	int CalculateTotalVertexCount(aiScene const *scene);


	//METHODS FOR GETTING KEY FRAMES
	std::vector<PosKey> GetPositionKeyFrames(aiNodeAnim *animNode);
	std::vector<RotKey> GetRotationKeyFrames(aiNodeAnim *animNode);
	std::vector<ScaKey> GetScalingKeyFrames(aiNodeAnim *animNode);
}

