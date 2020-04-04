///HEADER STUFF

#pragma once

///INCLUDES
#include "../Mesh.h"


//RENDERER CLASS
class DynamicGridMesh : public Mesh
{

//PUBLIC INTERFACE
public:
	
	DynamicGridMesh() {}


	//This ctor is being used just to easily create a custom mesh with vertices and normals only info
	DynamicGridMesh(std::vector<glm::vec4> const& vertices, 
		std::vector<glm::vec4> const& normals) 
	{
		//This is gonna be passed in the future
		int width = (int)std::sqrt(vertices.size());
		int height = width;

		Load_Vertices(vertices);
		Load_Normals(normals);

		//uv stuff
		float xstep = 1.0f / width;
		float ystep = 1.0f / height;
		for (int i = 0; i < height; ++i)
		{
			for (int j = 0; j < width; ++j)
			{
				texCoords.push_back(glm::vec2(j*xstep, i*ystep));
			}
		}

		//Faces stuff
		for (int h = 0; h < height - 1; ++h)
		{
			for (int w = 0; w < width - 1; ++w)
			{
				Face face;
				face[0] = w + h * width;
				face[1] = width * (h+1) + w;
				face[2] = (w+1) + h * width;
				faces.push_back(face);
				face[0] = (w+1) + h * width;
				face[1] = width * (h+1) + w;
				face[2] = width * (h+1) + w + 1;
				faces.push_back(face);
			}
		}

		//Bones indices (Empty for primitive)
		for (int i = 0; i < vertices.size(); ++i)
			this->boneIndices.push_back(glm::ivec4(-1));

		init();
	}


	virtual ~DynamicGridMesh()
	{
		glDeleteVertexArrays(1, &vao);
		glDeleteBuffers(vbo_index::NUM, vbo);
		glDeleteBuffers(1, &ebo);
	}

	virtual void Draw() const override
	{
		int faceCount = this->GetFaceCount();
		glDrawElements(GL_TRIANGLES, faceCount * 3, GL_UNSIGNED_INT, (void*)0);
	}

	virtual void DrawInstanced(int count) const override
	{
		int faceCount = this->GetFaceCount();
		glDrawElementsInstanced(GL_TRIANGLES, faceCount * 3, GL_UNSIGNED_INT, (void*)0, count);
	}

	void UpdateGridAttributes(std::vector<glm::vec4> const& verts,
		std::vector<glm::vec4> const& nmls,
		std::vector<glm::vec4> const& tg, 
		std::vector<glm::vec4> const& bitg)
	{
		//Allocate in gpu
		glBindBuffer(GL_ARRAY_BUFFER, this->vbo[VERTICES]);
		glBufferData(GL_ARRAY_BUFFER, GetVertexCount() * 4 * sizeof(GLfloat), &verts[0][0], GL_DYNAMIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, this->vbo[NORMALS]);
		glBufferData(GL_ARRAY_BUFFER, GetVertexCount() * 4 * sizeof(GLfloat), &nmls[0][0], GL_DYNAMIC_DRAW);

		//Tgt's and Bitgt's
		glBindBuffer(GL_ARRAY_BUFFER, this->vbo[TANGENTS]);
		glBufferData(GL_ARRAY_BUFFER, GetVertexCount() * 4 * sizeof(GLfloat), &tg[0][0], GL_DYNAMIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, this->vbo[BITANGENTS]);
		glBufferData(GL_ARRAY_BUFFER, GetVertexCount() * 4 * sizeof(GLfloat), &bitg[0][0], GL_DYNAMIC_DRAW);
	}


//PRIVATE METHODS
protected:

	virtual void init() override
	{
		//Generate VAO, VBO and EBO
		glGenVertexArrays(1, &this->vao);
		glGenBuffers(vbo_index::NUM, this->vbo);
		glGenBuffers(1, &this->ebo);

		//Bins the VAO
		glBindVertexArray(this->vao);

		//Allocate in gpu
		glBindBuffer(GL_ARRAY_BUFFER, this->vbo[VERTICES]);
		glBufferData(GL_ARRAY_BUFFER, GetVertexCount() * 4 * sizeof(GLfloat), &vertices[0][0], GL_DYNAMIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, this->vbo[NORMALS]);
		glBufferData(GL_ARRAY_BUFFER, GetVertexCount() * 4 * sizeof(GLfloat), &normals[0][0], GL_DYNAMIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, this->vbo[TEXCOORDS]);
		glBufferData(GL_ARRAY_BUFFER, GetVertexCount() * 2 * sizeof(GLfloat), &texCoords[0][0], GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, this->vbo[BONE_INDICES]);
		glBufferData(GL_ARRAY_BUFFER, GetVertexCount() * 4 * sizeof(GLint), &boneIndices[0][0], GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, this->vbo[BONE_WEIGHTS]);
		glBufferData(GL_ARRAY_BUFFER, GetVertexCount() * 4 * sizeof(GLfloat), &boneWeights[0][0], GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, this->vbo[TANGENTS]);
		glBufferData(GL_ARRAY_BUFFER, GetVertexCount() * 4 * sizeof(GLfloat), &m_tangents[0][0], GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, this->vbo[BITANGENTS]);
		glBufferData(GL_ARRAY_BUFFER, GetVertexCount() * 4 * sizeof(GLfloat), &m_bitangents[0][0], GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->GetFaceCount() * sizeof(Mesh::Face), &faces[0][0], GL_STATIC_DRAW);

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

protected:
	unsigned width;
	unsigned height;
};