///HEADER STUFF

#include "GameObjectManager.h"
#include "GameObject.h"
#include <iostream>


GameobjectManager::GameobjectManager() 
{
	std::cout << "Created GO manager" << std::endl;
}

GameobjectManager::~GameobjectManager()
{
	std::cout << "Destroying GO manager - Proceeding to delete gameobjects" << std::endl;
	for (auto go : gameObjects) 
	{
		delete go;
	}
	gameObjects.clear();
	std::cout << "Destroyed GO manager" << std::endl;
}

void GameobjectManager::Update(float dt)
{
	for (auto go : gameObjects)
	{
		go->Update(dt);
	}
}


void GameobjectManager::LateUpdate(float dt)
{
	for (auto go : gameObjects)
	{
		go->LateUpdate(dt);
	}
}

GameObject* GameobjectManager::AddGameObjects(GameObject *go)
{
	if (go) 
	{
		gameObjects.push_back(go);
		return go;
	}

	return nullptr;
}

void GameobjectManager::RemoveGameObjects(int go_id)
{
	for (auto go : gameObjects)
	{
		if (go->GetId() == go_id) 
		{
			gameObjects.remove(go);
			return;
		}
	}

	std::cout << "GO to be destroyed not found" << std::endl;
}

GameObject* GameobjectManager::GetGameObject(int go_id)
{
	for (auto go : gameObjects)
	{
		if (go->GetId() == go_id)
		{
			return go;
		}
	}

	std::cout << "GO not found" << std::endl;
	return nullptr;
}

