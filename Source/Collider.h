///HEADER STUFF

#pragma once

///INCLUDES
#include "../External/Includes/glm/glm.hpp"


class Mesh;

/*
class Collider
{
public:
	Collider() {}
	virtual ~Collider() {}

	glm::mat4 const& GetInertialTensor() const 
	{
		return Ibody;
	}

	glm::mat4 const& GetInertialTensorInv() const
	{
		return IbodyInv;
	}

	virtual void InertiaTensorSetup() = 0;

	virtual void DebugDraw() = 0;

protected:
	glm::mat4 Ibody;
	glm::mat4 IbodyInv;

	//We need body coords, so it will have 
	//a reference to a mesh with this info
	Mesh *mesh;

	//Something related to AABB or OBB
	//TODO
};


/////////////////////////////////////////
//////		DIFFERENT SHAPES		/////
/////////////////////////////////////////
class OOBBCollider : public Collider 
{
public:
	BoxCollider(glm::vec3 const& size)
	{
		InertiaTensorSetup(size);
	}

	virtual void InertiaTensorSetup(std::vec3 const& size) 
	{
		Ibody = {
			glm::vec4(12, 0, 0, 0),
			glm::vec4(0, 12, 0, 0),
			glm::vec4(0, 0, 12, 0),
			glm::vec4(0, 0, 0, 1)
		};

		IbodyInv = glm::inverse(Ibody);
	}

	virtual void DebugDraw() 
	{
		//TODO
	}
};


class SphereCollider : public Collider 
{
public:
	virtual void InertiaTensorSetup()
	{
		//Code to determine the 
	}

	virtual void DebugDraw()
	{
		//TODO
	}
};

//*/