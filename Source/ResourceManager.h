///HEADER

#pragma  once

#include "../External/Includes/SDL2/SDL_surface.h"
#include "../External/Includes/SDL2/SDL_image.h"
#include <unordered_map>
#include <string>

class Shader;


struct HDRImageDesc
{
	std::string name;
	int width, height, nComponents;
	float *data;
};


class ResourceManager 
{
public:
	ResourceManager();
	virtual ~ResourceManager();

	void Unload();

	HDRImageDesc loadHDR(std::string path);
	SDL_Surface *loadSurface(std::string path);
	Shader *loadShader(std::string& path);

private:
	std::unordered_map<std::string, SDL_Surface *> mSurfaces;
	std::unordered_map<std::string, HDRImageDesc> mHDRTextures;
	std::unordered_map<std::string, Shader *> mShadersDic;

	//Assets dirs
	std::string TextureDir;
	std::string TextureBasicDir;
	std::string TextureBackgroundDir;
	std::string TextureNormalMapDir;
	std::string TextureParticleDir;
	std::string TextureUIDir;
	std::string HDRDir;
	
	//MODEL DIR
	std::string ModelDir;
};