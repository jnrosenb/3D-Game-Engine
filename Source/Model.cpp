///HEADER

#include "Model.h"
#include <iostream>
#include <limits>

#include "Mesh.h"
#include "LoadedMesh.h"

//Experiment
#include <string>
#include "FBXLoader.h"



Model::Model(std::string path)
{
	load_model(path);

	//Experiment
	///std::string path2 = "gh_sample_animation.fbx";
	///FBXLoader::ReadAssimpFile(path2);
}


Model::~Model()
{
	for (auto mesh : meshes) 
	{
		delete mesh;
	}
	meshes.clear();
}


void Model::load_model(std::string path)
{
	Assimp::Importer importer;
	aiScene const *scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs |
		aiProcess_CalcTangentSpace );

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) 
	{
		std::cout << "ERROR LOADING MESH >> " << importer.GetErrorString() << std::endl;
		return;
	}

	dir_path = path.substr(0, path.find_last_of('/'));
	processNode(scene->mRootNode, scene);
}


void Model::processNode(aiNode *node, const aiScene *scene)
{
	//std::cout << "Entering processNode." << std::endl;

	//Process the meshes
	for (unsigned int i = 0; i < node->mNumMeshes; ++i) 
	{
		aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
		//std::cout << "\t>> Got Mesh from scene." << std::endl;
		LoadedMesh *processed_mesh = processMesh(mesh, scene);
		//std::cout << "\t>> Processed Mesh." << std::endl;
		meshes.push_back(processed_mesh);
		//std::cout << "\t>> Stored Mesh on vector." << std::endl;
	}

	for (unsigned int i = 0; i < node->mNumChildren; ++i) 
	{
		aiNode *child = node->mChildren[i];
		processNode(child, scene);
	}
}


LoadedMesh* Model::processMesh(aiMesh *mesh, const aiScene *scene)
{
	//vector<Vertex> vertices;
	//vector<unsigned int> indices;
	//vector<Texture> textures;

	std::vector<glm::vec4> vertices, normals, tgts, bitgts;
	std::vector<glm::vec2> uvs;
	std::vector<Mesh::Face> faces;

	//VERTICES- NORMALS- UVs - tgts - bitgts
	for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
	{
		// process vertex positions
		aiVector3D vertex = mesh->mVertices[i];
		glm::vec4 transf_vertex = glm::vec4(vertex.x, vertex.y, vertex.z, 1.0f);
		vertices.push_back(transf_vertex);

		// process normals
		aiVector3D normal = mesh->mNormals[i];
		glm::vec4 transf_normal = glm::vec4(normal.x, normal.y, normal.z, 0.0f);
		normals.push_back(transf_normal);

		// process tangents
		aiVector3D tgt = mesh->mTangents[i];
		glm::vec4 transf_tgt = glm::vec4(tgt.x, tgt.y, tgt.z, 0.0f);
		tgts.push_back(transf_tgt);
		
		// process bitangents
		aiVector3D bitgt = mesh->mBitangents[i];
		glm::vec4 transf_bitgt = glm::vec4(bitgt.x, bitgt.y, bitgt.z, 0.0f);
		bitgts.push_back(transf_bitgt);

		// process texture coordinates
		aiVector3D uv = mesh->mTextureCoords[0][i];
		glm::vec2 transf_uv = glm::vec2(uv.x, uv.y);
		uvs.push_back(transf_uv);
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
	
	// MATERIAL PROCESSING (NOT IMPLEMENTED)
	if (mesh->mMaterialIndex >= 0)
	{
		//...
	}

	//Here we could pass texture or material according to learnOpengl
	return new LoadedMesh(vertices, normals, uvs, faces, tgts, bitgts);
}