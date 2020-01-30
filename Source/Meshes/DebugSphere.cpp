///HEADER FILE

#include "DebugSphere.h"
#include <iostream>
#include <cmath>

#define PI			3.14159265359f


//////////////////////////////////////////////
////		OUTLINE SPHERE			     /////
//////////////////////////////////////////////
DebugSphereOutline::DebugSphereOutline(int subdivissions)
{
	int M = 2 * subdivissions;
	int N = subdivissions;
	int NORTH = ( M * (N - 1));
	int SOUTH = ( M * (N - 1) + 1);

	vertices.reserve(M * (N - 1) + 2);
	normals.reserve(M * (N - 1) + 2);

	//NORMALS
	for (int i = 1; i < N; ++i)
	{
		float theta = PI * i / N;
		for (int j = 0; j < M; ++j)
		{
			int index = M * (i - 1) + j;
			float phi = 2 * PI * j / M;
			glm::vec3 n = glm::vec3(sin(theta)*cos(phi), cos(theta), sin(theta)*sin(phi));
			n = glm::normalize(n);
			normals.push_back(glm::vec4(n.x, n.y, n.z, 0.0f));
		}
	}
	normals.push_back(glm::vec4(0.0f, 1.0f, 0.0f, 0.0f));
	normals.push_back(glm::vec4(0.0f, -1.0f, 0.0f, 0.0f));

	//VERTICES
	for (unsigned n = 0; n < normals.size(); ++n)
	{
		vertices.push_back(normals[n]);
		vertices[n].w = 1;
	}

	//FACES
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
		face[0] = jp1;
		face[1] = j;
		face[2] = NORTH;
		faces.push_back(face);
		face[0] = M * (N - 2) + j;
		face[1] = M * (N - 2) + jp1;
		face[2] = SOUTH;
		faces.push_back(face);
	}

	//Bones indices (Empty for primitive)
	for (int i = 0; i < GetVertexCount(); ++i)
		this->boneIndices.push_back(glm::ivec4(-1));

	//Init step
	this->init();
}


DebugSphereOutline::~DebugSphereOutline()
{
}


int DebugSphereOutline::GetVertexCount() const
{
  return vertices.size();
}


std::vector<glm::vec4>& DebugSphereOutline::GetVertices()
{
	return vertices;
}


