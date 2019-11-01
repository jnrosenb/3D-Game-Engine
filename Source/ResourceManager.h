///HEADER

#pragma  once

#include "../External/Includes/SDL2/SDL_surface.h"
#include "../External/Includes/SDL2/SDL_image.h"
#include "Shader.h"
#include <unordered_map>
#include <string>


struct HDRImageDesc
{
	std::string name;
	int width, height, nComponents;
	float *data;
};


//PROJECT DIR
static std::string const projectDir = "C:\\Users\\Jose\\Desktop\\OpenGl_Framework\\Source\\";

//TEXTURE DIR
static std::string const TextureDir = "C:\\Users\\Jose\\Desktop\\OpenGl_Framework\\Assets\\Textures\\";
static std::string const TextureBasicDir		= TextureDir + "Basic\\";
static std::string const TextureBackgroundDir	= TextureDir + "Background\\";
static std::string const TextureNormalMapDir	= TextureDir + "Normal_Maps\\";
static std::string const TextureParticleDir		= TextureDir + "Particles\\";
static std::string const TextureUIDir			= TextureDir + "UI\\";
static std::string const HDRDir					= TextureDir + "HDR\\";

//MODEL DIR
static std::string const ModelDir = "C:\\Users\\Jose\\Desktop\\OpenGl_Framework\\Assets\\Models\\";

class ResourceManager 
{
public:
	ResourceManager();
	virtual ~ResourceManager();
	
	void LoadShaders();

	void Unload();

	HDRImageDesc loadHDR(std::string path);
	SDL_Surface *loadSurface(std::string path);
	Shader *loadShader(std::string& path);

private:
	std::unordered_map<std::string, SDL_Surface *> mSurfaces;
	std::unordered_map<std::string, HDRImageDesc> mHDRTextures;
	std::unordered_map<std::string, Shader *> mShadersDic;
};