//////////////////////////////////////////////////////////////////////////////
//// Uses the ASSIMP library to read an animation file and print most of  ////
//// the relevant information.                                            ////
//////////////////////////////////////////////////////////////////////////////


#include "FBXLoader.h"

#include "Mesh.h"
#include "LoadedMesh.h"

#include "Skeleton.h"


namespace FBXLoader
{
	// Reads the assimp FBX and starts the unwrapping process
	void ReadAssimpFile(std::string& path, std::vector<Mesh*>& meshes,
		std::unordered_map<std::string, Bone>& boneMap,
		std::unordered_map<std::string, Animation>& animMap)
	{
		std::printf("Reading %s\n", path.c_str());
		Assimp::Importer importer;

		// Get the aiScene obj
		///path = Animation_Global_Path + path;
		aiScene const *scene = importer.ReadFile(path.c_str(),
			aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_CalcTangentSpace);
		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			std::cout << "ERROR LOADING MESH >> " << importer.GetErrorString() << std::endl;
			return;
		}


		//Not sure if keeping this way
		int totalNumberOfVertices = CalculateTotalVertexCount(scene);
		std::unordered_map<std::string, glm::mat4>	BoneOffsetMap;
		std::unordered_map<std::string, int> BoneNameIdMap;


		// Save bone data
		ProcessBoneHierarchy(scene, scene->mRootNode, "", boneMap, BoneNameIdMap, 0);
		
		
		//Loop through bones and id them
		int index = 0;
		for (auto& node : boneMap) 
		{
			Bone& bone = node.second;
			bone.index = index;
			BoneNameIdMap[bone.name] = index;
			++index;
		}


		// Stores all the mesh info
		for (int i = 0; i < scene->mNumMeshes; ++i)
			ProcessMesh(scene->mMeshes[i], BoneOffsetMap, meshes, BoneNameIdMap);


		//Retrieve the offset matrix for the bones we care about
		for (auto& node : boneMap)
		{
			Bone& bone = node.second;
			bone.offsetMatrix = BoneOffsetMap[bone.name];
		}


