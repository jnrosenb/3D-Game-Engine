///HEADER STUFF

#pragma once

///INCLUDES
#include <list>

class GameObject;


class GameobjectManager 
{
public:
	friend class Factory;

//PUBLIC INTERFACE
public:
	GameobjectManager();
	virtual ~GameobjectManager();

	virtual void Update(float dt);
	virtual void LateUpdate(float dt);
	
	virtual void Draw();

	GameObject* AddGameObjects(GameObject *go);
	void RemoveGameObjects(int go_id);
	GameObject* GetGameObject(int go_id);

private:
	virtual void CallBeginOnGameObjects();

private:
	std::list<GameObject*> gameObjects;

};