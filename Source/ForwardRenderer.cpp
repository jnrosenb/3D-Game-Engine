/// HEADER STUFF

#include <iostream>
#include "ForwardRenderer.h"

#include "RenderTarget.h"
#include "Shader.h"
#include "Camera.h"
#include "Sphere.h"



ForwardRenderer::ForwardRenderer() : numberOfTexturesLoaded(0)
{
	std::cout << "Renderer Constructor" << std::endl;
}


ForwardRenderer::~ForwardRenderer()
{
	//TODO delete stuff
	delete framebuffer;

	//Delete all the textures allocated by opengl
	glDeleteTextures(numberOfTexturesLoaded, textures);
	texturesDict.clear();
	numberOfTexturesLoaded = 0;

	//Shadow related stuff
	delete shadowShader;
}


void ForwardRenderer::init()
{

	////////////////////////////
	////	INITIALIZERS	////
	////////////////////////////

	initFrameBuffers();
	initCamera();
	initUniformBufferObjects(); 
	loadResources();


	////////////////////////////
	//// GL CONFIGURATIONS	////
	////////////////////////////
	
	//DEPTH
	glEnable(GL_DEPTH_TEST);

	//CULLING
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
}


void ForwardRenderer::initUniformBufferObjects()
{
	// FIRST UNIFORM BUFFER OBJECT (layout)-----------------------
	int index = 0;
	// mat4						    // 0   - 64
	// mat4						    // 64   -128
	// vec4 eye						// 128  -144
	// vec4 [MAX_LIGHT_COUNT]		// 144  -272
	// vec4 [MAX_LIGHT_COUNT]		// 272 - 400
	// float[MAX_LIGHT_COUNT]		// 400 - 528

	// 1- Generate buffer and bind
	glGenBuffers(1, &this->ubo_test);
	glBindBuffer(GL_UNIFORM_BUFFER, this->ubo_test);
	
	//Allocate gpu space
	glBufferData(GL_UNIFORM_BUFFER, 528, NULL, GL_STATIC_DRAW);
	
	//Map to an index
	glBindBufferBase(GL_UNIFORM_BUFFER, index, this->ubo_test);
	///glBindBufferRange(GL_UNIFORM_BUFFER, index, this->ubo_test, 0, 64);
	
	//Unbind
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	//------------------------------------------------------------
}


//Data that will be drawn into the scene
void ForwardRenderer::QueueForDraw(DrawData& data)
{
	graphicQueue.push_back(data);
}

void ForwardRenderer::QueueForDrawAlpha(DrawData& data)
{
	//TODO
}


void ForwardRenderer::Update(float dt)
{
	currentCamera->Update(dt);
}


