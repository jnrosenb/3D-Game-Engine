#include "Plane.h"
#include <iostream>
#include "Affine.h"

#define PI 3.14159265359


Plane::Plane(int mesh_size) :
	Mesh()
{
	//Not the most ideal, but will do for now
	vertices.resize((mesh_size + 1)*(mesh_size + 1));
	texCoords.resize(vertices.size());
	faces.resize(2 * mesh_size * mesh_size);
	
	std::cout << "Plane Ctor" << std::endl;

	//Vertices initial setting (NDC)
	float d = 1.0f / mesh_size;
	for (int j = 0; j <= mesh_size; ++j)
	{
		float y = j * d - 1.0f;
		for (int i = 0; i <= mesh_size; ++i)
		{
			float x = i * d - 1.0f;
			int index = (mesh_size + 1)*j + i;
			vertices[index] = glm::vec4(x, y, 0.0f, 1.0f);
		}
	}

	//Normals setup
	for (int i = 0; i < vertices.size(); ++i) 
	{
		normals.push_back(glm::vec4(0.0f, 0.0f, 1.0f, 0.0f));
	}

	//Faces setup
	for (int n = 0, j = 0; j < mesh_size; ++j)
	{		
		for (int i = 0; i < mesh_size; ++i)
		{
			faces[n][0] = (mesh_size + 1)*j + i;
			faces[n][1] = (mesh_size + 1)*j + i + 1;
			faces[n][2] = (mesh_size + 1)*(j + 1) + i + 1;
			++n;
			faces[n][0] = (mesh_size + 1)*j + i;
			faces[n][1] = (mesh_size + 1)*(j + 1) + i + 1;
			faces[n][2] = (mesh_size + 1)*(j + 1) + i;
			++n;
		}
	}
	
	//Set UV coords
	int vertexNum = vertices.size();
	for (int i = 0; i < GetVertexCount(); ++i)
	{
		glm::vec4 uv = 0.5f * (vertices[i] + glm::vec4(1.0f));
		texCoords[i].x = uv[0];
		texCoords[i].y = uv[1];
	}

	//Set all bone indices to -1
	Set_BoneIndices();

	//Init opengl buffers
	this->init();
}

Plane::~Plane()
{
}

std::vector<glm::vec4>& Plane::GetVertices()
{
	return this->vertices;
}


std::vector<glm::vec4>& Plane::GetNormals()
{
	return this->normals;
}


std::vector<glm::vec2>& Plane::GetTexCoords()
{
	return this->texCoords;
}


std::vector<Mesh::Face>& Plane::GetFaces()
{
	return this->faces;
}


int Plane::GetVertexCount() const
{
	return this->vertices.size();
}


int Plane::GetFaceCount() const
{
	return this->faces.size();
}

void Plane::Set_BoneIndices()
{
	for (int i = 0; i < GetVertexCount(); ++i)
	{
		glm::ivec4 ind = glm::ivec4(-1, -1, -1, -1);
		this->boneIndices.push_back(ind);
	}
}