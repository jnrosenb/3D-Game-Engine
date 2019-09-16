/// HEADER STUFF

#include <iostream>
#include "DeferredRenderer.h"

#include "RenderTarget.h"
#include "Shader.h"
#include "Camera.h"

#include "Model.h"
#include "Sphere.h"
#include "Quad.h"
#include <cstdlib>



DeferredRenderer::DeferredRenderer() : 
	numberOfTexturesLoaded(0)
{
	std::cout << "Renderer Constructor" << std::endl;
}


DeferredRenderer::~DeferredRenderer()
{
	//TODO delete stuff
	delete GeometryBuffer;

	//Delete all the textures allocated by opengl
	glDeleteTextures(numberOfTexturesLoaded, textures);
	texturesDict.clear();
	numberOfTexturesLoaded = 0;

	//TODO - for now
	delete FSQ;
	delete model;

	//Shadow related stuff
	delete shadowShader;
	delete geometryPassShader;
	delete FSQShader;
	delete DeferredPointLightShader;
	delete DeferredAmbientShader;
}


void DeferredRenderer::init()
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


void DeferredRenderer::initUniformBufferObjects()
{
	// FIRST UNIFORM BUFFER OBJECT (layout)-----------------------
	int index = 0;

	// mat4						    // 0   - 64
	// mat4						    // 64   -128
	// vec4 eye						// 128  -144

	// 1- Generate buffer and bind
	glGenBuffers(1, &this->ubo_test);
	glBindBuffer(GL_UNIFORM_BUFFER, this->ubo_test);
	
	//Allocate gpu space
	glBufferData(GL_UNIFORM_BUFFER, 144, NULL, GL_STATIC_DRAW);
	
	//Map to an index
	glBindBufferBase(GL_UNIFORM_BUFFER, index, this->ubo_test);
	
	//Unbind
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	//------------------------------------------------------------
}


//Data that will be drawn into the scene
void DeferredRenderer::QueueForDraw(DrawData& data)
{
	graphicQueue.push_back(data);
}

void DeferredRenderer::QueueForDrawAlpha(DrawData& data)
{
	//TODO
}


void DeferredRenderer::Update(float dt)
{
	currentCamera->Update(dt);
}


void DeferredRenderer::Draw()
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
	UniformBlockUnbind();


	//*GEOMETRY PASS
	GeometryPass();


	/*DEBUGGING PASS (Back to default framebuffer)-----------------------------
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
	glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	FSQShader->UseShader();
	FSQ->BindForDraw();

	//UNIFORM BINDING
	FSQShader->setTexture("GBufferPos", GeometryBuffer->texturesHandles[0], 0);
	FSQShader->setTexture("GBufferNormals", GeometryBuffer->texturesHandles[1], 1);
	FSQShader->setTexture("GBufferDiffuse", GeometryBuffer->texturesHandles[2], 2);
	FSQShader->setTexture("GBufferSpecGloss", GeometryBuffer->texturesHandles[3], 3);

	//DRAW
	int faceCount = FSQ->GetFaceCount();
	glDrawElements(GL_TRIANGLES, faceCount * 3, GL_UNSIGNED_INT, 0);

	//TODO - UNBIND SHADER AND MESH
	FSQ->UnbindForDraw();
	FSQShader->UnbindShader();
	//-------------------------------------------------------------------------*/
	

	//AMBIENT LIGHT PASS (Back to default framebuffer)
	AmbientLightPass();


	//SHADOW MAP PASS
	ShadowMapPass(); // TODO - fill it out
	

	//MULTIPLE POINT LIGHT PASS (Back to default framebuffer)
	MultiplePointLightPass(projView);


	//Empty both graphics queues--------------------------------
	graphicQueue.clear();
	graphicQueueAlpha.clear();
}


void DeferredRenderer::GeometryPass()
{
	//BIND RENDER TARGET
	GeometryBuffer->Bind();

	//DEPTH
	glEnable(GL_DEPTH_TEST);

	//Blending
	glDisable(GL_BLEND);

	//CULLING
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	glViewport(0, 0, GeometryBuffer->getWidth(), GeometryBuffer->getHeight()); //TODO - Change test values of width and height
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	geometryPassShader->UseShader();
	for (int i = 0; i < graphicQueue.size(); ++i)
	{
		//BINDING
		graphicQueue[i].mesh->BindForDraw();

		//UNIFORM BINDING
		geometryPassShader->setMat4f("model", graphicQueue[i].model);
		geometryPassShader->setMat4f("normalModel", graphicQueue[i].normalsModel);
		geometryPassShader->setTexture("diffuseTexture", graphicQueue[i].diffuseTexture, 0);

		//DRAW
		int faceCount = graphicQueue[i].mesh->GetFaceCount();
		glDrawElements(GL_TRIANGLES, faceCount * 3, GL_UNSIGNED_INT, 0);

		//TODO - UNBIND SHADER AND MESH
		graphicQueue[i].mesh->UnbindForDraw();
	}
	geometryPassShader->UnbindShader();
}


