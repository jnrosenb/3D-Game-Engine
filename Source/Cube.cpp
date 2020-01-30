///HEADER FILE

#include "Cube.h"
#include <iostream>
#include <cmath>


//CTOR
Cube::Cube()
{
	//VERTICES
	//front face
	vertices.push_back(glm::vec4(0.5f, 0.5f, 0.5f, 1));
	vertices.push_back(glm::vec4(-0.5f, -0.5f, 0.5f, 1));
	vertices.push_back(glm::vec4(0.5f, -0.5f, 0.5f, 1));
	vertices.push_back(glm::vec4(0.5f, 0.5f, 0.5f, 1));
	vertices.push_back(glm::vec4(-0.5f, 0.5f, 0.5f, 1));
	vertices.push_back(glm::vec4(-0.5f, -0.5f, 0.5f, 1));
	//back face
	vertices.push_back(glm::vec4(-0.5f, 0.5f, -0.5f, 1));
	vertices.push_back(glm::vec4(0.5f, 0.5f, -0.5f, 1));
	vertices.push_back(glm::vec4(0.5f, -0.5f, -0.5f, 1));
	vertices.push_back(glm::vec4(0.5f, -0.5f, -0.5f, 1));
	vertices.push_back(glm::vec4(-0.5f, -0.5f, -0.5f, 1));
	vertices.push_back(glm::vec4(-0.5f, 0.5f, -0.5f, 1));
	//Left
	vertices.push_back(glm::vec4(-0.5f, 0.5f, -0.5f, 1));
	vertices.push_back(glm::vec4(-0.5f, -0.5f, -0.5f, 1));
	vertices.push_back(glm::vec4(-0.5f, -0.5f, 0.5f, 1));
	vertices.push_back(glm::vec4(-0.5f, 0.5f, 0.5f, 1));
	vertices.push_back(glm::vec4(-0.5f, 0.5f, -0.5f, 1));
	vertices.push_back(glm::vec4(-0.5f, -0.5f, 0.5f, 1));
	//Right
	vertices.push_back(glm::vec4(0.5f, -0.5f, 0.5f, 1));
	vertices.push_back(glm::vec4(0.5f, -0.5f, -0.5f, 1));
	vertices.push_back(glm::vec4(0.5f, 0.5f, -0.5f, 1));
	vertices.push_back(glm::vec4(0.5f, 0.5f, -0.5f, 1));
	vertices.push_back(glm::vec4(0.5f, 0.5f, 0.5f, 1));
	vertices.push_back(glm::vec4(0.5f, -0.5f, 0.5f, 1));
	//Top
	vertices.push_back(glm::vec4(0.5f, 0.5f, 0.5f, 1));
	vertices.push_back(glm::vec4(0.5f, 0.5f, -0.5f, 1));
	vertices.push_back(glm::vec4(-0.5f, 0.5f, 0.5f, 1));
	vertices.push_back(glm::vec4(0.5f, 0.5f, -0.5f, 1));
	vertices.push_back(glm::vec4(-0.5f, 0.5f, -0.5f, 1));
	vertices.push_back(glm::vec4(-0.5f, 0.5f, 0.5f, 1));
	//Bottom
	vertices.push_back(glm::vec4(-0.5f, -0.5f, 0.5f, 1));
	vertices.push_back(glm::vec4(-0.5f, -0.5f, -0.5f, 1));
	vertices.push_back(glm::vec4(0.5f, -0.5f, -0.5f, 1));
	vertices.push_back(glm::vec4(0.5f, -0.5f, 0.5f, 1));
	vertices.push_back(glm::vec4(-0.5f, -0.5f, 0.5f, 1));
	vertices.push_back(glm::vec4(0.5f, -0.5f, -0.5f, 1));


	//Normals
	//front face
	normals.push_back(glm::vec4(0, 0, 1, 0));
	normals.push_back(glm::vec4(0, 0, 1, 0));
	normals.push_back(glm::vec4(0, 0, 1, 0));
	normals.push_back(glm::vec4(0, 0, 1, 0));
	normals.push_back(glm::vec4(0, 0, 1, 0));
	normals.push_back(glm::vec4(0, 0, 1, 0));
	//Back face
	normals.push_back(glm::vec4(0, 0, -1, 0));
	normals.push_back(glm::vec4(0, 0, -1, 0));
	normals.push_back(glm::vec4(0, 0, -1, 0));
	normals.push_back(glm::vec4(0, 0, -1, 0));
	normals.push_back(glm::vec4(0, 0, -1, 0));
	normals.push_back(glm::vec4(0, 0, -1, 0));
	//Left
	normals.push_back(glm::vec4(-1, 0, 0, 0));
	normals.push_back(glm::vec4(-1, 0, 0, 0));
	normals.push_back(glm::vec4(-1, 0, 0, 0));
	normals.push_back(glm::vec4(-1, 0, 0, 0));
	normals.push_back(glm::vec4(-1, 0, 0, 0));
	normals.push_back(glm::vec4(-1, 0, 0, 0));
	//Right
	normals.push_back(glm::vec4(1, 0, 0, 0));
	normals.push_back(glm::vec4(1, 0, 0, 0));
	normals.push_back(glm::vec4(1, 0, 0, 0));
	normals.push_back(glm::vec4(1, 0, 0, 0));
	normals.push_back(glm::vec4(1, 0, 0, 0));
	normals.push_back(glm::vec4(1, 0, 0, 0));
	//Top
	normals.push_back(glm::vec4(0, 1, 0, 0));
	normals.push_back(glm::vec4(0, 1, 0, 0));
	normals.push_back(glm::vec4(0, 1, 0, 0));
	normals.push_back(glm::vec4(0, 1, 0, 0));
	normals.push_back(glm::vec4(0, 1, 0, 0));
	normals.push_back(glm::vec4(0, 1, 0, 0));
	//Bottom
	normals.push_back(glm::vec4(0, -1, 0, 0));
	normals.push_back(glm::vec4(0, -1, 0, 0));
	normals.push_back(glm::vec4(0, -1, 0, 0));
	normals.push_back(glm::vec4(0, -1, 0, 0));
	normals.push_back(glm::vec4(0, -1, 0, 0));
	normals.push_back(glm::vec4(0, -1, 0, 0));


	//Uvs
	//front
	texCoords.push_back(glm::vec2(1, 1));
	texCoords.push_back(glm::vec2(0, 0));
	texCoords.push_back(glm::vec2(1, 0));
	texCoords.push_back(glm::vec2(1, 1));
	texCoords.push_back(glm::vec2(0, 1));
	texCoords.push_back(glm::vec2(0, 0));
	//back
	texCoords.push_back(glm::vec2(0, 1));
	texCoords.push_back(glm::vec2(1, 1));
	texCoords.push_back(glm::vec2(1, 0));
	texCoords.push_back(glm::vec2(1, 0));
	texCoords.push_back(glm::vec2(0, 0));
	texCoords.push_back(glm::vec2(0, 1));
	//left
	texCoords.push_back(glm::vec2(1, 1));
	texCoords.push_back(glm::vec2(0, 0));
	texCoords.push_back(glm::vec2(1, 0));
	texCoords.push_back(glm::vec2(1, 1));
	texCoords.push_back(glm::vec2(0, 1));
	texCoords.push_back(glm::vec2(0, 0));
	//right
	texCoords.push_back(glm::vec2(0, 0));
	texCoords.push_back(glm::vec2(1, 0));
	texCoords.push_back(glm::vec2(1, 1));
	texCoords.push_back(glm::vec2(1, 1));
	texCoords.push_back(glm::vec2(0, 1));
	texCoords.push_back(glm::vec2(0, 0));
	//top
	texCoords.push_back(glm::vec2(1, 0));
	texCoords.push_back(glm::vec2(1, 1));
	texCoords.push_back(glm::vec2(0, 0));
	texCoords.push_back(glm::vec2(1, 1));
	texCoords.push_back(glm::vec2(0, 1));
	texCoords.push_back(glm::vec2(0, 0));
	//bottom
	texCoords.push_back(glm::vec2(1, 1));
	texCoords.push_back(glm::vec2(0, 0));
	texCoords.push_back(glm::vec2(1, 0));
	texCoords.push_back(glm::vec2(1, 1));
	texCoords.push_back(glm::vec2(0, 1));
	texCoords.push_back(glm::vec2(0, 0));

	//Bones indices (Empty for primitive)
	for (int i = 0; i < 36; ++i)
		this->boneIndices.push_back(glm::ivec4(-1));

	//Init step
	this->init();
}


