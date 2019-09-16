///HEADER

#include "Quad.h"


Quad::Quad() 
{
    //Load Vertices
	vertices.push_back(glm::vec4(1.f, 1.f, 0.0f, 1.0f));
	vertices.push_back(glm::vec4(-1.f, 1.f, 0.0f, 1.0f));
	vertices.push_back(glm::vec4(-1.f, -1.f, 0.0f, 1.0f));
	vertices.push_back(glm::vec4(1.f, -1.f, 0.0f, 1.0f));
	///vertices.push_back(glm::vec4(0.5f, 0.5f, 0.0f, 1.0f));
	///vertices.push_back(glm::vec4(-0.5f, 0.5f, 0.0f, 1.0f));
	///vertices.push_back(glm::vec4(-0.5f, -0.5f, 0.0f, 1.0f));
	///vertices.push_back(glm::vec4(0.5f, -0.5f, 0.0f, 1.0f));

    //Load Normals
	normals.push_back(glm::vec4(0.0f, 0.0f, 1.0f, 0.0f));
	normals.push_back(glm::vec4(0.0f, 0.0f, 1.0f, 0.0f));
	normals.push_back(glm::vec4(0.0f, 0.0f, 1.0f, 0.0f));
	normals.push_back(glm::vec4(0.0f, 0.0f, 1.0f, 0.0f));

    //Load UVs
	texCoords.push_back(glm::vec2(1.0f, 1.0f));
	texCoords.push_back(glm::vec2(0.0f, 1.0f));
	texCoords.push_back(glm::vec2(0.0f, 0.0f));
	texCoords.push_back(glm::vec2(1.0f, 0.0f));

    //Load Faces
	Mesh::Face upperTriangle;
	Mesh::Face bottomTriangle;
	upperTriangle[0] = 0;
	upperTriangle[1] = 1;
	upperTriangle[2] = 2;
	bottomTriangle[0] = 3;
	bottomTriangle[1] = 0;
	bottomTriangle[2] = 2;
	faces.push_back(upperTriangle);
	faces.push_back(bottomTriangle);

    init();
}

Quad::~Quad() 
{
}


int Quad::GetVertexCount() const
{
	return vertices.size();
}


std::vector<glm::vec4>& Quad::GetVertices()
{
	return vertices;
}


std::vector<glm::vec4>& Quad::GetNormals()
{
	return normals;
}


std::vector <glm::vec2>& Quad::GetTexCoords() //Alignment?
{
	return texCoords;
}


std::vector<Mesh::Face>& Quad::GetFaces()
{
	return faces;
}


int Quad::GetFaceCount() const
{
	return faces.size();
}