void ForwardRenderer::Draw()
{
	//CALCULATE THE DIRECTIONAL LIGHT MATRICES
	CalculateLightProjView();


	//TODO - Better way to pass. Uniform block should know its own indices or calculate them
	//Pass uniforms and stuff
	glm::mat4 projView = currentCamera->getProj() * currentCamera->getView();
	UniformBlockBind(this->ubo_test);
	UniformBlockPassData(this->ubo_test, 0, projView);
	UniformBlockPassData(this->ubo_test, 64, lightProjView);
	UniformBlockPassData(this->ubo_test, 128, currentCamera->getEye());
	UniformBlockPassData(this->ubo_test, 144, MAX_LIGHT_COUNT, &Light_Colors[0][0]);
	UniformBlockPassData(this->ubo_test, 272, MAX_LIGHT_COUNT, &Light_Positions[0][0]);
	UniformBlockPassData(this->ubo_test, 400, MAX_LIGHT_COUNT, &Light_Radius[0]);
	UniformBlockUnbind();


	/*
	GLenum err = glGetError();
	if (err != GL_NO_ERROR)
	{
		switch (err)
		{
		case GL_INVALID_ENUM:
			std::cout << "ERROR1: GL_INVALID_ENUM" << std::endl;
			break;
		case GL_INVALID_VALUE:
			std::cout << "ERROR1: GL_INVALID_VALUE" << std::endl;
			break;
		case GL_INVALID_OPERATION:
			std::cout << "ERROR1: GL_INVALID_OPERATION" << std::endl;
			break;
		default:
			std::cout << "ERROR1: None of the previous" << std::endl;
			break;
		}
	}
	//*/


	//*FIRST PASS-----------------------------------------------
	framebuffer->Bind();
	glViewport(0, 0, framebuffer->getWidth(), framebuffer->getHeight()); //TODO - Change test values of width and height
	glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//Experiment with queues
	for (int i = 0; i < graphicQueue.size(); ++i)
	{
		//BINDING
		shadowShader->UseShader();

		DrawData &data = graphicQueue[i];
		for (int j = 0; j < data.meshes->size(); ++j)
		{
			//BINDING
			///graphicQueue[i].mesh->BindForDraw();
			(*data.meshes)[j]->BindForDraw();

			//UNIFORM BINDING
			shadowShader->setMat4f("projView", lightProjView);
			shadowShader->setMat4f("model", graphicQueue[i].model);

			//DRAW
			int faceCount = (*data.meshes)[j]->GetFaceCount();
			///int faceCount = graphicQueue[i].mesh->GetFaceCount();
			glDrawElements(GL_TRIANGLES, faceCount * 3, GL_UNSIGNED_INT, 0);

			//TODO - UNBIND SHADER AND MESH
			///graphicQueue[i].mesh->UnbindForDraw();
			(*data.meshes)[j]->UnbindForDraw();
		}

		shadowShader->UnbindShader();
	}
	//---------------------------------------------------------*/


	//SECOND PASS (Back to default framebuffer)-----------------------------------
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, VIEWPORT_WIDTH, VIEWPORT_HEIGHT); //TODO - replace hardcoding
	glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//Experiment with queues
	for (int i = 0; i < graphicQueue.size(); ++i) 
	{

		DrawData &data = graphicQueue[i];
		for (int j = 0; j < data.meshes->size(); ++j)
		{
			//BINDING
			graphicQueue[i].shader->UseShader();
			///graphicQueue[i].mesh->BindForDraw();
			(*data.meshes)[j]->BindForDraw();

			//UNIFORM BINDING
			graphicQueue[i].shader->setMat4f("model", graphicQueue[i].model);
			graphicQueue[i].shader->setMat4f("normalModel", graphicQueue[i].normalsModel);
			graphicQueue[i].shader->setTexture("diffuseTexture", graphicQueue[i].diffuseTexture, 0);
			graphicQueue[i].shader->setTexture("shadowMap", framebuffer->depthTexture, 1);

			//DRAW
			int faceCount = (*data.meshes)[j]->GetFaceCount();
			///int faceCount = graphicQueue[i].mesh->GetFaceCount();
			glDrawElements(GL_TRIANGLES, faceCount * 3, GL_UNSIGNED_INT, 0);

			//TODO - UNBIND SHADER AND MESH
			(*data.meshes)[j]->BindForDraw();
			///graphicQueue[i].mesh->UnbindForDraw();
			graphicQueue[i].shader->UnbindShader();
		}
	}
	
	//Experiment with queues
	for (int i = 0; i < graphicQueueAlpha.size(); ++i)
	{
		//EMPTY
	}
	//---------------------------------------------------------*/


	//Empty both graphics queues--------------------------------
	graphicQueue.clear();
	graphicQueueAlpha.clear();
}


void ForwardRenderer::initCamera()
{
	//Temporary, erase later somehow
	FPCamera = new Camera();

	//This is the one we will use
	currentCamera = FPCamera;
}


void ForwardRenderer::initFrameBuffers()
{
	framebuffer = new RenderTarget();
	RenderTargetDescriptor desc;
	desc.colorAttachmentCount = 0;
	desc.width = 1024;
	desc.height = 1024;
	desc.useStencil = false;
	desc.componentType = GL_UNSIGNED_BYTE;
	framebuffer->initFromDescriptor(desc);
}


//////////////////////////////////////
//////		SHADOWS				//////
//////////////////////////////////////
void ForwardRenderer::loadResources()
{
	//TODO - temporarily here
	//Shader
	shadowShader = new Shader("Shadow.vert", "Shadow.frag");
}

#include "../External/Includes/glm/gtc/matrix_transform.hpp"
#include "Affine.h"
void ForwardRenderer::CalculateLightProjView()
{
	//Light proj-view
	int width = 20;
	int height = 20;
	float ar = width / height;
	glm::mat4 lightView = AuxMath::view(sun.eye/*currentCamera->getEye()*/, sun.look/*currentCamera->getLook()*/, glm::vec4(0, 1, 0, 0));
	//glm::mat4 lightProj = AuxMath::orthographic(width, height, ar, sun.near, sun.far);
	glm::mat4 lightProj = glm::ortho(-15.0f, 15.0f, -15.0f, 15.0f, sun.near, sun.far);

	//Set the directional light vp matrix
	lightProjView = lightProj * lightView;
}



