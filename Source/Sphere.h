///HEADER STUFF

#pragma once

#include <vector>
#include "Mesh.h"


class Sphere : public Mesh
{

public:
    Sphere(int mesh_size);
	virtual ~Sphere();

	virtual std::vector <glm::vec4>& GetVertices();
	virtual std::vector <glm::vec4>& GetNormals();
	virtual std::vector <glm::vec2>& GetTexCoords(); //Alignment?
	virtual std::vector <Mesh::Face>& GetFaces();

	virtual int GetVertexCount() const;
	virtual int GetFaceCount() const;
};

