///HEADER STUFF

#pragma once

#include <vector>
#include "../Mesh.h"


class DebugBox : public Mesh
{

public:
	DebugBox(std::vector<glm::vec4> const& points);
	virtual ~DebugBox();

	virtual std::vector <glm::vec4>& GetVertices();
	virtual int GetVertexCount() const;

	virtual void Draw() const override;
	virtual void DrawInstanced(int count) const override;

private:
	virtual void init() override;
};



class DebugBoxFilled : public Mesh
{

public:
	DebugBoxFilled(glm::vec3 size);
	virtual ~DebugBoxFilled();

	virtual std::vector <glm::vec4>& GetVertices();
	virtual int GetVertexCount() const;

	virtual void Draw() const override;
	virtual void DrawInstanced(int count) const override;

private:
	virtual void init() override;
};

