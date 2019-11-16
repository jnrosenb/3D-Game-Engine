///HEADER STUFF

#pragma once

///INCLUDES
#include <list>

class GameObject;


class GameobjectManager 
{

//PUBLIC INTERFACE
public:
	GameobjectManager();
	virtual ~GameobjectManager();

	virtual void Update(float dt);
	virtual void LateUpdate(float dt);

	GameObject* AddGameObjects(GameObject *go);
	void RemoveGameObjects(int go_id);
	GameObject* GetGameObject(int go_id);

private:
	std::list<GameObject*> gameObjects;

};