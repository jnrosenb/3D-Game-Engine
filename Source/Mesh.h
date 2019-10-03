///HEADER STUFF

#pragma once

///INCLUDES
///#include "../External/Includes/GL/glew.h"
#include "../External/Includes/Glad_/Glad/glad.h"
#include <gl/GL.h>
#include "../External/Includes/glm/glm.hpp"
#include <vector>


enum vbo_index
{
	VERTICES = 0,
	NORMALS,
	TEXCOORDS,
	TANGENTS,
	BITANGENTS,
	BONE_INDICES,
	BONE_WEIGHTS,
	NUM
};


//RENDERER CLASS
class Mesh 
{

//PUBLIC INTERFACE
public:
	Mesh() {}
	virtual ~Mesh() {}
	
	//Struct representing a polygon (alligned 16 bytes)
	struct Face
	{
		unsigned index[3];
		//TODO does it need padding here?

		unsigned operator[](int i) const { return index[i]; }
		unsigned& operator[](int i) { return index[i]; }
	};

	virtual std::vector <glm::vec4>& GetVertices() = 0;
	virtual std::vector <glm::vec4>& GetNormals() = 0;
	virtual std::vector <glm::vec2>& GetTexCoords() = 0; //Alignment?
	virtual std::vector <Mesh::Face>& GetFaces() = 0;

	virtual int GetVertexCount() const = 0;
	virtual int GetFaceCount() const = 0;

	virtual void BindForDraw() 
	{
		glBindVertexArray(this->vao);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ebo); //For now, since it seems not to bind otherwise
	}

	virtual void UnbindForDraw() const 
	{
		glBindVertexArray(0);
	}

//PRIVATE METHODS
protected:

	virtual void init()
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

		glBindBuffer(GL_ARRAY_BUFFER, this->vbo[TEXCOORDS]);
		glBufferData(GL_ARRAY_BUFFER, GetVertexCount() * 2 * sizeof(GLfloat), &texCoords[0][0], GL_STATIC_DRAW);

		//BONE STUFF-----------------
		glBindBuffer(GL_ARRAY_BUFFER, this->vbo[BONE_INDICES]);
		glBufferData(GL_ARRAY_BUFFER, GetVertexCount() * 4 * sizeof(GLint), &boneIndices[0][0], GL_STATIC_DRAW);

		//BONE STUFF-----------------
		glBindBuffer(GL_ARRAY_BUFFER, this->vbo[BONE_WEIGHTS]);
		glBufferData(GL_ARRAY_BUFFER, GetVertexCount() * 4 * sizeof(GLfloat), &boneWeights[0][0], GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->GetFaceCount() * sizeof(Mesh::Face), &faces[0][0], GL_STATIC_DRAW);

		//PASS ATTRIBUTES AND ENABLE
		glBindBuffer(GL_ARRAY_BUFFER, this->vbo[VERTICES]);
		glVertexAttribPointer(0, 4, GL_FLOAT, false, 4 * sizeof(GLfloat), (void*)0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, this->vbo[NORMALS]);
		glVertexAttribPointer(1, 4, GL_FLOAT, false, 4 * sizeof(GLfloat), (void*)0);
		glEnableVertexAttribArray(1);

		glBindBuffer(GL_ARRAY_BUFFER, this->vbo[TEXCOORDS]);
		glVertexAttribPointer(2, 2, GL_FLOAT, false, 2 * sizeof(GLfloat), (void*)0);
		glEnableVertexAttribArray(2);

		//BONE STUFF----------------- Use location 3 and 4 for bones
		glBindBuffer(GL_ARRAY_BUFFER, this->vbo[BONE_INDICES]);
		glVertexAttribIPointer(3, 4, GL_INT, 4 * sizeof(GLint), (void*)0);
		glEnableVertexAttribArray(3);

		glBindBuffer(GL_ARRAY_BUFFER, this->vbo[BONE_WEIGHTS]);
		glVertexAttribPointer(4, 4, GL_FLOAT, false, 4 * sizeof(GLfloat), (void*)0);
		glEnableVertexAttribArray(4);

		//Unbind Everything
		glBindVertexArray(0);
	}

//VARIABLES
protected:
	//Arrays of stored data
	std::vector <glm::vec4> vertices;
	std::vector <glm::vec4> normals;
	std::vector <glm::vec2> texCoords; //Alignment?
	std::vector <Mesh::Face> faces;

	//Skeletal info
	std::vector <glm::ivec4> boneIndices;
	std::vector <glm::vec4> boneWeights;

	//VAO, VBO, EBO
	GLuint vao;
	GLuint vbo[vbo_index::NUM];
	GLuint ebo;
};