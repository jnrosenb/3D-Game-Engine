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


#define VIEWPORT_WIDTH						1280
#define VIEWPORT_HEIGHT						720
#define MAX_LIGHT_COUNT						2000	//Arbitrary value
#define MAX_NUMBER_TEXTURES					512		//Arbitrary value


//Forward decl
class Shader;
class Mesh;
class Camera;
class RenderTarget;


//Lights (UNUSED FOR NOW)
struct DirectionalLight
{
	glm::vec4 eye;
	glm::vec4 look;
	float near, far;

	DirectionalLight() : eye(glm::vec4(0.0f, 10.0f, -10.0f, 1)), 
		look(glm::vec4(0.0f, -1.0f, -1.0f, 0)), near(0.1f), far(1000.0f)
	{}

	DirectionalLight(float x, float y, float z, 
		float dx, float dy, float dz, float n, float f) : 
		eye(glm::vec4(x, y, z, 1)), look(glm::vec4(dx, dy, dz, 0)), 
		near(n), far(f)
	{}
};


//Struct with the data needed by renderer to draw everything
struct DrawData 
{
	glm::mat4 model;
	glm::mat4 normalsModel;
	Mesh *mesh;
	Shader *shader;
	GLuint diffuseTexture;
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


//VARIABLES
protected:

	//LIGHTS
	glm::vec4 Light_Colors[MAX_LIGHT_COUNT];
	glm::vec4 Light_Positions[MAX_LIGHT_COUNT];
	float Light_Radius[MAX_LIGHT_COUNT];
	int lightCount;
	DirectionalLight sun;
};