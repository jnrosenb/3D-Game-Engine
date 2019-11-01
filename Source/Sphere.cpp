///HEADER FILE

#include <iostream>
#include <cmath>
#include "Sphere.h"

///GLOBAL
const float PI = 4.0f * atan(1.0f);

///MACROS
#define M (2 * size)
#define N (size)
#define NORTH ( M * (N - 1))
#define SOUTH ( M * (N - 1) + 1)


//CTOR
Sphere::Sphere(int size)
{
	vertices.reserve(M * (N - 1) + 2);
	normals.reserve(M * (N - 1) + 2);

	for (int i = 1; i < N; ++i) 
	{
		float theta = PI * i / N;
		for (int j = 0; j < M; ++j) 
		{
			int index = M * (i - 1) + j;
			float phi = 2 * PI*j / M;
			normals.push_back( glm::vec4(
				sin(theta)*cos(phi), 
				sin(theta)*sin(phi), 
				cos(theta), 
				0));
		}
	}
	normals[NORTH] = glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);
	normals[SOUTH] = glm::vec4(0.0f, 0.0f, -1.0f, 0.0f);

	for (unsigned n = 0; n < normals.size(); ++n) 
	{
		vertices.push_back( normals[n] );
		vertices[n].w = 1;
	}

	for (int i = 2; i < N; ++i) 
	{
		for (int j = 0; j < M; ++j) 
		{
			Face face;
			int jp1 = (j + 1) % M;
			face[0] = M * (i - 2) + j;
			face[1] = M * (i - 2) + jp1;
			face[2] = M * (i - 1) + jp1;
			faces.push_back(face);
			face[1] = face[2];
			face[2] = M * (i - 1) + j;
			faces.push_back(face);
		}
	}

	for (int j = 0; j < M; ++j) 
	{
		Face face;
		int jp1 = (j + 1) % M;
		face[0] = j;
		face[1] = jp1;
		face[2] = NORTH;
		faces.push_back(face);
		face[0] = M * (N - 2) + j;
		face[1] = M * (N - 2) + jp1;
		face[2] = SOUTH;
		faces.push_back(face);
	}

	//Bones indices
	this->boneIndices.push_back(glm::ivec4(-1));
	this->boneIndices.push_back(glm::ivec4(-1));
	this->boneIndices.push_back(glm::ivec4(-1));
	this->boneIndices.push_back(glm::ivec4(-1));

	this->init();
}

#undef SOUTH
#undef NORTH
#undef N
#undef M

Sphere::~Sphere()
{
}


int Sphere::GetVertexCount() const
{
  return vertices.size();
}


std::vector<glm::vec4>& Sphere::GetVertices() 
{
	return vertices;
}


std::vector<glm::vec4>& Sphere::GetNormals()
{
	return normals;
}



std::vector <glm::vec2>& Sphere::GetTexCoords() //Alignment?
{
	return texCoords;
}


std::vector<Mesh::Face>& Sphere::GetFaces()
{
	return faces;
}


int Sphere::GetFaceCount() const
{
	return faces.size();
}

