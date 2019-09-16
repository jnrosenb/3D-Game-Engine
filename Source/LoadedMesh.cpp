///HEADER

#include "LoadedMesh.h"
///#include "Helper\Transformations3D.h"
///#include "glm/gtx/transform.hpp"


LoadedMesh::LoadedMesh( std::vector<glm::vec4> &vertices,
	std::vector<glm::vec4> &normals, std::vector<glm::vec2> &uvs,
	std::vector<Mesh::Face> &faces, std::vector<glm::vec4> &tangents, 
	std::vector<glm::vec4> &bitangents) : Mesh()
{
	/*Bounding box, initial bounding values
	Bounding_Limits.push_back(std::numeric_limits<float>::max());
	Bounding_Limits.push_back(-std::numeric_limits<float>::max());
	Bounding_Limits.push_back(std::numeric_limits<float>::max());
	Bounding_Limits.push_back(-std::numeric_limits<float>::max());
	Bounding_Limits.push_back(std::numeric_limits<float>::max());
	Bounding_Limits.push_back(-std::numeric_limits<float>::max());
	//*/

	Load_Vertices(vertices);
	Load_Normals(normals);
	Load_TexCoords(uvs);
	Load_Faces(faces);
	///Load_Tangents_and_Bitangents(tangents, bitangents);
	
	//Once all info is loaded, setup the openGL side
	init();
}

LoadedMesh::~LoadedMesh() 
{
}

void LoadedMesh::Load_Vertices(std::vector<glm::vec4> &vertices)
{
	for (glm::vec4 const &vertex : vertices) 
	{
		this->vertices.push_back(vertex);

		/*Bounding box stuff
		if (vertex.x < Bounding_Limits[0]) Bounding_Limits[0] = vertex.x;		//xmin
		else if (vertex.x > Bounding_Limits[1]) Bounding_Limits[1] = vertex.x;	//xmax
		if (vertex.y < Bounding_Limits[2]) Bounding_Limits[2] = vertex.y;		//ymin
		else if (vertex.y > Bounding_Limits[3]) Bounding_Limits[3] = vertex.y;	//ymax
		if (vertex.z < Bounding_Limits[4]) Bounding_Limits[4] = vertex.z;		//zmin
		else if (vertex.z > Bounding_Limits[5]) Bounding_Limits[5] = vertex.z;	//zmax
		//*/
	}
}

void LoadedMesh::Load_Normals(std::vector<glm::vec4> &normals)
{
	for (glm::vec4 const& normal : normals)
	{
		this->normals.push_back(normal);
	}
}

void LoadedMesh::Load_TexCoords(std::vector<glm::vec2> &uvs)
{
	for (glm::vec2 const &uv : uvs)
	{
		this->texCoords.push_back(uv);
	}
}

void LoadedMesh::Load_Faces(std::vector<Mesh::Face> &faces)
{
	for (Mesh::Face const &face : faces)
	{
		this->faces.push_back(face);
	}
}

/*
void LoadedMesh::Load_Tangents_and_Bitangents(std::vector<glm::vec4> &tangents, 
	std::vector<glm::vec4> &bitangents)
{
	for (int i = 0; i < tangents.size(); ++i)
	{
		m_tangents.push_back(tangents[i]);
		m_bitangents.push_back(bitangents[i]);
	}
}
//*/

////////////////////////////
////	OVERRIDES		////
////////////////////////////
/*
void LoadedMesh::init()
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

	//Unbind Everything
	glBindVertexArray(0);
	///glBindBuffer(GL_ARRAY_BUFFER, 0);
	///glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
//*/


std::vector <glm::vec4>& LoadedMesh::GetVertices()
{
	return this->vertices;
}

std::vector <glm::vec4>& LoadedMesh::GetNormals()
{
	return this->normals;
}

std::vector <glm::vec2>& LoadedMesh::GetTexCoords()
{
	return this->texCoords;
}

std::vector <Mesh::Face>& LoadedMesh::GetFaces()
{
	return this->faces;
}

int LoadedMesh::GetVertexCount() const
{
	return this->vertices.size();
}

int LoadedMesh::GetFaceCount() const
{
	return this->faces.size();
}