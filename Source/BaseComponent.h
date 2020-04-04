///HEADER STUFF

#pragma once

///INCLUDES
#include <iostream>

class GameObject;


//FOR COMPARING
enum COMPONENT_TYPES
{
	TRANSFORM = 0,
	RENDERER,
	ANIMATION,
	PARTICLE_SYSTEM,
	RIGIDBODY,
	GRID_WAVE,		// WAVE STUFF, still erase later
	PATH_FOLLOW,	//UGH, ERASE
	IK_GOAL,		//UGH, ERASE
	CLOTH,			//SAME
	COUNT
};


class BaseComponent 
{
public:
	friend class ComponentFactory;

//PUBLIC INTERFACE
public:
	BaseComponent(GameObject *owner, COMPONENT_TYPES type)
	{
		this->m_owner = owner;
		this->type = type;
	}

	virtual ~BaseComponent() 
	{
		//TODO
	}

	virtual BaseComponent* clone() = 0;
	virtual void Update(float dt) {};
	virtual void LateUpdate(float dt) {}
	virtual void PhysicsUpdate(float dt) {}
	virtual void Draw() {}

	//These two should be privates. 
	//Also, start thinking how to bring this together with scripting
	virtual void DeserializeInit() = 0;
	virtual void Begin() {}

	COMPONENT_TYPES GetType() const { return type; }
	
	GameObject *GetOwner() const { return m_owner; }

private:
	BaseComponent(BaseComponent& rhs);

protected:
	COMPONENT_TYPES type;
	GameObject *m_owner;
};