Cube::~Cube()
{
}


int Cube::GetVertexCount() const
{
  return vertices.size();
}


std::vector<glm::vec4>& Cube::GetVertices()
{
	return vertices;
}


std::vector<glm::vec4>& Cube::GetNormals()
{
	return normals;
}



std::vector <glm::vec2>& Cube::GetTexCoords() //Alignment?
{
	return texCoords;
}


std::vector<Mesh::Face>& Cube::GetFaces()
{
	return faces;
}


int Cube::GetFaceCount() const
{
	return faces.size();
}


void Cube::Draw() const
{
	glDrawArrays(GL_TRIANGLES, 0, this->GetVertexCount());
}


void Cube::DrawInstanced(int count) const
{
	glDrawArraysInstanced(GL_TRIANGLES, 0, this->GetVertexCount(), count);
}


void Cube::init()
{
	//Generate VAO, VBO and EBO
	glGenVertexArrays(1, &this->vao);
	glGenBuffers(vbo_index::NUM, this->vbo);

	//Bins the VAO
	glBindVertexArray(this->vao);

	//Allocate in gpu
	glBindBuffer(GL_ARRAY_BUFFER, this->vbo[VERTICES]);
	glBufferData(GL_ARRAY_BUFFER, GetVertexCount() * 4 * sizeof(GLfloat), &vertices[0][0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, this->vbo[NORMALS]);
	glBufferData(GL_ARRAY_BUFFER, GetVertexCount() * 4 * sizeof(GLfloat), &normals[0][0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, this->vbo[TEXCOORDS]);
	glBufferData(GL_ARRAY_BUFFER, GetVertexCount() * 2 * sizeof(GLfloat), &texCoords[0][0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, this->vbo[TANGENTS]);
	glBufferData(GL_ARRAY_BUFFER, GetVertexCount() * 4 * sizeof(GLfloat), &m_tangents[0][0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, this->vbo[BITANGENTS]);
	glBufferData(GL_ARRAY_BUFFER, GetVertexCount() * 4 * sizeof(GLfloat), &m_bitangents[0][0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, this->vbo[BONE_INDICES]);
	glBufferData(GL_ARRAY_BUFFER, GetVertexCount() * 4 * sizeof(GLint), &boneIndices[0][0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, this->vbo[BONE_WEIGHTS]);
	glBufferData(GL_ARRAY_BUFFER, GetVertexCount() * 4 * sizeof(GLfloat), &boneWeights[0][0], GL_STATIC_DRAW);

	//PASS ATTRIBUTES AND ENABLE
	glBindBuffer(GL_ARRAY_BUFFER, this->vbo[VERTICES]);
	glVertexAttribPointer(0, 4, GL_FLOAT, false, 4 * sizeof(GLfloat), (void*)0);

	glBindBuffer(GL_ARRAY_BUFFER, this->vbo[NORMALS]);
	glVertexAttribPointer(1, 4, GL_FLOAT, false, 4 * sizeof(GLfloat), (void*)0);

	glBindBuffer(GL_ARRAY_BUFFER, this->vbo[TEXCOORDS]);
	glVertexAttribPointer(2, 2, GL_FLOAT, false, 2 * sizeof(GLfloat), (void*)0);

	glBindBuffer(GL_ARRAY_BUFFER, this->vbo[BONE_INDICES]);
	glVertexAttribIPointer(3, 4, GL_INT, 4 * sizeof(GLint), (void*)0);

	glBindBuffer(GL_ARRAY_BUFFER, this->vbo[BONE_WEIGHTS]);
	glVertexAttribPointer(4, 4, GL_FLOAT, false, 4 * sizeof(GLfloat), (void*)0);

	glBindBuffer(GL_ARRAY_BUFFER, this->vbo[TANGENTS]);
	glVertexAttribPointer(5, 4, GL_FLOAT, false, 4 * sizeof(GLfloat), (void*)0);

	glBindBuffer(GL_ARRAY_BUFFER, this->vbo[BITANGENTS]);
	glVertexAttribPointer(6, 4, GL_FLOAT, false, 4 * sizeof(GLfloat), (void*)0);


	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);
	glEnableVertexAttribArray(4);
	glEnableVertexAttribArray(5);
	glEnableVertexAttribArray(6);

	//Unbind Everything
	glBindVertexArray(0);
}
