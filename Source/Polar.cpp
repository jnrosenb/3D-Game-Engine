#include "Polar.h"
#include <iostream>
#include "Affine.h"

#define PI 3.14159265359


PolarPlane::PolarPlane(int mesh_size) : 
	Mesh()
{
	//Not the most ideal, but will do for now
	vertices.resize((mesh_size + 1)*(mesh_size + 1));
	normals.resize(vertices.size());
	texCoords.resize(vertices.size());
	faces.resize(2 * mesh_size * mesh_size);
	
	std::cout << "PolarPlane Ctor" << std::endl;

	//Vertices initial setting (NDC)
	float d = 2.0f / mesh_size;
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
	//glm::mat4 Std2Unit = AuxMath::scale(0.5f, 0.5f, 1) * AuxMath::translate(glm::vec4(1, 1, 0, 0));
	for (int i = 0; i < GetVertexCount(); ++i)
	{
		//glm::vec4 uv = Std2Unit * vertices[i];
		glm::vec4 uv = 0.5f * (vertices[i] + 1.0f);
		texCoords[i].x = uv[0];
		texCoords[i].y = uv[1];
	}

	//Transform vertices to polar rectangle
	glm::mat4 Std2Plane = AuxMath::scale(PI, PI / 2.0f, 1) * AuxMath::translate(glm::vec4(1, 1, 0, 0)) * AuxMath::scale(1, -1, 1);
	for (int i = 0; i < vertexNum; ++i)
	{
		glm::vec4 tempVertex = Std2Plane * vertices[i];
		glm::vec4 newVertex;
		newVertex.x = sin(tempVertex.y) * cos(tempVertex.x);
		newVertex.y = cos(tempVertex.y);
		newVertex.z = sin(tempVertex.y) * sin(tempVertex.x);
		newVertex.w = 1.0f;
		vertices[i] = newVertex;
	}

	//Transform normals
	for (int i = 0; i < vertexNum; ++i)
	{
		glm::vec4 newNormals = glm::normalize(vertices[i]);
		normals[i] = newNormals;
	}

	//Set all bone indices to -1
	Set_BoneIndices();

	//Init opengl buffers
	this->init();
}

PolarPlane::~PolarPlane()
{
}

std::vector<glm::vec4>& PolarPlane::GetVertices()
{
	return this->vertices;
}


std::vector<glm::vec4>& PolarPlane::GetNormals()
{
	return this->normals;
}


std::vector<glm::vec2>& PolarPlane::GetTexCoords()
{
	return this->texCoords;
}


std::vector<Mesh::Face>& PolarPlane::GetFaces()
{
	return this->faces;
}


int PolarPlane::GetVertexCount() const
{
	return this->vertices.size();
}


int PolarPlane::GetFaceCount() const
{
	return this->faces.size();
}

void PolarPlane::Set_BoneIndices()
{
	for (int i = 0; i < GetVertexCount(); ++i)
	{
		glm::ivec4 ind = glm::ivec4(-1, -1, -1, -1);
		this->boneIndices.push_back(ind);
	}
}