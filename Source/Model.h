///HEADER

#pragma once

#include <string>
#include <vector>
#include "Mesh.h"

#include <../External/Includes/Assimp/Importer.hpp>
#include <../External/Includes/Assimp/scene.h>
#include <../External/Includes/Assimp/postprocess.h>

class LoadedMesh;

class Model 
{
public:
	Model(std::string path);
	~Model();

public: //TODO - switch to private
	std::vector<Mesh*> meshes;
	std::string dir_path;

private:
	void load_model(std::string path);
	void processNode(aiNode *node, const aiScene *scene);
	LoadedMesh *processMesh(aiMesh *mesh, const aiScene *scene);

	///Check this texture class later
	//std::vector<Texture> loadMaterialTextures(aiMaterial *mat, 
	//	aiTextureType type, string typeName);
};