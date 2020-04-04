///HEADER

#pragma once

#include <unordered_map>
#include "Mesh.h"


class LoadedMesh : public Mesh
{

public:
	friend class Model;

public:
	LoadedMesh
	(
		std::vector<glm::vec4> &vertices, std::vector<glm::vec4> &normals,
		std::vector<glm::vec2> &uvs, std::vector<Mesh::Face> &faces,
		std::vector<glm::vec4> &tangents, std::vector<glm::vec4> &bitangents,
		std::vector<std::vector<int>>& bones_indices,
		std::vector<std::vector<float>>& bones_weights,
		std::unordered_map<std::string, glm::mat4>& BoneOffsetMap
	);
	virtual ~LoadedMesh();

	//GETTERS (overrides)
	virtual std::vector <glm::vec4>& GetVertices() override;
	virtual std::vector <glm::vec4>& GetNormals() override;
	virtual std::vector <glm::vec2>& GetTexCoords() override; //ALIGNMENT
	virtual std::vector <Mesh::Face>& GetFaces() override;
	virtual std::vector <glm::ivec4> const& GetBoneIndices() const;
	virtual std::vector <glm::vec4> const& GetBoneWeights() const;
	virtual int GetVertexCount() const override;
	virtual int GetFaceCount() const override;


private:

	//Load methods
	///void Load_Vertices(std::vector<glm::vec4> &vertices);
	///void Load_Normals(std::vector<glm::vec4> &normals);
	void Load_TexCoords(std::vector<glm::vec2> &uvs);
	void Load_Faces(std::vector<Mesh::Face> &faces);
	void Load_Tangents_and_Bitangents(std::vector<glm::vec4> &tangents, 
		std::vector<glm::vec4> &bitangents);
	void Load_BoneIndices(std::vector<std::vector<int>> const& indices);
	void Load_BoneWeights(std::vector<std::vector<float>> const& weights);
};