		// Prints all the anim info for each animation
		std::printf("Processing the animations\n");
		for (int i = 0; i < scene->mNumAnimations; ++i)
			ProcessAnimation(scene->mAnimations[i], animMap);
	}

	
	// Process the aiMesh and get all the information
	void ProcessMesh(aiMesh* mesh,
		std::unordered_map<std::string, glm::mat4>& BoneOffsetMap, 
		std::vector<Mesh*>& meshes, 
		std::unordered_map<std::string, int> const& BonesNameIdMap)
	{
		//Uninitialized data vectors
		std::vector<glm::vec4> vertices, normals, tgts, bitgts;
		std::vector<glm::vec2> uvs;
		std::vector<Mesh::Face> faces;
		std::vector< std::vector<int> > bones_indices(mesh->mNumVertices);
		std::vector< std::vector<float> > bones_weights(mesh->mNumVertices);

		//VERTICES- NORMALS- UVS - TGS - BI_TGS
		for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
		{
			// process vertex positions
			if (mesh->mVertices)
			{
				aiVector3D vertex = mesh->mVertices[i];
				glm::vec4 transf_vertex = glm::vec4(vertex.x, vertex.y, vertex.z, 1.0f);
				vertices.push_back(transf_vertex);
			}

			// process normals
			if (mesh->mNormals)
			{
				aiVector3D normal = mesh->mNormals[i];
				glm::vec4 transf_normal = glm::vec4(normal.x, normal.y, normal.z, 0.0f);
				normals.push_back(transf_normal);
			}

			// process tangents
			if (mesh->mTangents)
			{
				aiVector3D tgt = mesh->mTangents[i];
				glm::vec4 transf_tgt = glm::vec4(tgt.x, tgt.y, tgt.z, 0.0f);
				tgts.push_back(transf_tgt);
			}

			// process bitangents
			if (mesh->mBitangents)
			{
				aiVector3D bitgt = mesh->mBitangents[i];
				glm::vec4 transf_bitgt = glm::vec4(bitgt.x, bitgt.y, bitgt.z, 0.0f);
				bitgts.push_back(transf_bitgt);
			}

			// process texture coordinates
			if (mesh->mTextureCoords && mesh->mTextureCoords[0])
			{
				aiVector3D uv = mesh->mTextureCoords[0][i];
				glm::vec2 transf_uv = glm::vec2(uv.x, uv.y);
				uvs.push_back(transf_uv);
			}
		}

		//INDICES PROCESSING
		for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
		{
			aiFace face = mesh->mFaces[i];
			Mesh::Face triangle;
			for (unsigned int j = 0; j < face.mNumIndices; j++)
				triangle.index[j] = face.mIndices[j];
			faces.push_back(triangle);
		}

		//BONES PROCESSING
		for (unsigned int i = 0; i < mesh->mNumBones; ++i)
		{
			aiBone *bone = mesh->mBones[i];
			int boneIndex = BonesNameIdMap.find(bone->mName.C_Str())->second;

			//Once we have the bone, we need to get the vertex info and store it per vertex
			for (int j = 0; j < bone->mNumWeights; ++j)
			{
				aiVertexWeight node = bone->mWeights[j];
				bones_indices[node.mVertexId].push_back(boneIndex);
				bones_weights[node.mVertexId].push_back(node.mWeight);
			}

			//Also, hash the offset matrix to the name of the bone
			aiMatrix4x4 const& om = bone->mOffsetMatrix;
			glm::mat4 offsetMatrix = aiMat_To_glmMat(om);
			BoneOffsetMap[bone->mName.C_Str()] = offsetMatrix;
		}

		// MATERIAL PROCESSING (NOT IMPLEMENTED)
		if (mesh->mMaterialIndex >= 0)
		{
			//...
		}

		//Here we could pass texture or material according to learnOpengl
		Mesh* m = new LoadedMesh(vertices, normals, uvs, faces, tgts,
			bitgts, bones_indices, bones_weights, BoneOffsetMap);
		meshes.push_back(m);
	}



	// Process an animation and stores all the channels and info in myAnimList
	void ProcessAnimation(aiAnimation* aiAnim, std::unordered_map<std::string, Animation>& myAnimMap)
	{
		Animation animation;
		animation.animName = aiAnim->mName.C_Str();
		animation.ticksPerSecond = aiAnim->mTicksPerSecond;
		animation.duration = aiAnim->mDuration;

		// Animations Channels (One per aiNode)
		for (int i = 0; i < aiAnim->mNumChannels; ++i)
		{
			aiNodeAnim* animNode = aiAnim->mChannels[i];

			//Create a new animation
			AnimChannel anim;
			///anim.animName = aiAnim->mName.C_Str();
			///anim.ticksPerSecond = aiAnim->mTicksPerSecond;
			///anim.duration = aiAnim->mDuration;

			anim.boneName = animNode->mNodeName.C_Str();
			anim.PositionKeys = GetPositionKeyFrames(animNode);		//Vector of PosKey
			anim.RotationKeys = GetRotationKeyFrames(animNode);		//Vector of RotKey
			anim.ScalingKeys = GetScalingKeyFrames(animNode);		//Vector of ScaKey 

			animation.channels.push_back(anim);
		}

		myAnimMap[animation.animName] = animation;
	}



	// Process the nodes and get some bone information from here
	void ProcessBoneHierarchy(const aiScene* scene, const aiNode* node,
		std::string const& parentName, 
		std::unordered_map<std::string, Bone>& myBoneMap,
		std::unordered_map<std::string, int>& BoneNameIdMap, int boneIndex)
	{
		//HERE  WE GO THROUGH THE NODE HIERARCHY. NOTE, NOT ALL
		//NODES ARE NECESSARILY BONES!

		//Create bone and store node info in it
		Bone bone;
		bone.updatedVQS = false;
		bone.name = node->mName.C_Str();
		bone.parent = parentName;

		//Transformations
		bone.nodeTransformation = aiMat_To_glmMat(node->mTransformation);
		bone.vqs = glm::mat4(1);
		bone.accumTransformation = glm::mat4(1);

		// Recurse onto this node's children
		for (unsigned i = 0; i < node->mNumChildren; ++i) 
		{
			bone.children.push_back(node->mChildren[i]->mName.C_Str());

			ProcessBoneHierarchy(scene, node->mChildren[i], bone.name,
				myBoneMap, BoneNameIdMap, boneIndex + i + 1);
		}

		//Push bone (copy) to the vector
		myBoneMap[bone.name] = bone;
	}



	// TODO - check if the convertion is actually done correctly
	glm::mat4 aiMat_To_glmMat(aiMatrix4x4 const& inMat)
	{
		glm::mat4 mat;

		//mat is a column major (first index is column?)
		mat[0][0] = inMat.a1;
		mat[1][0] = inMat.a2;
		mat[2][0] = inMat.a3;
		mat[3][0] = inMat.a4;

		mat[0][1] = inMat.b1;
		mat[1][1] = inMat.b2;
		mat[2][1] = inMat.b3;
		mat[3][1] = inMat.b4;

		mat[0][2] = inMat.c1;
		mat[1][2] = inMat.c2;
		mat[2][2] = inMat.c3;
		mat[3][2] = inMat.c4;

		mat[0][3] = inMat.d1;
		mat[1][3] = inMat.d2;
		mat[2][3] = inMat.d3;
		mat[3][3] = inMat.d4;

		//TODO - check if this is right
		return mat;
	}


	// TODO - check if the convertion is actually done correctly
	aiMatrix4x4 glmMat_to_aiMat(glm::mat4 const& inMat) 
	{
		return aiMatrix4x4
		(
			inMat[0][0], inMat[0][1], inMat[0][2], inMat[0][3],
			inMat[1][0], inMat[1][1], inMat[1][2], inMat[1][3],
			inMat[2][0], inMat[2][1], inMat[2][2], inMat[2][3],
			inMat[3][0], inMat[3][1], inMat[3][2], inMat[3][3]
		);
	}


	//Calculates the sum of vertices from all meshes
	int CalculateTotalVertexCount(aiScene const *scene)
	{
		int sum = 0;
		for (int i = 0; i < scene->mNumMeshes; ++i)
		{
			aiMesh *mesh = scene->mMeshes[i];
			sum += mesh->mNumVertices;
		}
		return sum;
	}


	//METHODS FOR GETTING KEY FRAMES
	std::vector<PosKey> GetPositionKeyFrames(aiNodeAnim *animNode) 
	{
		std::vector<PosKey> positions;
		for (int i = 0; i < animNode->mNumPositionKeys; ++i) 
		{
			PosKey pos;
			aiVectorKey PKey = animNode->mPositionKeys[i];
			pos.position = glm::vec3(PKey.mValue.x, PKey.mValue.y, PKey.mValue.z);
			pos.time = PKey.mTime;
			positions.push_back(pos);
		}
		return positions;
	}

	//METHODS FOR GETTING KEY FRAMES
	std::vector<RotKey> GetRotationKeyFrames(aiNodeAnim *animNode)
	{
		std::vector<RotKey> quaternions;
		for (int i = 0; i < animNode->mNumRotationKeys; ++i)
		{
			RotKey rot;
			aiQuatKey RKey = animNode->mRotationKeys[i];
			rot.quaternion = AuxMath::Quaternion(RKey.mValue.w, glm::vec3(RKey.mValue.x, RKey.mValue.y, RKey.mValue.z));
			rot.time = RKey.mTime;
			quaternions.push_back(rot);
		}
		return quaternions;
	}

	//METHODS FOR GETTING KEY FRAMES
	std::vector<ScaKey> GetScalingKeyFrames(aiNodeAnim *animNode)
	{
		std::vector<ScaKey> scales;
		for (int i = 0; i < animNode->mNumScalingKeys; ++i)
		{
			ScaKey scale;
			aiVectorKey SKey = animNode->mScalingKeys[i];
			scale.scale = glm::vec3(SKey.mValue.x, SKey.mValue.y, SKey.mValue.z);
			scale.time = SKey.mTime;
			scales.push_back(scale);
		}
		return scales;
	}

}

