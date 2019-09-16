///HEADER

#include "ResourceManager.h"
#include <iostream>

#include "../External/Includes/SDL2/SDL_surface.h"
#include "../External/Includes/SDL2/SDL_image.h"



ResourceManager::ResourceManager() 
{
	std::cout << "Resource manager constructor." << std::endl;
}

ResourceManager::~ResourceManager() 
{
	Unload();
	std::cout << "Resource manager destructor." << std::endl;
}


void ResourceManager::Unload()
{
	for (auto &node : mSurfaces)
	{
		std::cout << "Releasing surface resource." << std::endl;
		SDL_FreeSurface(node.second);
		std::cout << "--OK--." << std::endl;
	}
	mSurfaces.clear();


	for (auto &node : mShadersDic)
	{
		std::cout << "Releasing Shader resource." << std::endl;
		delete node.second;
		std::cout << "--OK--." << std::endl;
	}
	mShadersDic.clear();
}


Shader *ResourceManager::loadShader(std::string& shaderName)
{
	std::string path_vert = shaderName + ".vert";
	std::string path_frag = shaderName + ".frag";
	Shader *pShader = mShadersDic[shaderName];

	if (pShader == NULL)
	{
		std::cout << "Shader not loaded yet. Proceeding to load.------------" << std::endl;

		pShader = new Shader(path_vert.c_str(), path_frag.c_str());
		if (pShader)
		{
			std::cout << "Shader loaded succesfully. Adding to shader dictionary." << std::endl;
			mShadersDic[shaderName] = pShader;
		}
		else
		{
			std::cout << "Load failed *-*-*-*-*" << std::endl;
		}
	}
	else
	{
		std::cout << "Shader was already loaded. Proceeding to return Shader * * * * * " << std::endl;
	}

	return pShader;
}


SDL_Surface *ResourceManager::loadSurface(std::string path) 
{
	path = TextureBasicDir + path;
	SDL_Surface *pSurface = mSurfaces[path];

	if (pSurface == NULL) 
	{
		std::cout << "Image not loaded yet. Proceeding to load." << std::endl;

		pSurface = IMG_Load(path.c_str());
		if (pSurface) 
		{
			std::cout << "Image loaded succesfully. Adding to mSurfaces dictionary." << std::endl;
			mSurfaces[path] = pSurface;
		}
		else 
		{
			std::cout << "Load failed." << std::endl;
		}
	}
	else 
	{
		std::cout << "Image was already loaded. Proceeding to return surface." << std::endl;
	}

	return pSurface;
}