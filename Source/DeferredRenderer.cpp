/// HEADER STUFF

#include <iostream>
#include "DeferredRenderer.h"
#include "Stat.h"

#include "RenderTarget.h"
#include "Shader.h"
#include "Camera.h"

#include "Model.h"
#include "Mesh.h"
#include "Sphere.h"
#include "Quad.h"
#include <cstdlib>


#define DEBUGGING_PASS				0
#define KERNEL_FILTER_COUNT			100


DeferredRenderer::DeferredRenderer() : 
	numberOfTexturesLoaded(0)
{
	std::cout << "Renderer Constructor" << std::endl;
}


DeferredRenderer::~DeferredRenderer()
{
	//TODO delete stuff
	delete GeometryBuffer;
	delete ShadowBuffer;

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
	delete DirectionalLightShader;
	delete LineShader;

	//Delete compute shader
	delete blurShaderHoriz;
	delete blurShaderVert;

	//Delete the FBO's
	glDeleteBuffers(1, &this->ubo_weights);
	glDeleteBuffers(1, &this->ubo_test);
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
	int index = 1;
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



	// FIRST UNIFORM BUFFER OBJECT (layout)-----------------------
	index = 2;
	// float array					// 0   - 16 * KERNEL_FILTER_COUNT
	
	// 1- Generate buffer and bind
	glGenBuffers(1, &this->ubo_weights);
	glBindBuffer(GL_UNIFORM_BUFFER, this->ubo_weights);
	//Allocate gpu space
	int size = 16 * KERNEL_FILTER_COUNT;
	glBufferData(GL_UNIFORM_BUFFER, size, NULL, GL_STATIC_DRAW);
	//Map to an index
	glBindBufferBase(GL_UNIFORM_BUFFER, index, this->ubo_weights);

	//Unbind crap
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
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
	glm::mat4 projView = currentCamera->getProj() * currentCamera->getView();
	UniformBlockBind(this->ubo_test);
	UniformBlockPassData(this->ubo_test, 0, projView);
	UniformBlockPassData(this->ubo_test, 64, lightProjView);
	UniformBlockPassData(this->ubo_test, 128, currentCamera->getEye());
	UniformBlockUnbind();


	//*GEOMETRY PASS
	GeometryPass();


	//AMBIENT LIGHT PASS (Back to default framebuffer)
	AmbientLightPass();


	//SHADOW MAP PASS
	FilteredShadowPass();
	

	//MULTIPLE POINT LIGHT PASS (Back to default framebuffer)
	MultiplePointLightPass(projView);


	//BONE DEBUG SHIT
	///glDisable(GL_DEPTH_TEST);
	///for (int i = 0; i < graphicQueue.size(); ++i)
	///	if (graphicQueue[i].boneCount > 0) 
	///		DebugDrawSkeleton(graphicQueue[i]);


	//Empty both graphics queues
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
		//Get the current drawNode
		DrawData &data = graphicQueue[i];
	
		//Since a node has a model (and all the meshes of a model for now share bones), we pass the uniform array
		if (graphicQueue[i].BoneTransformations)
			geometryPassShader->setMat4fArray("BoneTransf", 100, (*(graphicQueue[i].BoneTransformations))[0]);

		for (int j = 0; j < data.meshes->size(); ++j) 
		{
			//BINDING
			(*data.meshes)[j]->BindForDraw();

			//UNIFORM BINDING
			geometryPassShader->setMat4f("model", graphicQueue[i].model);
			geometryPassShader->setMat4f("normalModel", graphicQueue[i].normalsModel);
			geometryPassShader->setTexture("diffuseTexture", graphicQueue[i].diffuseTexture, 0);

			//DRAW
			int faceCount = (*data.meshes)[j]->GetFaceCount();
			glDrawElements(GL_TRIANGLES, faceCount * 3, GL_UNSIGNED_INT, 0);

			//TODO - UNBIND SHADER AND MESH
			(*data.meshes)[j]->UnbindForDraw();
		}
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



void DeferredRenderer::FilteredShadowPass()
{
	//GENERATE THE SHADOW MAP--------------------------------------------
	ShadowBuffer->Bind();
	glViewport(0, 0, ShadowBuffer->getWidth(), ShadowBuffer->getHeight());
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//DEPTH
	glEnable(GL_DEPTH_TEST);

	//glEnable(GL_BLEND);
	//glBlendFunc(GL_ONE, GL_ONE_MINUS_DST_COLOR);

	//Opaque objects queue
	for (int i = 0; i < graphicQueue.size(); ++i)
	{
		//BINDING
		shadowShader->UseShader();

		//Since a node has a model (and all the meshes of a model for now share bones), we pass the uniform array
		if (graphicQueue[i].BoneTransformations)
			shadowShader->setMat4fArray("BoneTransf", 100, (*(graphicQueue[i].BoneTransformations))[0]);

		DrawData &data = graphicQueue[i];
		for (int j = 0; j < data.meshes->size(); ++j)
		{
			//BINDING
			(*data.meshes)[j]->BindForDraw();

			//UNIFORM BINDING
			shadowShader->setMat4f("projView", lightProjView);
			shadowShader->setMat4f("model", graphicQueue[i].model);
			
			//DRAW
			int faceCount = (*data.meshes)[j]->GetFaceCount();
			glDrawElements(GL_TRIANGLES, faceCount * 3, GL_UNSIGNED_INT, 0);

			//TODO - UNBIND SHADER AND MESH
			(*data.meshes)[j]->UnbindForDraw();
		}

		shadowShader->UnbindShader();
	}


	
	
	//-----------------------------
	int width = ShadowBuffer->getWidth();
	int height = ShadowBuffer->getHeight();

	//Use compute shader
	this->blurShaderHoriz->UseShader();
	this->blurShaderHoriz->setImage("src", ShadowBuffer->texturesHandles[0], 0, GL_READ_ONLY, GL_RGBA32F);
	this->blurShaderHoriz->setImage("dst", ShadowBuffer->texturesHandles[1], 1, GL_WRITE_ONLY, GL_RGBA32F);
	this->blurShaderHoriz->setFloatArray("weights", kernelCount, &weights[0]);
	this->blurShaderHoriz->setInt("actualWeightCount", kernelCount);
	glDispatchCompute(width / 128, height, 1);

	//Use compute shader
	this->blurShaderVert->UseShader();
	this->blurShaderVert->setImage("src", ShadowBuffer->texturesHandles[1], 2, GL_READ_ONLY, GL_RGBA32F);
	this->blurShaderVert->setImage("dst", ShadowBuffer->texturesHandles[0], 3, GL_WRITE_ONLY, GL_RGBA32F);
	this->blurShaderVert->setFloatArray("weights", kernelCount, &weights[0]);
	this->blurShaderVert->setInt("actualWeightCount", kernelCount);
	glDispatchCompute(width, height / 128, 1);
	
	this->blurShaderVert->UnbindShader();
	//--------------------



	//NOW DRAW INTO THE SCENE---------------------------------------------
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
	//blending and depth
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	glDisable(GL_DEPTH_TEST);
	//USE SHADER AND BIND VAO
	DirectionalLightShader->UseShader();
	FSQ->BindForDraw();
	//UNIFORM BINDING
	DirectionalLightShader->setTexture("GBufferPos", GeometryBuffer->texturesHandles[0], 0);
	DirectionalLightShader->setTexture("GBufferNormals", GeometryBuffer->texturesHandles[1], 1);
	DirectionalLightShader->setTexture("GBufferSpecGloss", GeometryBuffer->texturesHandles[3], 2);
	DirectionalLightShader->setTexture("shadowMoments", ShadowBuffer->texturesHandles[0], 3);
	//Directional light uniforms
	DirectionalLightShader->setVec4f("lightLook", sun.look.x, sun.look.y, sun.look.z, 0.0f);
	DirectionalLightShader->setVec3f("lightColor", sun.color.r, sun.color.g, sun.color.b);
	DirectionalLightShader->setFloat("Intensity", sun.color.a);
	DirectionalLightShader->setFloat("ShadowIntensity", sun.shadowIntensity);
	//DRAW
	int faceCount = FSQ->GetFaceCount();
	glDrawElements(GL_TRIANGLES, faceCount * 3, GL_UNSIGNED_INT, 0);
	//TODO - UNBIND SHADER AND MESH
	FSQ->UnbindForDraw();
	DirectionalLightShader->UnbindShader();
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

	//Comments refer to how it would be for normal shadow map
	ShadowBuffer = new RenderTarget();
	RenderTargetDescriptor desc2;
	desc2.colorAttachmentCount = 2;			// 0
	desc2.width = 1024;
	desc2.height = 1024;
	desc2.useStencil = false;
	desc2.componentType = GL_FLOAT;			// GL_UNSIGNED_BYTE
	desc2.imageInternalFormat = GL_RGBA32F;	// GL_RGBA
	desc2.imageFormat = GL_RGBA;			// GL_RGBA
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

	DeferredAmbientShader = new Shader("DeferredAmbient.vert", "DeferredAmbient.frag");

	DirectionalLightShader = new Shader("DirectionalLight.vert", "DirectionalLight.frag");
	DirectionalLightShader->BindUniformBlock("test_gUBlock", 1);

	DeferredPointLightShader = new Shader("PointLightFSQ.vert", "PointLightFSQ.frag");
	DeferredPointLightShader->BindUniformBlock("test_gUBlock", 1);

	LineShader = new Shader("Line.vert", "Line.frag");
	LineShader->BindUniformBlock("test_gUBlock", 1);

	//Compute shader creation
	blurShaderHoriz = new Shader("BlurHoriz.comp");
	blurShaderVert = new Shader("BlurVert.comp");

	//Set weights
	SetKernelCount(5);

	//FSQ
	FSQ = new Quad();
	std::string const abs_path_prefix = "C:\\Users\\Jose\\Desktop\\OpenGl_Framework\\Assets\\Models\\";
	model = new Model(abs_path_prefix + "Sphere.fbx");
	PointLightSphere = this->model->meshes[0];

	//Many lights
	int range = 0; //44 for near 2000
	for (int i = -(range/2); i < (range / 2); ++i)
	{
		for (int j = -(range / 2); j < (range / 2); ++j)
		{
			float light_separation = 5.0f;
			Light_Positions[lightCount] = glm::vec4(light_separation * i, 0, light_separation * j, 1);
			Light_Colors[lightCount] = glm::vec4(static_cast<float>(rand()) / RAND_MAX, static_cast<float>(rand()) / RAND_MAX, static_cast<float>(rand()) / RAND_MAX, 1);
			Light_Radius[lightCount++] = 5.0f +(rand() % 10);
		}
	}
}

#include "../External/Includes/glm/gtc/matrix_transform.hpp"
#include "Affine.h"
void DeferredRenderer::CalculateLightProjView()
{
	//Light proj-view
	float halfWidth = sun.width/2.f;
	float halfHeight = sun.height/2.f;
	glm::mat4 lightView = AuxMath::view(sun.eye/*currentCamera->getEye()*/, sun.look/*currentCamera->getLook()*/, glm::vec4(0, 1, 0, 0));
	//glm::mat4 lightProj = AuxMath::orthographic(width, height, ar, sun.near, sun.far);
	glm::mat4 lightProj = glm::ortho(-halfWidth, halfWidth, -halfHeight, halfHeight, sun.near, sun.far);

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
	glBindBuffer(GL_UNIFORM_BUFFER, ubo);// this->ubo_test);
}

void DeferredRenderer::UniformBlockUnbind()
{
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}


//This is where basic types (int, float, bool) fall into (4 bytes size)
template<typename T>
void DeferredRenderer::UniformBlockPassData(GLuint ubo, int begin, T data)
{
	glBufferSubData(GL_UNIFORM_BUFFER, begin, 4, data);
}

//Arrays of both literals and vecs or mats should fall here
template<typename T>
void DeferredRenderer::UniformBlockPassData(GLuint ubo, int begin, int count, T *data)
{
	int size = count * 16;
	glBufferSubData(GL_UNIFORM_BUFFER, begin, size, data);
}

void DeferredRenderer::UniformBlockPassData(GLuint ubo, int begin, glm::vec2& data)
{
	glBufferSubData(GL_UNIFORM_BUFFER, begin, 8, &data[0]);
}

void DeferredRenderer::UniformBlockPassData(GLuint ubo, int begin, glm::vec3& data)
{
	glBufferSubData(GL_UNIFORM_BUFFER, begin, 16, &data[0]);
}

void DeferredRenderer::UniformBlockPassData(GLuint ubo, int begin, glm::vec4& data)
{
	glBufferSubData(GL_UNIFORM_BUFFER, begin, 16, &data[0]);
}

void DeferredRenderer::UniformBlockPassData(GLuint ubo, int begin, glm::mat4& data)
{
	glBufferSubData(GL_UNIFORM_BUFFER, begin, 64, &data[0][0]);
}



//BONE DEBUG DRAWING
void DeferredRenderer::DebugDrawSkeleton(DrawData const& data)
{
	//Loop through the bones and draw them
	for (int i = 0; i < data.boneCount; ++i)
	{
		float vertices[8] =
		{
			0.0f, 0.0f, 0.0f, 1.0f,
			0.0f, 1000.0f, 0.0f, 1.0f
		};

		GLuint lineVAO, LineVBO = static_cast<unsigned>(-1);
		glGenVertexArrays(1, &lineVAO);
		glGenBuffers(1, &LineVBO);
		LineShader->UseShader();
		int num_verts = 2;

		glBindVertexArray(lineVAO);
		glBindBuffer(GL_ARRAY_BUFFER, LineVBO);
		glBufferData(GL_ARRAY_BUFFER, num_verts * 4 * sizeof(float), vertices, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 4, GL_FLOAT, false, 4 * sizeof(GLfloat), (void*)0);
		glEnableVertexAttribArray(0);


		/////////////////////////////////////////////////////////////////
		glm::mat4 const& boneTransformation = (*data.BoneTransformations)[i];
		LineShader->setMat4f("boneTransform", boneTransformation);
		LineShader->setMat4f("model", data.model);
		glDrawArrays(GL_LINES, 0, 2);
		/////////////////////////////////////////////////////////////////

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		LineShader->UnbindShader();
		glDeleteVertexArrays(1, &lineVAO);
		glDeleteBuffers(1, &LineVBO);
	}
}