void DebugSphereOutline::Draw() const
{
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	int size = GetFaceCount();
	glDrawElements(GL_TRIANGLES, 3 * size, GL_UNSIGNED_INT, (void*)0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}


void DebugSphereOutline::DrawInstanced(int count) const
{
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	int size = GetFaceCount();
	glDrawElementsInstanced(GL_TRIANGLES, 3 * size, GL_UNSIGNED_INT, (void*)0, count);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}


void DebugSphereOutline::init()
{
	//Generate VAO, VBO and EBO
	glGenVertexArrays(1, &this->vao);
	glGenBuffers(vbo_index::NUM, this->vbo);
	glGenBuffers(1, &this->ebo);

	//Bins the VAO
	glBindVertexArray(this->vao);

	//Allocate in gpu
	glBindBuffer(GL_ARRAY_BUFFER, this->vbo[VERTICES]);
	glBufferData(GL_ARRAY_BUFFER, GetVertexCount() * 4 * sizeof(GLfloat), &vertices[0][0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, this->vbo[NORMALS]);
	glBufferData(GL_ARRAY_BUFFER, GetVertexCount() * 4 * sizeof(GLfloat), &normals[0][0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, this->vbo[BONE_INDICES]);
	glBufferData(GL_ARRAY_BUFFER, GetVertexCount() * 4 * sizeof(GLint), &boneIndices[0][0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->GetFaceCount() * sizeof(Mesh::Face), &faces[0][0], GL_STATIC_DRAW);

	//PASS ATTRIBUTES AND ENABLE
	glBindBuffer(GL_ARRAY_BUFFER, this->vbo[VERTICES]);
	glVertexAttribPointer(0, 4, GL_FLOAT, false, 4 * sizeof(GLfloat), (void*)0);

	glBindBuffer(GL_ARRAY_BUFFER, this->vbo[NORMALS]);
	glVertexAttribPointer(1, 4, GL_FLOAT, false, 4 * sizeof(GLfloat), (void*)0);

	glBindBuffer(GL_ARRAY_BUFFER, this->vbo[BONE_INDICES]);
	glVertexAttribIPointer(3, 4, GL_INT, 4 * sizeof(GLint), (void*)0);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(3);

	//Unbind Everything
	glBindVertexArray(0);
}



/////////////////////////////////////////
/////	FILLED SPHERE				/////
/////////////////////////////////////////
DebugSphereFilled::DebugSphereFilled(int subdivissions)
{
	int M = 2 * subdivissions;
	int N = subdivissions;
	int NORTH = (M * (N - 1));
	int SOUTH = (M * (N - 1) + 1);

	vertices.reserve(M * (N - 1) + 2);
	normals.reserve(M * (N - 1) + 2);

	//NORMALS
	for (int i = 1; i < N; ++i)
	{
		float theta = PI * i / N;
		for (int j = 0; j < M; ++j)
		{
			int index = M * (i - 1) + j;
			float phi = 2 * PI * j / M;
			glm::vec3 n = glm::vec3(sin(theta)*cos(phi), cos(theta), sin(theta)*sin(phi));
			n = glm::normalize(n);
			normals.push_back(glm::vec4(n.x, n.y, n.z, 0.0f));
		}
	}
	normals.push_back(glm::vec4(0.0f, 1.0f, 0.0f, 0.0f));
	normals.push_back(glm::vec4(0.0f, -1.0f, 0.0f, 0.0f));

	//VERTICES
	for (unsigned n = 0; n < normals.size(); ++n)
	{
		vertices.push_back(normals[n]);
		vertices[n].w = 1;
	}

	//FACES
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
		face[0] = jp1;
		face[1] = j;
		face[2] = NORTH;
		faces.push_back(face);
		face[0] = M * (N - 2) + j;
		face[1] = M * (N - 2) + jp1;
		face[2] = SOUTH;
		faces.push_back(face);
	}

	//Bones indices (Empty for primitive)
	for (int i = 0; i < GetVertexCount(); ++i)
		this->boneIndices.push_back(glm::ivec4(-1));

	//Init step
	this->init();
}


DebugSphereFilled::~DebugSphereFilled()
{
}


int DebugSphereFilled::GetVertexCount() const
{
  return vertices.size();
}


std::vector<glm::vec4>& DebugSphereFilled::GetVertices()
{
	return vertices;
}


void DebugSphereFilled::Draw() const
{
	glDrawElements(GL_TRIANGLES, 3 * GetFaceCount(), GL_UNSIGNED_INT, (void*)0);
}


void DebugSphereFilled::DrawInstanced(int count) const
{
	glDrawElementsInstanced(GL_TRIANGLES, 3 * GetFaceCount(), GL_UNSIGNED_INT, (void*)0, count);
}


void DebugSphereFilled::init()
{
	//Generate VAO, VBO and EBO
	glGenVertexArrays(1, &this->vao);
	glGenBuffers(vbo_index::NUM, this->vbo);
	glGenBuffers(1, &this->ebo);

	//Bins the VAO
	glBindVertexArray(this->vao);

	//Allocate in gpu
	glBindBuffer(GL_ARRAY_BUFFER, this->vbo[VERTICES]);
	glBufferData(GL_ARRAY_BUFFER, GetVertexCount() * 4 * sizeof(GLfloat), &vertices[0][0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, this->vbo[NORMALS]);
	glBufferData(GL_ARRAY_BUFFER, GetVertexCount() * 4 * sizeof(GLfloat), &normals[0][0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, this->vbo[BONE_INDICES]);
	glBufferData(GL_ARRAY_BUFFER, GetVertexCount() * 4 * sizeof(GLint), &boneIndices[0][0], GL_STATIC_DRAW);
	
	glBindBuffer(GL_ARRAY_BUFFER, this->vbo[BONE_WEIGHTS]);
	glBufferData(GL_ARRAY_BUFFER, GetVertexCount() * 4 * sizeof(GLfloat), &boneWeights[0][0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->GetFaceCount() * sizeof(Mesh::Face), &faces[0][0], GL_STATIC_DRAW);

	//PASS ATTRIBUTES AND ENABLE
	glBindBuffer(GL_ARRAY_BUFFER, this->vbo[VERTICES]);
	glVertexAttribPointer(0, 4, GL_FLOAT, false, 4 * sizeof(GLfloat), (void*)0);

	glBindBuffer(GL_ARRAY_BUFFER, this->vbo[NORMALS]);
	glVertexAttribPointer(1, 4, GL_FLOAT, false, 4 * sizeof(GLfloat), (void*)0);

	glBindBuffer(GL_ARRAY_BUFFER, this->vbo[BONE_INDICES]);
	glVertexAttribIPointer(3, 4, GL_INT, 4 * sizeof(GLint), (void*)0);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(3);

	//Unbind Everything
	glBindVertexArray(0);
}