void DeferredRenderer::AmbientLightPass()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//DEPTH
	glDisable(GL_DEPTH_TEST);

	DeferredAmbientShader->UseShader();
	FSQ->BindForDraw();

	//UNIFORM BINDING
	DeferredAmbientShader->setTexture("GBufferDiffuse", GeometryBuffer->texturesHandles[2], 2);

	//DRAW
	int faceCount = FSQ->GetFaceCount();
	glDrawElements(GL_TRIANGLES, faceCount * 3, GL_UNSIGNED_INT, 0);

	//TODO - UNBIND SHADER AND MESH
	FSQ->UnbindForDraw();
	DeferredAmbientShader->UnbindShader();
}


void DeferredRenderer::ShadowMapPass()
{
	ShadowBuffer->Bind();
	glViewport(0, 0, ShadowBuffer->getWidth(), ShadowBuffer->getHeight());
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//Experiment with queues
	for (int i = 0; i < graphicQueue.size(); ++i)
	{
		//BINDING
		shadowShader->UseShader();
		graphicQueue[i].mesh->BindForDraw();

		//UNIFORM BINDING
		shadowShader->setMat4f("projView", lightProjView);
		shadowShader->setMat4f("model", graphicQueue[i].model);

		//DRAW
		int faceCount = graphicQueue[i].mesh->GetFaceCount();
		glDrawElements(GL_TRIANGLES, faceCount * 3, GL_UNSIGNED_INT, 0);

		//TODO - UNBIND SHADER AND MESH
		graphicQueue[i].mesh->UnbindForDraw();
		shadowShader->UnbindShader();
	}
}


void DeferredRenderer::MultiplePointLightPass(glm::mat4& projView) //Later pass this on uniform buffer
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

	//BLENDING
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);

	//DEPTH
	glDisable(GL_DEPTH_TEST);

	//CULLING
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);

	//USE SHADER AND BIND VAO
	DeferredPointLightShader->UseShader();
	PointLightSphere->BindForDraw();

	//UNIFORM BINDING (common to all lights)
	DeferredPointLightShader->setTexture("GBufferPos", GeometryBuffer->texturesHandles[0], 0);
	DeferredPointLightShader->setTexture("GBufferNormals", GeometryBuffer->texturesHandles[1], 1);
	DeferredPointLightShader->setTexture("GBufferDiffuse", GeometryBuffer->texturesHandles[2], 2);
	DeferredPointLightShader->setTexture("GBufferSpecGloss", GeometryBuffer->texturesHandles[3], 3);
	DeferredPointLightShader->setMat4f("projViewMatrix", projView);

	glm::mat4 pointLightModel = glm::mat4(1);
	for (int i = 0; i < lightCount; ++i)
	{
		//UNIFORM BINDING
		DeferredPointLightShader->setVec4f("lightWorldPos", Light_Positions[i].x, Light_Positions[i].y, Light_Positions[i].z, 1.0f);
		DeferredPointLightShader->setVec3f("lightColor", Light_Colors[i].r, Light_Colors[i].g, Light_Colors[i].b);
		pointLightModel[3] = Light_Positions[i];
		pointLightModel[0][0] = Light_Radius[i];
		pointLightModel[1][1] = Light_Radius[i];
		pointLightModel[2][2] = Light_Radius[i];
		DeferredPointLightShader->setMat4f("lightModel", pointLightModel);
		DeferredPointLightShader->setFloat("radius", Light_Radius[i]);

		//DRAW
		int faceCount = PointLightSphere->GetFaceCount();
		glDrawElements(GL_TRIANGLES, faceCount * 3, GL_UNSIGNED_INT, 0);
	}

	//TODO - UNBIND SHADER AND MESH
	PointLightSphere->UnbindForDraw();
	DeferredPointLightShader->UnbindShader();
}


void DeferredRenderer::initCamera()
{
	//Temporary, erase later somehow
	FPCamera = new Camera();

	//This is the one we will use
	currentCamera = FPCamera;
}


void DeferredRenderer::initFrameBuffers()
{
	GeometryBuffer = new RenderTarget();
	RenderTargetDescriptor desc;
	desc.colorAttachmentCount = 4;
	desc.width = VIEWPORT_WIDTH;
	desc.height = VIEWPORT_HEIGHT;
	desc.useStencil = false;
	desc.componentType = GL_FLOAT;
	desc.imageInternalFormat = GL_RGBA32F;
	desc.imageFormat = GL_RGBA;
	GeometryBuffer->initFromDescriptor(desc);

	ShadowBuffer = new RenderTarget();
	RenderTargetDescriptor desc2;
	desc2.colorAttachmentCount = 0;
	desc2.width = 1280;
	desc2.height = 1280;
	desc2.useStencil = false;
	desc2.componentType = GL_UNSIGNED_BYTE;
	ShadowBuffer->initFromDescriptor(desc2);
}


