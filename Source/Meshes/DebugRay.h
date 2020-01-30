///HEADER STUFF

#pragma once

#include <vector>
#include "../Mesh.h"


class DebugRay : public Mesh
{
public:
	DebugRay();
	virtual ~DebugRay();

	virtual std::vector <glm::vec4>& GetVertices();
	virtual int GetVertexCount() const;

	virtual void Draw() const override;
	virtual void DrawInstanced(int count) const override;

private:
	virtual void init() override;
};
