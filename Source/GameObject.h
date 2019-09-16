///HEADER STUFF

#pragma once

///INCLUDES
#include <vector>

class BaseComponent;

class GameObject
{

//PUBLIC INTERFACE
public:
	GameObject();
	GameObject(GameObject& rhs);
	virtual ~GameObject();

	virtual void Update(float dt);
	virtual void Draw();

	template <typename T>
	T* GetComponent() 
	{
		for (auto comp : components)
		{
			if (T::comp_class_type == comp->GetType())
				return static_cast<T*>(comp);
		}

		return nullptr;
	}

	template <typename T>
	T* AddComponent()
	{
		COMPONENT_TYPES type = T::comp_class_type;

		//TODO create a new comp of that type, add it, and return it
		T* component = static_cast<T*>(factory.GetComponent(type, this));
		if (component)
		{
			this->components.push_back(component);
			return component;
		}

		return nullptr;
	}
	
	int GetId() const;

//VARIABLEs
private:
	std::vector<BaseComponent*> components;

	//Not sure if this will be used
	static int go_count;
	unsigned int gameobj_ID;

};