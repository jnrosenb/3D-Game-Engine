///HEADER

#pragma once

#include "Mesh.h"


class LoadedMesh : public Mesh 
{
public:
	LoadedMesh( std::vector<glm::vec4> &vertices, std::vector<glm::vec4> &normals, 
		std::vector<glm::vec2> &uvs, std::vector<Mesh::Face> &faces, 
		std::vector<glm::vec4> &tangents, std::vector<glm::vec4> &bitangents);

	virtual ~LoadedMesh();

	//OVERRIDES
	virtual std::vector <glm::vec4>& GetVertices() override;
	virtual std::vector <glm::vec4>& GetNormals() override;
	virtual std::vector <glm::vec2>& GetTexCoords() override; //ALIGNMENT
	virtual std::vector <Mesh::Face>& GetFaces() override;

	virtual int GetVertexCount() const override;
	virtual int GetFaceCount() const override;

private:
	void Load_Vertices(std::vector<glm::vec4> &vertices);
	void Load_Normals(std::vector<glm::vec4> &normals);
	void Load_TexCoords(std::vector<glm::vec2> &uvs);
	void Load_Faces(std::vector<Mesh::Face> &faces);
	//void Load_Tangents_and_Bitangents(std::vector<glm::vec4> &tangents, 
	//	std::vector<glm::vec4> &bitangents);
};