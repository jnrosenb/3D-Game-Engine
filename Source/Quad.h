///HEADER STUFF

#pragma once

#include "Mesh.h"


class Quad : public Mesh 
{

public:
    Quad();
    virtual ~Quad();

	//OVERRIDES
	virtual std::vector <glm::vec4>& GetVertices() override;
	virtual std::vector <glm::vec4>& GetNormals() override;
	virtual std::vector <glm::vec2>& GetTexCoords() override; //ALIGNMENT
	virtual std::vector <Mesh::Face>& GetFaces() override;

	virtual int GetVertexCount() const override;
	virtual int GetFaceCount() const override;
};
