///HEADER

#include <iostream>
#include "LoadedMesh.h"
///#include "Helper\Transformations3D.h"
///#include "glm/gtx/transform.hpp"


LoadedMesh::LoadedMesh
( 
	std::vector<glm::vec4> &vertices,
	std::vector<glm::vec4> &normals, 
	std::vector<glm::vec2> &uvs,
	std::vector<Mesh::Face> &faces, 
	std::vector<glm::vec4> &tangents, 
	std::vector<glm::vec4> &bitangents,
	std::vector<std::vector<int>>& bones_indices,
	std::vector<std::vector<float>>& bones_weights,
	std::unordered_map<std::string, glm::mat4>& BoneOffsetMap) : Mesh()
{
	//Bounding box stuff
	/*Bounding box, initial bounding values
	Bounding_Limits.push_back(std::numeric_limits<float>::max());
	Bounding_Limits.push_back(-std::numeric_limits<float>::max());
	Bounding_Limits.push_back(std::numeric_limits<float>::max());
	Bounding_Limits.push_back(-std::numeric_limits<float>::max());
	Bounding_Limits.push_back(std::numeric_limits<float>::max());
	Bounding_Limits.push_back(-std::numeric_limits<float>::max());
	//*/

	//Usual stuff
	Load_Vertices(vertices);
	Load_Normals(normals);
	Load_TexCoords(uvs);
	Load_Faces(faces);

	//Normal map
	Load_Tangents_and_Bitangents(tangents, bitangents);
	
	//Bone information loading
	Load_BoneIndices(bones_indices);
	Load_BoneWeights(bones_weights);

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

void LoadedMesh::Load_BoneIndices(std::vector<std::vector<int>> const& indices)
{
	for (std::vector<int> const& index : indices)
	{
		unsigned index_count = index.size();
		glm::ivec4 ind;

		switch (index_count) 
		{
		case 0:
			ind = glm::ivec4(-1, -1, -1, -1);
			break;
		case 1:
			ind = glm::ivec4(index[0], -1, -1, -1);
			break;
		case 2:
			ind = glm::ivec4(index[0], index[1], -1, -1);
			break;
		case 3:
			ind = glm::ivec4(index[0], index[1], index[2], -1);
			break;
		case 4:
			ind = glm::ivec4(index[0], index[1], index[2], index[3]);
			break;
		default:
			ind = glm::ivec4(index[0], index[1], index[2], index[3]);
			break;
		};

		///if (index_count == 0)
		///	continue;
		///else if (index_count == 1)
		///	ind.x = glm::ivec4(index[0], 0.0f, 0.0f, 0.0f);
		///else if (index_count == 2)
		///	ind = glm::ivec4(index[0], index[1], 0.0f, 0.0f);
		///else if (index_count == 3)
		///	ind = glm::ivec4(index[0], index[1], index[2], 0.0f);
		///else
		///	ind = glm::ivec4(index[0], index[1], index[2], index[3]);

		this->boneIndices.push_back(ind);
	}
}

void LoadedMesh::Load_BoneWeights(std::vector<std::vector<float>> const& weights)
{
	for (std::vector<float> const& weight : weights)
	{
		
		unsigned weight_count = weight.size();
		glm::vec4 wgts = glm::vec4(0);

		switch (weight_count)
		{
		case 0:
			wgts = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
			break;
		case 1:
			wgts = glm::vec4(weight[0], 0.0f, 0.0f, 0.0f);
			break;
		case 2:
			wgts = glm::vec4(weight[0], weight[1], 0.0f, 0.0f);
			break;
		case 3:
			wgts = glm::vec4(weight[0], weight[1], weight[2], 0.0f);
			break;
		case 4:
			wgts = glm::vec4(weight[0], weight[1], weight[2], weight[3]);
			break;
		default:
			float sum = weight[0] + weight[1] + weight[2] + weight[3];
			wgts = glm::vec4(weight[0] / sum, weight[1] / sum, weight[2] / sum, weight[3] / sum);
			break;
		};

		this->boneWeights.push_back(wgts);
		///this->boneWeights.push_back(glm::normalize(wgts));

		/// if (weight_count > 0 && 
		/// 	abs((wgts.x + wgts.y + wgts.z + wgts.w) - 1.0f) >= 0.00001f)
		/// {
		/// 	int a = 234;
		/// }
	}
}


void LoadedMesh::Load_Tangents_and_Bitangents(std::vector<glm::vec4> &tangents, 
	std::vector<glm::vec4> &bitangents)
{
	for (int i = 0; i < tangents.size(); ++i)
	{
		m_tangents.push_back(tangents[i]);
		m_bitangents.push_back(bitangents[i]);
	}

	passTangentSpaceInfo = true;
}

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

std::vector<glm::ivec4> const& LoadedMesh::GetBoneIndices() const
{
	return this->boneIndices;
}

std::vector<glm::vec4> const& LoadedMesh::GetBoneWeights() const
{
	return this->boneWeights;
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