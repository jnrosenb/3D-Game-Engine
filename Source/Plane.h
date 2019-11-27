//HEADER

#pragma once

#include "Mesh.h"
#include <vector>


class Plane : public Mesh 
{
public:
	Plane(int mesh_size);
	virtual ~Plane();

	virtual std::vector <glm::vec4>& GetVertices();
	virtual std::vector <glm::vec4>& GetNormals();
	virtual std::vector <glm::vec2>& GetTexCoords();
	virtual std::vector <Mesh::Face>& GetFaces();

	virtual int GetVertexCount() const;
	virtual int GetFaceCount() const;

private:
	void Set_BoneIndices();
};