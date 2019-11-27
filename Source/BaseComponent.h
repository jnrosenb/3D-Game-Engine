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
	virtual void Update(float dt) = 0;
	virtual void LateUpdate(float dt) {}
	virtual void DeserializeInit() = 0;

	COMPONENT_TYPES GetType() const { return type; }

private:
	BaseComponent(BaseComponent& rhs);

protected:
	COMPONENT_TYPES type;
	GameObject *m_owner;
};