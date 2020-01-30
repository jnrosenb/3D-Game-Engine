///HEADER FILE

#include "DebugRay.h"
#include <iostream>
#include <cmath>



//////////////////////////////////////////////
////		OUTLINE BOX					 /////
//////////////////////////////////////////////
DebugRay::DebugRay()
{
	this->vertices.push_back(glm::vec4(0, 0, 0, 1));
	this->vertices.push_back(glm::vec4(1, 0, 0, 1));

	//Bones indices (Empty for primitive)
	for (int i = 0; i < GetVertexCount(); ++i)
		this->boneIndices.push_back(glm::ivec4(-1));

	//Init step
	this->init();
}


DebugRay::~DebugRay()
{
}


int DebugRay::GetVertexCount() const
{
  return vertices.size();
}


std::vector<glm::vec4>& DebugRay::GetVertices()
{
	return vertices;
}


void DebugRay::Draw() const
{
	glDrawArrays(GL_LINES, 0, this->GetVertexCount());
}


void DebugRay::DrawInstanced(int count) const
{
	glDrawArraysInstanced(GL_LINES, 0, this->GetVertexCount(), count);
}


void DebugRay::init()
{
	//Generate VAO, VBO and EBO
	glGenVertexArrays(1, &this->vao);
	glGenBuffers(vbo_index::NUM, this->vbo);

	//Bins the VAO
	glBindVertexArray(this->vao);

	//Allocate in gpu
	glBindBuffer(GL_ARRAY_BUFFER, this->vbo[VERTICES]);
	glBufferData(GL_ARRAY_BUFFER, GetVertexCount() * 4 * sizeof(GLfloat), &vertices[0][0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, this->vbo[BONE_INDICES]);
	glBufferData(GL_ARRAY_BUFFER, GetVertexCount() * 4 * sizeof(GLint), &boneIndices[0][0], GL_STATIC_DRAW);
	
	glBindBuffer(GL_ARRAY_BUFFER, this->vbo[BONE_WEIGHTS]);
	glBufferData(GL_ARRAY_BUFFER, GetVertexCount() * 4 * sizeof(GLfloat), &boneWeights[0][0], GL_STATIC_DRAW);

	//PASS ATTRIBUTES AND ENABLE
	glBindBuffer(GL_ARRAY_BUFFER, this->vbo[VERTICES]);
	glVertexAttribPointer(0, 4, GL_FLOAT, false, 4 * sizeof(GLfloat), (void*)0);

	glBindBuffer(GL_ARRAY_BUFFER, this->vbo[BONE_INDICES]);
	glVertexAttribIPointer(3, 4, GL_INT, 4 * sizeof(GLint), (void*)0);
	
	glBindBuffer(GL_ARRAY_BUFFER, this->vbo[BONE_WEIGHTS]);
	glVertexAttribPointer(4, 4, GL_FLOAT, false, 4 * sizeof(GLfloat), (void*)0);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(3);
	glEnableVertexAttribArray(4);

	//Unbind Everything
	glBindVertexArray(0);
}