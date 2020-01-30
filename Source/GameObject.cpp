///HEADER STUFF

#include "Factory.h"
#include "GameObject.h"
#include "BaseComponent.h"


//TODO Later move this
extern ComponentFactory factory;


// Initialize static member of class
int GameObject::go_count = 0;


GameObject::GameObject()
{
	gameobj_ID = go_count++;
}


GameObject::GameObject(GameObject& rhs)
{
	//TODO
}


GameObject::~GameObject()
{
	std::cout << "Destroying GO - Proceeding to delete components" << std::endl;
	for (auto comp : components)
	{
		delete comp;
	}
	components.clear();
	std::cout << "Destroyed GO" << std::endl;
}


//This method gets called after all the 
//gameobjects have been created this frame
void GameObject::Begin()
{
	for (auto comp : components)
	{
		comp->Begin();
	}
}


void GameObject::Update(float dt)
{
	for (auto comp : components)
	{
		comp->Update(dt);
	}
}


void GameObject::LateUpdate(float dt)
{
	for (auto comp : components)
	{
		comp->LateUpdate(dt);
	}
}


void GameObject::Draw()
{
	for (auto comp : components)
	{
		comp->Draw();
	}
}

/*
template <typename T>
T* GameObject::GetComponent()
{
	for (auto comp : components)
	{
		if (T::comp_class_type == comp->GetType())
			return static_cast<T*>(comp);
	}

	return nullptr;
}


template <typename T>
T* GameObject::AddComponent()
{
	COMPONENT_TYPES type = T::comp_class_type;

	//TODO create a new comp of that type, add it, and return it
	T* component = factory.GetComponent(type);
	if (component) 
	{
		this->components.push_back(component);
		return component;
	}

	return nullptr;
}
//*/

int GameObject::GetId() const
{
	return this->gameobj_ID;
}
