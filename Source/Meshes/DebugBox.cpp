///HEADER FILE

#include "DebugBox.h"
#include <iostream>
#include <cmath>



//////////////////////////////////////////////
////		OUTLINE BOX					 /////
//////////////////////////////////////////////
DebugBox::DebugBox(std::vector<glm::vec4> const& points)
{
	//Vertices
	for (int i = 0; i < points.size(); ++i)
		this->vertices.push_back(points[i]);

	//Bones indices (Empty for primitive)
	for (int i = 0; i < GetVertexCount(); ++i)
		this->boneIndices.push_back(glm::ivec4(-1));

	//Init step
	this->init();
}


DebugBox::~DebugBox()
{
}


int DebugBox::GetVertexCount() const
{
  return vertices.size();
}


std::vector<glm::vec4>& DebugBox::GetVertices()
{
	return vertices;
}


void DebugBox::Draw() const
{
	glDrawArrays(GL_LINE_STRIP, 0, this->GetVertexCount());
}


void DebugBox::DrawInstanced(int count) const
{
	glDrawArraysInstanced(GL_LINE_STRIP, 0, this->GetVertexCount(), count);
}


void DebugBox::init()
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



/////////////////////////////////////////
/////	FILLED BOX					/////
/////////////////////////////////////////
DebugBoxFilled::DebugBoxFilled(glm::vec3 size)
{
	float xVal = size.x * 0.5f;
	float yVal = size.y * 0.5f;
	float zVal = size.z * 0.5f;

	//VERTICES
	//front face
	vertices.push_back(glm::vec4( xVal,  yVal, zVal, 1));
	vertices.push_back(glm::vec4(-xVal, -yVal, zVal, 1));
	vertices.push_back(glm::vec4( xVal, -yVal, zVal, 1));
	vertices.push_back(glm::vec4( xVal,  yVal, zVal, 1));
	vertices.push_back(glm::vec4(-xVal,  yVal, zVal, 1));
	vertices.push_back(glm::vec4(-xVal, -yVal, zVal, 1));
	//back face
	vertices.push_back(glm::vec4(-xVal,  yVal, -zVal, 1));
	vertices.push_back(glm::vec4( xVal,  yVal, -zVal, 1));
	vertices.push_back(glm::vec4( xVal, -yVal, -zVal, 1));
	vertices.push_back(glm::vec4( xVal, -yVal, -zVal, 1));
	vertices.push_back(glm::vec4(-xVal, -yVal, -zVal, 1));
	vertices.push_back(glm::vec4(-xVal,  yVal, -zVal, 1));
	//Left
	vertices.push_back(glm::vec4(-xVal,  yVal, -zVal, 1));
	vertices.push_back(glm::vec4(-xVal, -yVal, -zVal, 1));
	vertices.push_back(glm::vec4(-xVal, -yVal,  zVal, 1));
	vertices.push_back(glm::vec4(-xVal,  yVal,  zVal, 1));
	vertices.push_back(glm::vec4(-xVal,  yVal, -zVal, 1));
	vertices.push_back(glm::vec4(-xVal, -yVal,  zVal, 1));
	//Right
	vertices.push_back(glm::vec4(xVal, -yVal,  zVal, 1));
	vertices.push_back(glm::vec4(xVal, -yVal, -zVal, 1));
	vertices.push_back(glm::vec4(xVal,  yVal, -zVal, 1));
	vertices.push_back(glm::vec4(xVal,  yVal, -zVal, 1));
	vertices.push_back(glm::vec4(xVal,  yVal,  zVal, 1));
	vertices.push_back(glm::vec4(xVal, -yVal,  zVal, 1));
	//Top
	vertices.push_back(glm::vec4( xVal, yVal,  zVal, 1));
	vertices.push_back(glm::vec4( xVal, yVal, -zVal, 1));
	vertices.push_back(glm::vec4(-xVal, yVal,  zVal, 1));
	vertices.push_back(glm::vec4( xVal, yVal, -zVal, 1));
	vertices.push_back(glm::vec4(-xVal, yVal, -zVal, 1));
	vertices.push_back(glm::vec4(-xVal, yVal,  zVal, 1));
	//Bottom
	vertices.push_back(glm::vec4(-xVal, -yVal,  zVal, 1));
	vertices.push_back(glm::vec4(-xVal, -yVal, -zVal, 1));
	vertices.push_back(glm::vec4( xVal, -yVal, -zVal, 1));
	vertices.push_back(glm::vec4( xVal, -yVal,  zVal, 1));
	vertices.push_back(glm::vec4(-xVal, -yVal,  zVal, 1));
	vertices.push_back(glm::vec4( xVal, -yVal, -zVal, 1));


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

	//Bones indices (Empty for primitive)
	for (int i = 0; i < GetVertexCount(); ++i)
		this->boneIndices.push_back(glm::ivec4(-1));

	//Init step
	this->init();
}


DebugBoxFilled::~DebugBoxFilled()
{
}


int DebugBoxFilled::GetVertexCount() const
{
  return vertices.size();
}


std::vector<glm::vec4>& DebugBoxFilled::GetVertices()
{
	return vertices;
}


void DebugBoxFilled::Draw() const
{
	glDrawArrays(GL_TRIANGLES, 0, this->GetVertexCount());
}


void DebugBoxFilled::DrawInstanced(int count) const
{
	glDrawArraysInstanced(GL_TRIANGLES, 0, this->GetVertexCount(), count);
}


void DebugBoxFilled::init()
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

	glBindBuffer(GL_ARRAY_BUFFER, this->vbo[BONE_INDICES]);
	glBufferData(GL_ARRAY_BUFFER, GetVertexCount() * 4 * sizeof(GLint), &boneIndices[0][0], GL_STATIC_DRAW);
	
	glBindBuffer(GL_ARRAY_BUFFER, this->vbo[BONE_WEIGHTS]);
	glBufferData(GL_ARRAY_BUFFER, GetVertexCount() * 4 * sizeof(GLfloat), &boneWeights[0][0], GL_STATIC_DRAW);

	//PASS ATTRIBUTES AND ENABLE
	glBindBuffer(GL_ARRAY_BUFFER, this->vbo[VERTICES]);
	glVertexAttribPointer(0, 4, GL_FLOAT, false, 4 * sizeof(GLfloat), (void*)0);

	glBindBuffer(GL_ARRAY_BUFFER, this->vbo[NORMALS]);
	glVertexAttribPointer(1, 4, GL_FLOAT, false, 4 * sizeof(GLfloat), (void*)0);

	glBindBuffer(GL_ARRAY_BUFFER, this->vbo[BONE_INDICES]);
	glVertexAttribIPointer(3, 4, GL_INT, 4 * sizeof(GLint), (void*)0);
	
	glBindBuffer(GL_ARRAY_BUFFER, this->vbo[BONE_WEIGHTS]);
	glVertexAttribPointer(4, 4, GL_FLOAT, false, 4 * sizeof(GLfloat), (void*)0);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(3);
	glEnableVertexAttribArray(4);

	//Unbind Everything
	glBindVertexArray(0);
}
