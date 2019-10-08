///HEADER STUFF

#pragma once

///INCLUDES
///#include "../External/Includes/GL/glew.h"
#include "../External/Includes/SDL2/SDL_surface.h"
#include "../External/Includes/Glad_/Glad/glad.h"
#include <gl/GL.h>
#include "../External/Includes/glm/glm.hpp"
#include <vector>
#include <unordered_map>
#include "Stat.h"


#define VIEWPORT_WIDTH						1280
#define VIEWPORT_HEIGHT						720
#define MAX_LIGHT_COUNT						2000	//Arbitrary value
#define MAX_NUMBER_TEXTURES					512		//Arbitrary value


//Forward decl
class Shader;
class Mesh;
class Camera;
class RenderTarget;
class Bone;


//Lights (UNUSED FOR NOW)
struct DirectionalLight
{
	glm::vec4 eye;
	glm::vec4 color;
	glm::vec4 look;
	float near, far;
	int width, height;
	float shadowIntensity;

	//Intensity of light coded in alpha of light color ***

	DirectionalLight() : eye(glm::vec4(0.0f, 10.0f, -10.0f, 1)), 
		color(glm::vec4(1.f, 1.f, 1.f, 1.f)), look(glm::vec4(0.0f, -1.0f, -1.0f, 0)), 
		near(0.1f), far(1000.0f), width(40), height(30), shadowIntensity(1.f)
	{}

	DirectionalLight(float x, float y, float z, float r, float g, float b,
		float dx, float dy, float dz, float n, float f) : 
		eye(glm::vec4(x, y, z, 1)), color(glm::vec4(r, g, b, 1.f)),
		look(glm::vec4(dx, dy, dz, 0)), near(n), far(f)
	{}
};


//Struct with the data needed by renderer to draw everything
struct DrawData 
{
	glm::mat4 model;
	glm::mat4 normalsModel;
	std::vector<Mesh*> *meshes;
	Shader *shader;
	GLuint diffuseTexture;

	unsigned boneCount;
	Bone *root;

	//Bones experiment
	std::vector<glm::mat4> *BoneTransformations;
};


//RENDERER CLASS
class Renderer 
{
public:
	friend class Factory;

//PUBLIC INTERFACE
public:
	Renderer() : lightCount(0)
	{}
	virtual ~Renderer() {}

	//Initialization
	virtual void init() = 0;
	virtual void initCamera() = 0;
	virtual void initFrameBuffers() = 0;

	//Data that will be drawn into the scene
	virtual void QueueForDraw(DrawData& data) = 0;
	virtual void QueueForDrawAlpha(DrawData& data) = 0;

	///virtual void loadResources();
	///virtual void CalculateLightProjView();

	//TEXTURE LOADING
	virtual GLuint generateTextureFromSurface(SDL_Surface *surface, std::string key) = 0;
	virtual GLuint GetTexture(std::string key) = 0;

	virtual void Update(float dt) = 0;
	virtual void Draw() = 0;

//LATER MOVE TO PROTECTED
public:

	//Compute shader for blurring
	void SetKernelCount(int newVal) 
	{
		kernelCount = newVal;
		OnChangingKernelCount();
	}
	int GetKernelCount() const
	{
		return kernelCount;
	}

//VARIABLES
protected:

	//Compute shader for blurring
	void OnChangingKernelCount() 
	{
		this->weights.clear();
		int kernel_radius = (kernelCount - 1) * 0.5f;
		AuxMath::genGaussianWeights(kernel_radius, this->weights);
	}
	Shader *blurShaderHoriz;
	Shader *blurShaderVert;
	std::vector<float> weights;
	int kernelCount;

	//LIGHTS
	glm::vec4 Light_Colors[MAX_LIGHT_COUNT];
	glm::vec4 Light_Positions[MAX_LIGHT_COUNT];
	float Light_Radius[MAX_LIGHT_COUNT];
	int lightCount;
	DirectionalLight sun;
};