//////////////////////////////////////
//////		RANDOM STUFF		//////
//////////////////////////////////////
void DeferredRenderer::loadResources()
{
	shadowShader = new Shader("Shadow.vert", "Shadow.frag");

	geometryPassShader = new Shader("GeometryPass.vert", "GeometryPass.frag");
	geometryPassShader->BindUniformBlock("test_gUBlock", 1);
	
	FSQShader = new Shader("FSQ.vert", "FSQ.frag");
	DeferredPointLightShader = new Shader("PointLightFSQ.vert", "PointLightFSQ.frag");
	DeferredAmbientShader = new Shader("DeferredAmbient.vert", "DeferredAmbient.frag");

	//FSQ
	FSQ = new Quad();
	std::string const abs_path_prefix = "C:\\Users\\Jose\\Desktop\\OpenGl_Framework\\Assets\\Models\\";
	model = new Model(abs_path_prefix + "Sphere.fbx");
	PointLightSphere = this->model->meshes[0];

	//Many lights
	int range = 30; //44 for near 2000
	for (int i = -(range/2); i < (range / 2); ++i)
	{
		for (int j = -(range / 2); j < (range / 2); ++j)
		{
			float light_separation = 5.0f;
			Light_Positions[lightCount] = glm::vec4(light_separation * i, 0, light_separation * j, 1);
			Light_Colors[lightCount] = glm::vec4(static_cast<float>(rand()) / RAND_MAX, static_cast<float>(rand()) / RAND_MAX, static_cast<float>(rand()) / RAND_MAX, 1);
			Light_Radius[lightCount++] = 10.0f +(rand() % 30);
		}
	}
}

#include "../External/Includes/glm/gtc/matrix_transform.hpp"
#include "Affine.h"
void DeferredRenderer::CalculateLightProjView()
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



GLuint DeferredRenderer::GetTexture(std::string key)
{
	//First check if the map has the texture
	GLint mTexture = texturesDict[key];

	if (mTexture != 0) 
		return static_cast<GLuint>(mTexture);
	return -1;
}


GLuint DeferredRenderer::generateTextureFromSurface(SDL_Surface *surface, std::string key)
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

void DeferredRenderer::UniformBlockBind(GLuint ubo)
{
	glBindBuffer(GL_UNIFORM_BUFFER, this->ubo_test);
}

void DeferredRenderer::UniformBlockUnbind()
{
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}


//This is where basic types (int, float, bool) fall into (4 bytes size)
template<typename T>
void DeferredRenderer::UniformBlockPassData(GLuint ubo, int begin, T data)
{
	//std::cout << "UniformBlockData - Simple type" << std::endl;

	//glBindBuffer(GL_UNIFORM_BUFFER, this->ubo_test);
	glBufferSubData(GL_UNIFORM_BUFFER, begin, 4, data);
	//glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

//Arrays of both literals and vecs or mats should fall here
template<typename T>
void DeferredRenderer::UniformBlockPassData(GLuint ubo, int begin, int count, T *data)
{
	//std::cout << "UniformBlockData - Array of something" << std::endl;

	int size = count * 16;
	//glBindBuffer(GL_UNIFORM_BUFFER, this->ubo_test);
	glBufferSubData(GL_UNIFORM_BUFFER, begin, size, data);
	//glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void DeferredRenderer::UniformBlockPassData(GLuint ubo, int begin, glm::vec2& data)
{
	//glBindBuffer(GL_UNIFORM_BUFFER, this->ubo_test);
	glBufferSubData(GL_UNIFORM_BUFFER, begin, 8, &data[0]);
	//glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void DeferredRenderer::UniformBlockPassData(GLuint ubo, int begin, glm::vec3& data)
{
	//glBindBuffer(GL_UNIFORM_BUFFER, this->ubo_test);
	glBufferSubData(GL_UNIFORM_BUFFER, begin, 16, &data[0]);
	//glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void DeferredRenderer::UniformBlockPassData(GLuint ubo, int begin, glm::vec4& data)
{
	//glBindBuffer(GL_UNIFORM_BUFFER, this->ubo_test);
	glBufferSubData(GL_UNIFORM_BUFFER, begin, 16, &data[0]);
	//glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void DeferredRenderer::UniformBlockPassData(GLuint ubo, int begin, glm::mat4& data)
{
	//glBindBuffer(GL_UNIFORM_BUFFER, this->ubo_test);
	glBufferSubData(GL_UNIFORM_BUFFER, begin, 64, &data[0][0]);
	//glBindBuffer(GL_UNIFORM_BUFFER, 0);
}