GLuint ForwardRenderer::GetTexture(std::string key)
{
	//First check if the map has the texture
	GLint mTexture = texturesDict[key];

	if (mTexture != 0) 
		return static_cast<GLuint>(mTexture);
	return -1;
}


GLuint ForwardRenderer::generateTextureFromSurface(SDL_Surface *surface, std::string key)
{
	//First check if the map has the texture
	GLint mTexture = texturesDict[key];

	if (mTexture == 0)
	{
		//Textures generation
		glGenTextures(1, textures + numberOfTexturesLoaded);
		glBindTexture(GL_TEXTURE_2D, textures[numberOfTexturesLoaded]);
		int mode = GL_RGB;
		if (surface->format->BytesPerPixel == 4)
		{
			mode = GL_RGBA;
			std::cout << "LOADING TEXTURE IS RGBA MODE" << std::endl;
		}

		glTexImage2D(GL_TEXTURE_2D, 0, mode, (*surface).w, (*surface).h, 0, mode, GL_UNSIGNED_BYTE, (*surface).pixels);
		///if (glGetError() != GL_NO_ERROR)
		///	std::cout << "PROBLEM IN LOADING TEXTURE" << std::endl;
		///else
		///	std::cout << "LOADING TEXTURE OK. surface: " << surface << ". Surface bytes per pixel: "
		///		<< static_cast<int>(surface->format->BytesPerPixel) << std::endl;

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		++numberOfTexturesLoaded;
		texturesDict[key] = textures[numberOfTexturesLoaded - 1];
		mTexture = texturesDict[key];
	}

	return mTexture;
}





////////////////////////////////////////
////	UNIFORM BUFFER OBJECTS		////
////////////////////////////////////////
//TODO - See extra cases for other data types (structs)

void ForwardRenderer::UniformBlockBind(GLuint ubo)
{
	glBindBuffer(GL_UNIFORM_BUFFER, this->ubo_test);
}

void ForwardRenderer::UniformBlockUnbind()
{
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}


//This is where basic types (int, float, bool) fall into (4 bytes size)
template<typename T>
void ForwardRenderer::UniformBlockPassData(GLuint ubo, int begin, T data)
{
	//std::cout << "UniformBlockData - Simple type" << std::endl;

	//glBindBuffer(GL_UNIFORM_BUFFER, this->ubo_test);
	glBufferSubData(GL_UNIFORM_BUFFER, begin, 4, data);
	//glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

//Arrays of both literals and vecs or mats should fall here
template<typename T>
void ForwardRenderer::UniformBlockPassData(GLuint ubo, int begin, int count, T *data)
{
	//std::cout << "UniformBlockData - Array of something" << std::endl;

	int size = count * 16;
	//glBindBuffer(GL_UNIFORM_BUFFER, this->ubo_test);
	glBufferSubData(GL_UNIFORM_BUFFER, begin, size, data);
	//glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void ForwardRenderer::UniformBlockPassData(GLuint ubo, int begin, glm::vec2& data)
{
	//glBindBuffer(GL_UNIFORM_BUFFER, this->ubo_test);
	glBufferSubData(GL_UNIFORM_BUFFER, begin, 8, &data[0]);
	//glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void ForwardRenderer::UniformBlockPassData(GLuint ubo, int begin, glm::vec3& data)
{
	//glBindBuffer(GL_UNIFORM_BUFFER, this->ubo_test);
	glBufferSubData(GL_UNIFORM_BUFFER, begin, 16, &data[0]);
	//glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void ForwardRenderer::UniformBlockPassData(GLuint ubo, int begin, glm::vec4& data)
{
	//glBindBuffer(GL_UNIFORM_BUFFER, this->ubo_test);
	glBufferSubData(GL_UNIFORM_BUFFER, begin, 16, &data[0]);
	//glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void ForwardRenderer::UniformBlockPassData(GLuint ubo, int begin, glm::mat4& data)
{
	//glBindBuffer(GL_UNIFORM_BUFFER, this->ubo_test);
	glBufferSubData(GL_UNIFORM_BUFFER, begin, 64, &data[0][0]);
	//glBindBuffer(GL_UNIFORM_BUFFER, 0);
}