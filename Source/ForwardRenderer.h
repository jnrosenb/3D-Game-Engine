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
#include "Renderer.h"


#define VIEWPORT_WIDTH						1280
#define VIEWPORT_HEIGHT						720
#define MAX_NUMBER_TEXTURES					512		//Arbitrary value


//Forward decl
class Camera;
class RenderTarget;



//RENDERER CLASS
class ForwardRenderer : public Renderer
{
public:
	friend class Factory;

//PUBLIC INTERFACE
public:
	ForwardRenderer();
	virtual ~ForwardRenderer();

	//Initialization
	virtual void init();
	virtual void initCamera();
	virtual void initFrameBuffers();

	//Data that will be drawn into the scene
	virtual void QueueForDraw(DrawData& data);
	virtual void QueueForDrawAlpha(DrawData& data);

	void loadResources();
	void CalculateLightProjView();

	//TEXTURE LOADING
	GLuint generateTextureFromSurface(SDL_Surface *surface, std::string key);
	GLuint GetTexture(std::string key);

	virtual void Update(float dt);
	virtual void Draw();

//PRIVATE METHODS
private:
	ForwardRenderer(ForwardRenderer& rhs);

	//Uniform buffer object (LATER TAKE OUT OF HERE)
	void initUniformBufferObjects();


	//TEXTURE LOADING
	//Map that pairs surface to glTexture
	std::unordered_map<std::string, GLint> texturesDict;
	GLuint textures[MAX_NUMBER_TEXTURES];
	int numberOfTexturesLoaded;


	//UNIFORM BUFFER OBJECT DATA PASSING STUFF
	//TODO - Ask question of templates and weird last line include (?)
	void UniformBlockBind(GLuint ubo);
	void UniformBlockUnbind();
	template<typename T>
	void UniformBlockPassData(GLuint ubo, int begin, T data);
	template<typename T>
	void UniformBlockPassData(GLuint ubo, int begin, int count, T *data);
	void UniformBlockPassData(GLuint ubo, int begin, glm::vec2& data);
	void UniformBlockPassData(GLuint ubo, int begin, glm::vec3& data);
	void UniformBlockPassData(GLuint ubo, int begin, glm::vec4& data);
	void UniformBlockPassData(GLuint ubo, int begin, glm::mat4& data);


//VARIABLES
private:
	//SHADOW MAP STUFF - TODO (move from here)
	Shader *shadowShader;
	glm::mat4 lightProjView;

	//Elements to be drawn
	std::vector<DrawData> graphicQueue;
	std::vector<DrawData> graphicQueueAlpha; //TODO - Change to a std::set or something

	//First person camera
	Camera *currentCamera; //Future version. Will hold the current camera to render scene
	Camera *FPCamera;

	//UBO
	GLuint ubo_test;

	//Framebuffer
	RenderTarget *framebuffer;
};