///HEADER STUFF

#pragma once

#include <vector>
#include "Mesh.h"


class Cube : public Mesh
{

public:
	Cube();
	virtual ~Cube();

	virtual std::vector <glm::vec4>& GetVertices();
	virtual std::vector <glm::vec4>& GetNormals();
	virtual std::vector <glm::vec2>& GetTexCoords(); //Alignment?
	virtual std::vector <Mesh::Face>& GetFaces();

	virtual int GetVertexCount() const;
	virtual int GetFaceCount() const;

	virtual void Draw() const override;
	virtual void DrawInstanced(int count) const override;

private:
	virtual void init() override;
};

