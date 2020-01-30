///HEADER STUFF

#pragma once

#include <vector>
#include "../Mesh.h"


class DebugSphereOutline : public Mesh
{

public:
	DebugSphereOutline(int subdivissions);
	virtual ~DebugSphereOutline();

	virtual std::vector <glm::vec4>& GetVertices();
	virtual int GetVertexCount() const;

	virtual void Draw() const override;
	virtual void DrawInstanced(int count) const override;

private:
	virtual void init() override;
};



class DebugSphereFilled : public Mesh
{

public:
	DebugSphereFilled(int subdivissions);
	virtual ~DebugSphereFilled();

	virtual std::vector <glm::vec4>& GetVertices();
	virtual int GetVertexCount() const;

	virtual void Draw() const override;
	virtual void DrawInstanced(int count) const override;

private:
	virtual void init() override;
};

