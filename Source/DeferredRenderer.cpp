/// HEADER STUFF

#include "ResourceManager.h"
#include "DeferredRenderer.h"
#include <iostream>

#include "RenderTarget.h"
#include "Camera.h"

#include <limits>
#include "Model.h"
#include "Polar.h"
#include "Sphere.h"
#include "Quad.h"
#include <cstdlib>
#include "Math/Stat.h"


#include "../External/Includes/glm/gtc/matrix_transform.hpp"
#include "Affine.h"


#define DEBUGGING_PASS				0
#define PI							3.1415926535f
#define KERNEL_FILTER_COUNT			100				//Arbitrary number
#define AO_KERNEL_COUNT				50				//Arbitrary number
#define MAX_SPECULAR_IBL_SAMPLES	64				//Arbitrary number
#define SPECULAR_SAMPLES			20				//Arbitrary number
#define CASCADES					3				//NOT BEING USED


DeferredRenderer::DeferredRenderer() :
	numberOfTexturesLoaded(0)
{
	std::cout << "Renderer Constructor" << std::endl;

	//Draw the skin, not the bones
	DrawSkin = true;
	DrawSkeleton = false;
	InDebugView = false;
	visualCascadesVisible = false;
}


DeferredRenderer::~DeferredRenderer()
{
	//TODO delete stuff
	delete GeometryBuffer;
	delete ShadowBuffer;
	delete AOBuffer;
	//Temporary, for cascaded
	delete CascadedBuffer01;
	delete CascadedBuffer02;
	delete CascadedBuffer03;

	//Delete all the textures allocated by opengl
	glDeleteTextures(numberOfTexturesLoaded, textures);
	texturesDict.clear();
	numberOfTexturesLoaded = 0;

	//Delete camera
	if (mainCamera)
		delete mainCamera;
	if (debugCamera)
		delete debugCamera;

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
	delete geometryInstancedShader;
	delete LineShader;
	delete IBLShader;
	delete AOShader;
	delete momentShadowShader;
	delete DirectionalLightCascadedShader;

	//Delete compute shader
	delete blurShaderHoriz;
	delete blurShaderVert;
	delete blurAOHoriz;
	delete blurAOVert;

	//Delete blocm with low discrepancy random points
	delete specularBlock;

	//Delete the FBO's
	glDeleteBuffers(1, &this->ubo_weights);
	glDeleteBuffers(1, &this->ubo_test);
	glDeleteBuffers(1, &this->ubo_IBLSpecular);
}


void DeferredRenderer::init()
{

	////////////////////////////
	////	INITIALIZERS	////
	////////////////////////////

	initFrameBuffers();
	initUniformBufferObjects();
	initCascadedParams();
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
	// mat4						    // 64  - 128
	// vec4 eye						// 128 - 144
	//int width						// 144 - 148
	//int height					// 148 - 152 (trying with 160)

	// 1- Generate buffer and bind
	glGenBuffers(1, &this->ubo_test);
	glBindBuffer(GL_UNIFORM_BUFFER, this->ubo_test);
	//Allocate gpu space
	glBufferData(GL_UNIFORM_BUFFER, 160, NULL, GL_STATIC_DRAW);//Changed because of renderdoc weird warning. was 152
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



	// IBL SPECULAR LOW DISCREPANCY PAIRS (layout)-----------------------
	index = 0;
	// int					// 0   -  16
	// vec2 array[100]		// 16  -  4 + 16 * MAX_SPECULAR_IBL_SAMPLES

	// 1- Generate buffer and bind
	glGenBuffers(1, &this->ubo_IBLSpecular);
	glBindBuffer(GL_UNIFORM_BUFFER, this->ubo_IBLSpecular);
	//Allocate gpu space
	size = 16 + 16 * MAX_SPECULAR_IBL_SAMPLES;
	glBufferData(GL_UNIFORM_BUFFER, size, NULL, GL_STATIC_DRAW);
	//Map to an index
	glBindBufferBase(GL_UNIFORM_BUFFER, index, this->ubo_IBLSpecular);


	//Unbind crap
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}


//Data that will be drawn into the scene
void DeferredRenderer::QueueForDraw(DrawData& data)
{
	graphicQueue.push_back(data);
}


void DeferredRenderer::QueueForDrawInstanced(DrawInstanceData& data)
{
	//Add instanced stuff to VAO
	for (int j = 0; j < data.meshes->size(); ++j)
	{
		(*data.meshes)[j]->BindVAO();

		int size = data.instanceCount * sizeof(glm::mat4);
		glBindBuffer(GL_ARRAY_BUFFER, data.TempVBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, size, &(*data.modelMatrices)[0][0][0]);

		glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
		glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
		glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2*sizeof(glm::vec4)));
		glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3*sizeof(glm::vec4)));
		
		glEnableVertexAttribArray(3);
		glEnableVertexAttribArray(4);
		glEnableVertexAttribArray(5);
		glEnableVertexAttribArray(6);

		glVertexAttribDivisor(3, 1);
		glVertexAttribDivisor(4, 1);
		glVertexAttribDivisor(5, 1);
		glVertexAttribDivisor(6, 1);

		(*data.meshes)[j]->UnbindVAO();
	}

	instanceQueue.push_back(data);
}


void DeferredRenderer::QueueForDrawAlpha(DrawData& data)
{
	graphicQueueAlpha.push_back(data);
}


void DeferredRenderer::QueueForDebugDraw(DrawDebugData& data) 
{
	debugQueue.push_back(data);
}


void DeferredRenderer::Update(float dt)
{
	currentCamera->Update(dt);
}


void DeferredRenderer::Draw()
{
	//CALCULATE THE DIRECTIONAL LIGHT MATRICES
	CalculateLightProjView();
	GenerateCascadedFustrums();
	int width = VIEWPORT_WIDTH;
	int height = VIEWPORT_HEIGHT;

	//TODO - Better way to pass. Uniform block should know its own indices or calculate them
	glm::mat4 projView = currentCamera->getProj() * currentCamera->getView();
	UniformBlockBind(this->ubo_test);
	UniformBlockPassData(this->ubo_test, 0, projView);
	UniformBlockPassData(this->ubo_test, 64, lightProjView);
	UniformBlockPassData(this->ubo_test, 128, currentCamera->getEye());
	UniformBlockPassData(this->ubo_test, 144, &width);
	UniformBlockPassData(this->ubo_test, 148, &height);
	//Passing the IBL specular low discrepancy pairs
	UniformBlockBind(this->ubo_IBLSpecular);
	UniformBlockPassData(this->ubo_IBLSpecular, 0, &(specularBlock->N));
	UniformBlockPassData(this->ubo_IBLSpecular, 16, specularBlock->N, &(specularBlock->pairs[0]));
	UniformBlockUnbind();


	//*GEOMETRY PASS
	GeometryPass();

	//Ambient Occlusion pass
	AmbientOcclusionPass();

	//AMBIENT LIGHT PASS (Back to default framebuffer)
	///AmbientLightPass();
	AmbientIBLPass();

	//Draw skydome
	SkydomePass();

	//SHADOW MAP PASS										   
	///FilteredShadowPass();									   
	CascadedShadowPass();
															   
	//MULTIPLE POINT LIGHT PASS (Back to default framebuffer)  
	MultiplePointLightPass(projView);						   												

	//DEBUG PASS FOR DEBUG VIEW (FUSTRUM, CASCADED)
	if (InDebugView) 
	{
		//Fake temporary debug call for cascaded fustrum
		DrawDebugView();
	}


	//Empty both graphics queues
	graphicQueue.clear();
	graphicQueueAlpha.clear();
	instanceQueue.clear();
	debugQueue.clear();
}


void DeferredRenderer::GeometryPass()
{
	//BIND RENDER TARGET
	GeometryBuffer->Bind();

	//DEPTH - BLEND - CULL
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	//VIEWPORT AND CLEAR
	glViewport(0, 0, GeometryBuffer->getWidth(), GeometryBuffer->getHeight()); //TODO - Change test values of width and height
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//Draw normal objects
	geometryPassShader->UseShader();
	for (int i = 0; i < graphicQueue.size(); ++i)
	{
		//Get the current drawNode
		DrawData &data = graphicQueue[i];

		//Since a node has a model (and all the meshes of a model for now share bones), we pass the uniform array
		if (graphicQueue[i].BoneTransformations)
		{
			geometryPassShader->setMat4fArray("BoneTransf", 100, (*(graphicQueue[i].BoneTransformations))[0]);
			if (!DrawSkin)
				continue;
		}

		for (int j = 0; j < data.meshes->size(); ++j)
		{
			//BINDING
			(*data.meshes)[j]->BindVAO();

			//SAMPLER UNIFORMS---------------------------------
			//Diffuse texture, should be optional 
			geometryPassShader->setTexture("diffuseTexture", data.diffuseTexture, 0);
			//Roughness and metallic
			if (data.metallic == -1.0f)
			{
				geometryPassShader->setTexture("metallicTexture", data.metallicTexture, 1);
			}
			if (data.roughness == -1.0f)
			{
				geometryPassShader->setTexture("roughnessTexture", data.roughnessTexture, 2);
			}
			//Normal map, should be optional
			geometryPassShader->setTexture("normalMap", data.normalMap, 3);

			//UNIFORM BINDING------------------------------------
			geometryPassShader->setMat4f("model", data.model);
			geometryPassShader->setMat4f("normalModel", data.normalsModel);
			geometryPassShader->setInt("useNormalMap", data.useNormalMap);
			geometryPassShader->setInt("useDiffuseTexture", data.useDiffuseTexture);
			geometryPassShader->setFloat("roughness", data.roughness);
			geometryPassShader->setFloat("metallic", data.metallic);
			geometryPassShader->setVec4f("specularColor", data.specularColor.r,
				data.specularColor.g, data.specularColor.b, data.specularColor.a);
			geometryPassShader->setVec4f("diffuseColor", data.diffuseColor.r,
				data.diffuseColor.g, data.diffuseColor.b, data.diffuseColor.a);
			geometryPassShader->setInt("xTiling", data.xTiling);
			geometryPassShader->setInt("yTiling", data.yTiling);

			//DRAW and unbind
			(*data.meshes)[j]->Draw();
			(*data.meshes)[j]->UnbindVAO();
		}
	}
	geometryPassShader->UnbindShader();

	//Transparent objects
	geometryPassShader->UseShader();
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	for (int i = 0; i < graphicQueueAlpha.size(); ++i)
	{
		//Get the current drawNode
		DrawData &data = graphicQueueAlpha[i];

		//Since a node has a model (and all the meshes of a model for now share bones), we pass the uniform array
		if (graphicQueueAlpha[i].BoneTransformations)
		{
			geometryPassShader->setMat4fArray("BoneTransf", 100, (*(graphicQueueAlpha[i].BoneTransformations))[0]);
			if (!DrawSkin)
				continue;
		}

		for (int j = 0; j < data.meshes->size(); ++j)
		{
			//BINDING
			(*data.meshes)[j]->BindVAO();

			//SAMPLER UNIFORMS---------------------------------
			//Diffuse texture, should be optional 
			geometryPassShader->setTexture("diffuseTexture", data.diffuseTexture, 0);
			//Roughness and metallic
			if (data.metallic == -1.0f)
			{
				geometryPassShader->setTexture("metallicTexture", data.metallicTexture, 1);
			}
			if (data.roughness == -1.0f)
			{
				geometryPassShader->setTexture("roughnessTexture", data.roughnessTexture, 2);
			}
			//Normal map, should be optional
			geometryPassShader->setTexture("normalMap", data.normalMap, 3);

			//UNIFORM BINDING------------------------------------
			geometryPassShader->setMat4f("model", data.model);
			geometryPassShader->setMat4f("normalModel", data.normalsModel);
			geometryPassShader->setInt("useNormalMap", data.useNormalMap);
			geometryPassShader->setInt("useDiffuseTexture", data.useDiffuseTexture);
			geometryPassShader->setFloat("roughness", data.roughness);
			geometryPassShader->setFloat("metallic", data.metallic);
			geometryPassShader->setVec4f("specularColor", data.specularColor.r,
				data.specularColor.g, data.specularColor.b, data.specularColor.a);
			geometryPassShader->setVec4f("diffuseColor", data.diffuseColor.r,
				data.diffuseColor.g, data.diffuseColor.b, data.diffuseColor.a);
			geometryPassShader->setInt("xTiling", data.xTiling);
			geometryPassShader->setInt("yTiling", data.yTiling);

			//DRAW and unbind
			(*data.meshes)[j]->Draw();
			(*data.meshes)[j]->UnbindVAO();
		}
	}
	glDisable(GL_BLEND);
	geometryPassShader->UnbindShader();

	//Draw instanced objects
	geometryInstancedShader->UseShader();
	for (int i = 0; i < instanceQueue.size(); ++i)
	{
		DrawInstanceData &data = instanceQueue[i];			
		for (int j = 0; j < data.meshes->size(); ++j)
		{
			(*data.meshes)[j]->BindVAO();

			//Textures
			geometryInstancedShader->setTexture("diffuseTexture", data.diffuseTexture, 0);
			//UNIFORM BINDING------------------------------------
			geometryInstancedShader->setInt("useDiffuseTexture", data.useDiffuseTexture);
			geometryInstancedShader->setVec4f("diffuseColor", data.diffuseColor.r,
				data.diffuseColor.g, data.diffuseColor.b, data.diffuseColor.a);
			geometryInstancedShader->setInt("xTiling", data.xTiling);
			geometryInstancedShader->setInt("yTiling", data.yTiling);

			//DRAW and unbind
			(*data.meshes)[j]->DrawInstanced(data.instanceCount);
			(*data.meshes)[j]->UnbindVAO();
		}
	}
	geometryInstancedShader->UnbindShader();

	//Draw debug shapes
	if (DrawDebugPass) 
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);
		for (int i = 0; i < debugQueue.size(); ++i)
		{
			//Get the current drawNode
			DrawDebugData &data = debugQueue[i];
			data.shader->UseShader();

			//Later for having animated OBB
			if (data.BoneTransformations)
				geometryPassShader->setMat4fArray("BoneTransf", 100, (*(data.BoneTransformations))[0]);

			//Later we could have each mesh have their own OBB. For now, one per rigidbody
			///for (int j = 0; j < data.meshes->size(); ++j)
			{
				//BINDING
				///(*data.meshes)[j]->BindForDraw();
				data.mesh->BindVAO();

				//UNIFORM BINDING
				data.shader->setMat4f("model", data.model);
				data.shader->setVec4f("diffuseColor", data.diffuseColor.r,
					data.diffuseColor.g, data.diffuseColor.b, data.diffuseColor.a);

				//DRAW and unbind
				data.mesh->Draw();
				data.mesh->UnbindVAO();
				///(*data.meshes)[j]->Draw();
				///(*data.meshes)[j]->UnbindVAO();
			}
			data.shader->UnbindShader();
		}
	}
}


void DeferredRenderer::AmbientOcclusionPass()
{
	//Why calculate if we wont use it
	if (useAO == false)
		return;

	//1ST PASS - WRITE INTO AO-BUFFER------------
	AOBuffer->Bind();
	glViewport(0, 0, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //Not really necessary

	AOShader->UseShader();
	FSQ->BindVAO();

	//UNIFORM BINDING
	AOShader->setTexture("GBufferPos", GeometryBuffer->texturesHandles[0], 0);
	AOShader->setTexture("GBufferNormals", GeometryBuffer->texturesHandles[1], 1);
	AOShader->setMat4f("view", this->currentCamera->getView());

	//DRAW
	FSQ->Draw();
	///int faceCount = FSQ->GetFaceCount();
	///glDrawElements(GL_TRIANGLES, faceCount * 3, GL_UNSIGNED_INT, 0);

	//TODO - UNBIND SHADER AND MESH
	FSQ->UnbindVAO();
	AOShader->UnbindShader();


	//COMPUTE SHADER PASS------------------------
	int width = AOBuffer->getWidth();
	int height = AOBuffer->getHeight();

	//Use compute shader
	this->blurAOHoriz->UseShader();
	this->blurAOHoriz->setImage("src", AOBuffer->texturesHandles[0], 0, GL_READ_ONLY, GL_R16F);
	this->blurAOHoriz->setImage("dst", AOBuffer->texturesHandles[1], 1, GL_WRITE_ONLY, GL_R16F);
	this->blurAOHoriz->setImage("GBufferPos", GeometryBuffer->texturesHandles[0], 2, GL_READ_ONLY, GL_RGBA32F);
	this->blurAOHoriz->setImage("GBufferNormals", GeometryBuffer->texturesHandles[1], 3, GL_READ_ONLY, GL_RGBA32F);
	this->blurAOHoriz->setMat4f("view", this->currentCamera->getView());
	this->blurAOHoriz->setFloatArray("weights", AOKernelCount, &AOweights[0]);
	this->blurAOHoriz->setInt("actualWeightCount", AOKernelCount);
	glDispatchCompute(width / 128, height, 1);
	
	//Use compute shader
	this->blurAOVert->UseShader();
	this->blurAOVert->setImage("src", AOBuffer->texturesHandles[1], 1, GL_READ_ONLY, GL_R16F);
	this->blurAOVert->setImage("dst", AOBuffer->texturesHandles[0], 0, GL_WRITE_ONLY, GL_R16F);
	this->blurAOVert->setImage("GBufferPos", GeometryBuffer->texturesHandles[0], 2, GL_READ_ONLY, GL_RGBA32F);
	this->blurAOVert->setImage("GBufferNormals", GeometryBuffer->texturesHandles[1], 3, GL_READ_ONLY, GL_RGBA32F);
	this->blurAOVert->setMat4f("view", this->currentCamera->getView());
	this->blurAOVert->setFloatArray("weights", AOKernelCount, &AOweights[0]);
	this->blurAOVert->setInt("actualWeightCount", AOKernelCount);
	glDispatchCompute(width, (height / 128) + 1, 1);
	
	//Unbind compute shader
	this->blurAOVert->UnbindShader();
}


void DeferredRenderer::AmbientLightPass()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);// | GL_DEPTH_BUFFER_BIT); //Not really necessary to

	//DEPTH
	glDisable(GL_DEPTH_TEST);

	//weird artifact with moments shadow map
	///glEnable(GL_BLEND);
	///glBlendFunc(GL_ONE, GL_ONE);

	DeferredAmbientShader->UseShader();
	FSQ->BindVAO();

	//UNIFORM BINDING
	DeferredAmbientShader->setTexture("GBufferDiffuse", GeometryBuffer->texturesHandles[2], 2);
	DeferredAmbientShader->setTexture("geoDepthBuffer", GeometryBuffer->depthTexture, 3);

	//AO Uniforms
	DeferredAmbientShader->setTexture("AOTexture", AOBuffer->texturesHandles[0], 4);
	DeferredAmbientShader->setInt("useAO", static_cast<int>(useAO));

	//DRAW
	FSQ->Draw();
	FSQ->UnbindVAO();
	DeferredAmbientShader->UnbindShader();
}


void DeferredRenderer::AmbientIBLPass()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);// | GL_DEPTH_BUFFER_BIT); //Not really necessary to

	//DEPTH
	glDisable(GL_DEPTH_TEST);

	//weird artifact with moments shadow map
	///glEnable(GL_BLEND);
	///glBlendFunc(GL_ONE, GL_ONE);

	//Shader and VAO BINDING
	IBLShader->UseShader();
	FSQ->BindVAO();

	//GBUFFER BINDINGS
	IBLShader->setTexture("GBufferPos", GeometryBuffer->texturesHandles[0], 0);
	IBLShader->setTexture("GBufferNormals", GeometryBuffer->texturesHandles[1], 1);
	IBLShader->setTexture("GBufferDiffuse", GeometryBuffer->texturesHandles[2], 2);
	IBLShader->setTexture("GBufferSpecGloss", GeometryBuffer->texturesHandles[3], 3);
	IBLShader->setTexture("AOTexture", AOBuffer->texturesHandles[0], 4);
	//SKYDOME BINDINGS
	IBLShader->setTexture("skyMap", currentCamera->GetSkydome()->texture, 5);
	IBLShader->setTexture("irradianceMap", currentCamera->GetSkydome()->irradiance, 6);
	IBLShader->setTexture("geoDepthBuffer", GeometryBuffer->depthTexture, 7);

	//Other uniforms
	IBLShader->setInt("maxMipmapLevel", currentCamera->GetSkydome()->specularMipmap);
	IBLShader->setInt("useAO", static_cast<int>(useAO));

	//DRAW
	FSQ->Draw();
	///int faceCount = FSQ->GetFaceCount();
	///glDrawElements(GL_TRIANGLES, faceCount * 3, GL_UNSIGNED_INT, 0);

	//TODO - UNBIND SHADER AND MESH
	FSQ->UnbindVAO();
	IBLShader->UnbindShader();
}


void DeferredRenderer::SkydomePass() 
{	
	//Pass depth as a texture to the thing
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE_MINUS_DST_ALPHA, GL_ONE);
	this->currentCamera->GetSkydome()->Draw(/*GeometryBuffer->depthTexture*/);
	glDisable(GL_BLEND);
}


void DeferredRenderer::CascadedShadowPass()
{
	//This is so the calculation for the cascaded only are done with respect to the main cam
	Camera *temp = currentCamera;
	currentCamera = mainCamera;

	//FOR EACH CASCADE---------------------------------------
	glEnable(GL_DEPTH_TEST);
	for (int cascade = 0; cascade < subdivissions; ++cascade) 
	{
		//1st PASS-------------------------------------------------------------------
		CascadedBuffers[cascade]->Bind();
		glViewport(0, 0, CascadedBuffers[cascade]->getWidth(), CascadedBuffers[cascade]->getHeight());
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glCullFace(GL_FRONT);

		//For each opaque objects
		for (int i = 0; i < graphicQueue.size(); ++i)
		{
			//BINDING
			shadowShader->UseShader();
			DrawData &data = graphicQueue[i];

			//Make non casting shadow being skipped
			if (data.castShadow == 0)
				continue;

			//Since a node has a model 
			//(and all the meshes of a model for now share bones):
			if (data.BoneTransformations)
				shadowShader->setMat4fArray("BoneTransf", 100, (*(data.BoneTransformations))[0]);

			//For each mesh in data.meshes
			for (int j = 0; j < data.meshes->size(); ++j)
			{
				(*data.meshes)[j]->BindVAO();

				//For each cascade, we need to pass a light view and light 
				//proj (different from the one in the uniform block)
				glm::mat4 cascadedProjView = C_LightPROJi[cascade] * C_lightView;
				///glm::mat4 cascadedProjView = C_LightPROJi[cascade] * C_LightVIEWi[cascade];
				shadowShader->setMat4f("LightProjView", cascadedProjView);
				shadowShader->setMat4f("model", data.model);

				///int faceCount = (*data.meshes)[j]->GetFaceCount();
				///glDrawElements(GL_TRIANGLES, faceCount * 3, GL_UNSIGNED_INT, 0);
				(*data.meshes)[j]->Draw();
				(*data.meshes)[j]->UnbindVAO();
			}
			shadowShader->UnbindShader();
		}
	}

	//2d PASS-------------------------------------------------------------------
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	glDisable(GL_DEPTH_TEST);
	glCullFace(GL_BACK);
	
	//USE SHADER AND BIND VAO
	DirectionalLightCascadedShader->UseShader();
	FSQ->BindVAO();
	
	//UNIFORM BINDING
	DirectionalLightCascadedShader->setTexture("GBufferPos", GeometryBuffer->texturesHandles[0], 0);
	DirectionalLightCascadedShader->setTexture("GBufferNormals", GeometryBuffer->texturesHandles[1], 1);
	DirectionalLightCascadedShader->setTexture("GBufferSpecGloss", GeometryBuffer->texturesHandles[3], 2);
	//Cascaded depth buffers
	DirectionalLightCascadedShader->setTexture("cascaded01", CascadedBuffers[0]->depthTexture, 3);
	DirectionalLightCascadedShader->setTexture("cascaded02", CascadedBuffers[1]->depthTexture, 4);
	DirectionalLightCascadedShader->setTexture("cascaded03", CascadedBuffers[2]->depthTexture, 5);
	//Other cascaded values
	std::vector<glm::mat4> CLightProjView;
	///CLightProjView.push_back(C_LightPROJi[0] * C_LightVIEWi[0]);
	///CLightProjView.push_back(C_LightPROJi[1] * C_LightVIEWi[1]);
	///CLightProjView.push_back(C_LightPROJi[2] * C_LightVIEWi[2]);
	CLightProjView.push_back(C_LightPROJi[0] * C_lightView);
	CLightProjView.push_back(C_LightPROJi[1] * C_lightView);
	CLightProjView.push_back(C_LightPROJi[2] * C_lightView);
	DirectionalLightCascadedShader->setMat4fArray("C_LPV", 3, CLightProjView[0]);
	DirectionalLightCascadedShader->setFloatArray("C_depths", 4, &Zi[0]);
	DirectionalLightCascadedShader->setMat4f("cameraView", currentCamera->getView());
	DirectionalLightCascadedShader->setInt("debugMode", static_cast<int>(visualCascadesVisible));

	//Directional light uniforms
	DirectionalLightCascadedShader->setVec4f("lightLook", sun.look.x, sun.look.y, sun.look.z, 0.0f);
	DirectionalLightCascadedShader->setVec3f("lightColor", sun.color.r, sun.color.g, sun.color.b);
	DirectionalLightCascadedShader->setFloat("Intensity", sun.color.a);
	DirectionalLightCascadedShader->setFloat("ShadowIntensity", sun.shadowIntensity);
	
	//DRAW
	///int faceCount = FSQ->GetFaceCount();
	///glDrawElements(GL_TRIANGLES, faceCount * 3, GL_UNSIGNED_INT, 0);
	FSQ->Draw();
	FSQ->UnbindVAO();
	DirectionalLightCascadedShader->UnbindShader();

	//Put everything back the way it was
	currentCamera = temp;
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

	//Opaque objects queue
	for (int i = 0; i < graphicQueue.size(); ++i)
	{
		//BINDING
		momentShadowShader->UseShader();
		DrawData &data = graphicQueue[i];

		//Since a node has a model (and all the meshes of a model for now share bones), we pass the uniform array
		if (data.BoneTransformations)
			shadowShader->setMat4fArray("BoneTransf", 100, (*(data.BoneTransformations))[0]);

		for (int j = 0; j < data.meshes->size(); ++j)
		{
			//BINDING
			(*data.meshes)[j]->BindVAO();

			//UNIFORM BINDING
			momentShadowShader->setMat4f("projView", lightProjView);
			momentShadowShader->setMat4f("model", data.model);
			
			//DRAW
			///int faceCount = (*data.meshes)[j]->GetFaceCount();
			///glDrawElements(GL_TRIANGLES, faceCount * 3, GL_UNSIGNED_INT, 0);
			(*data.meshes)[j]->Draw();

			//TODO - UNBIND SHADER AND MESH
			(*data.meshes)[j]->UnbindVAO();
		}

		momentShadowShader->UnbindShader();
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
	FSQ->BindVAO();
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
	///int faceCount = FSQ->GetFaceCount();
	///glDrawElements(GL_TRIANGLES, faceCount * 3, GL_UNSIGNED_INT, 0);
	FSQ->Draw();
	//TODO - UNBIND SHADER AND MESH
	FSQ->UnbindVAO();
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
	PointLightSphere->BindVAO();

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
		///int faceCount = PointLightSphere->GetFaceCount();
		///glDrawElements(GL_TRIANGLES, faceCount * 3, GL_UNSIGNED_INT, 0);
		PointLightSphere->Draw();
	}

	//TODO - UNBIND SHADER AND MESH
	PointLightSphere->UnbindVAO();
	DeferredPointLightShader->UnbindShader();
}


///////////////////////////////////////
/////		CASCADED STUFF        /////
///////////////////////////////////////
void DeferredRenderer::initCascadedParams()
{
	this->subdivissions = 3;					//SERIALIZE and/or IMGUI
	this->useCascadedShadows = true;			//SERIALIZE and/or IMGUI
	this->C_LightPROJi.resize(subdivissions);
	this->C_LightVIEWi.resize(subdivissions);

	//Debug view mode stuff----------------
	FustrumVertices1.resize(16);
	FustrumVertices2.resize(16);
	FustrumVertices3.resize(16);
	cascadeBB1.resize(16);
	cascadeBB2.resize(16);
	cascadeBB3.resize(16);
	//Debug view mode stuff----------------

	//Calculate the different z values (in view space) 
	//for the cascaded fustrums
	this->CalculateCascadedSubdivissions();

	//Once the z values are done, we need to create the different
	//Light fustrums that contain each of the subdivissions
	this->GenerateCascadedFustrums();
}


//This has to be called everytime the camera moves!
void DeferredRenderer::GenerateCascadedFustrums()
{
	//For now, its gonna be called independent of the camera moving or not
	this->GenerateCascadedLightView(this->C_lightView);

	float fov = mainCamera->getFOV();
	float fovRadians = fov * PI / 180.0f;
	float AR = mainCamera->getAspectRatio();
	float tanHalfFOV = std::tanf(fovRadians * 0.5F);

	int M = subdivissions;
	for (int i = 0; i < M; ++i)
	{
		//Six values needed for getting the light transformation matrices (view and proj)
		float zMin = this->Zi[i];
		float zMax = this->Zi[i+1];
		float n_HalfW = fabs(zMin) * tanHalfFOV;
		float n_HalfH = n_HalfW / AR;
		float f_HalfW = fabs(zMax) * tanHalfFOV;
		float f_HalfH = f_HalfW / AR;

		//Get the 8 bounding positions, in world space
		glm::vec4 n_topRight    = mainCamera->getViewInv() * glm::vec4( n_HalfW,  n_HalfH, zMin, 1.0f);
		glm::vec4 n_topLeft     = mainCamera->getViewInv() * glm::vec4(-n_HalfW,  n_HalfH, zMin, 1.0f);
		glm::vec4 n_bottomRight = mainCamera->getViewInv() * glm::vec4( n_HalfW, -n_HalfH, zMin, 1.0f);
		glm::vec4 n_bottomLeft  = mainCamera->getViewInv() * glm::vec4(-n_HalfW, -n_HalfH, zMin, 1.0f);
		glm::vec4 f_topRight    = mainCamera->getViewInv() * glm::vec4( f_HalfW,  f_HalfH, zMax, 1.0f);
		glm::vec4 f_topLeft     = mainCamera->getViewInv() * glm::vec4(-f_HalfW,  f_HalfH, zMax, 1.0f);
		glm::vec4 f_bottomRight = mainCamera->getViewInv() * glm::vec4( f_HalfW, -f_HalfH, zMax, 1.0f);
		glm::vec4 f_bottomLeft  = mainCamera->getViewInv() * glm::vec4(-f_HalfW, -f_HalfH, zMax, 1.0f);

		//Debug draw stuff-------------------------
		if (InDebugView && i == 0)
		{
			FustrumVertices1[0] = n_topRight;
			FustrumVertices1[1] = n_topLeft;
			FustrumVertices1[2] = n_bottomLeft;
			FustrumVertices1[3] = n_bottomRight;
			FustrumVertices1[4] = n_topRight;
			FustrumVertices1[5] = f_topRight;
			FustrumVertices1[6] = f_topLeft;
			FustrumVertices1[7] = n_topLeft;
			FustrumVertices1[8] = f_topLeft;
			FustrumVertices1[9] = f_bottomLeft;
			FustrumVertices1[10] = n_bottomLeft;
			FustrumVertices1[11] = f_bottomLeft;
			FustrumVertices1[12] = f_bottomRight;
			FustrumVertices1[13] = n_bottomRight;
			FustrumVertices1[14] = f_bottomRight;
			FustrumVertices1[15] = f_topRight;
		}
		else if (InDebugView && i == 1)
		{
			FustrumVertices2[0] = n_topRight;
			FustrumVertices2[1] = n_topLeft;
			FustrumVertices2[2] = n_bottomLeft;
			FustrumVertices2[3] = n_bottomRight;
			FustrumVertices2[4] = n_topRight;
			FustrumVertices2[5] = f_topRight;
			FustrumVertices2[6] = f_topLeft;
			FustrumVertices2[7] = n_topLeft;
			FustrumVertices2[8] = f_topLeft;
			FustrumVertices2[9] = f_bottomLeft;
			FustrumVertices2[10] = n_bottomLeft;
			FustrumVertices2[11] = f_bottomLeft;
			FustrumVertices2[12] = f_bottomRight;
			FustrumVertices2[13] = n_bottomRight;
			FustrumVertices2[14] = f_bottomRight;
			FustrumVertices2[15] = f_topRight;
		}
		else if (InDebugView && i == 2)
		{
			FustrumVertices3[0] = n_topRight;
			FustrumVertices3[1] = n_topLeft;
			FustrumVertices3[2] = n_bottomLeft;
			FustrumVertices3[3] = n_bottomRight;
			FustrumVertices3[4] = n_topRight;
			FustrumVertices3[5] = f_topRight;
			FustrumVertices3[6] = f_topLeft;
			FustrumVertices3[7] = n_topLeft;
			FustrumVertices3[8] = f_topLeft;
			FustrumVertices3[9] = f_bottomLeft;
			FustrumVertices3[10] = n_bottomLeft;
			FustrumVertices3[11] = f_bottomLeft;
			FustrumVertices3[12] = f_bottomRight;
			FustrumVertices3[13] = n_bottomRight;
			FustrumVertices3[14] = f_bottomRight;
			FustrumVertices3[15] = f_topRight;
		}
		//Debug draw stuff-------------------------

		//Add vertices to a list for easier iteration
		std::vector<glm::vec4> values;
		values.push_back(n_topRight);
		values.push_back(n_topLeft);
		values.push_back(n_bottomRight);
		values.push_back(n_bottomLeft);
		values.push_back(f_topRight);
		values.push_back(f_topLeft);
		values.push_back(f_bottomRight);
		values.push_back(f_bottomLeft);

		//Define the max and min variables
		float min_x = std::numeric_limits<float>::max();
		float min_y = std::numeric_limits<float>::max();
		float min_z = std::numeric_limits<float>::max();
		float max_x = std::numeric_limits<float>::lowest();
		float max_y = std::numeric_limits<float>::lowest();
		float max_z = std::numeric_limits<float>::lowest();

		//DEBU DRAW BB
		//Find the 6 boundary values (WORLD, FOR DEBUG DRAW)
		for (int index = 0; index < values.size(); ++index)
		{
			glm::vec4& vertex = values[index];
			min_x = std::fminf(min_x, vertex.x);
			min_y = std::fminf(min_y, vertex.y);
			min_z = std::fminf(min_z, vertex.z);
			max_x = std::fmaxf(max_x, vertex.x);
			max_y = std::fmaxf(max_y, vertex.y);
			max_z = std::fmaxf(max_z, vertex.z);
		}
		//-----------------------------------------
		if (i == 0 && InDebugView)
		{
			cascadeBB1[0] = glm::vec4(max_x, max_y, min_z, 1.0f);//topRight
			cascadeBB1[1] = glm::vec4(min_x, max_y, min_z, 1.0f);//topLeft
			cascadeBB1[2] = glm::vec4(min_x, min_y, min_z, 1.0f);//bottomLeft
			cascadeBB1[3] = glm::vec4(max_x, min_y, min_z, 1.0f);//bottomRight
			cascadeBB1[4] = glm::vec4(max_x, max_y, min_z, 1.0f);//topRight
			cascadeBB1[5] = glm::vec4(max_x, max_y, max_z, 1.0f);//topRight_far
			cascadeBB1[6] = glm::vec4(min_x, max_y, max_z, 1.0f);//topLeft_far
			cascadeBB1[7] = glm::vec4(min_x, max_y, min_z, 1.0f);//topLeft
			cascadeBB1[8] = glm::vec4(min_x, max_y, max_z, 1.0f);//topLeft_far
			cascadeBB1[9] = glm::vec4(min_x, min_y, max_z, 1.0f);//bottomLeft_far
			cascadeBB1[10] = glm::vec4(min_x, min_y, min_z, 1.0f);//bottomLeft
			cascadeBB1[11] = glm::vec4(min_x, min_y, max_z, 1.0f);//bottomLeft_far
			cascadeBB1[12] = glm::vec4(max_x, min_y, max_z, 1.0f);//bottomRight_far
			cascadeBB1[13] = glm::vec4(max_x, min_y, min_z, 1.0f);//bottomRight
			cascadeBB1[14] = glm::vec4(max_x, min_y, max_z, 1.0f);//bottomRight_far
			cascadeBB1[15] = glm::vec4(max_x, max_y, max_z, 1.0f);//topRight_far
		}
		else if (i == 1 && InDebugView)
		{
			cascadeBB2[0] = glm::vec4(max_x, max_y, min_z, 1.0f);//topRight
			cascadeBB2[1] = glm::vec4(min_x, max_y, min_z, 1.0f);//topLeft
			cascadeBB2[2] = glm::vec4(min_x, min_y, min_z, 1.0f);//bottomLeft
			cascadeBB2[3] = glm::vec4(max_x, min_y, min_z, 1.0f);//bottomRight
			cascadeBB2[4] = glm::vec4(max_x, max_y, min_z, 1.0f);//topRight
			cascadeBB2[5] = glm::vec4(max_x, max_y, max_z, 1.0f);//topRight_far
			cascadeBB2[6] = glm::vec4(min_x, max_y, max_z, 1.0f);//topLeft_far
			cascadeBB2[7] = glm::vec4(min_x, max_y, min_z, 1.0f);//topLeft
			cascadeBB2[8] = glm::vec4(min_x, max_y, max_z, 1.0f);//topLeft_far
			cascadeBB2[9] = glm::vec4(min_x, min_y, max_z, 1.0f);//bottomLeft_far
			cascadeBB2[10] = glm::vec4(min_x, min_y, min_z, 1.0f);//bottomLeft
			cascadeBB2[11] = glm::vec4(min_x, min_y, max_z, 1.0f);//bottomLeft_far
			cascadeBB2[12] = glm::vec4(max_x, min_y, max_z, 1.0f);//bottomRight_far
			cascadeBB2[13] = glm::vec4(max_x, min_y, min_z, 1.0f);//bottomRight
			cascadeBB2[14] = glm::vec4(max_x, min_y, max_z, 1.0f);//bottomRight_far
			cascadeBB2[15] = glm::vec4(max_x, max_y, max_z, 1.0f);//topRight_far
		}
		else if (i == 2 && InDebugView)
		{
			cascadeBB3[0] = glm::vec4(max_x, max_y, min_z, 1.0f);//topRight
			cascadeBB3[1] = glm::vec4(min_x, max_y, min_z, 1.0f);//topLeft
			cascadeBB3[2] = glm::vec4(min_x, min_y, min_z, 1.0f);//bottomLeft
			cascadeBB3[3] = glm::vec4(max_x, min_y, min_z, 1.0f);//bottomRight
			cascadeBB3[4] = glm::vec4(max_x, max_y, min_z, 1.0f);//topRight
			cascadeBB3[5] = glm::vec4(max_x, max_y, max_z, 1.0f);//topRight_far
			cascadeBB3[6] = glm::vec4(min_x, max_y, max_z, 1.0f);//topLeft_far
			cascadeBB3[7] = glm::vec4(min_x, max_y, min_z, 1.0f);//topLeft
			cascadeBB3[8] = glm::vec4(min_x, max_y, max_z, 1.0f);//topLeft_far
			cascadeBB3[9] = glm::vec4(min_x, min_y, max_z, 1.0f);//bottomLeft_far
			cascadeBB3[10] = glm::vec4(min_x, min_y, min_z, 1.0f);//bottomLeft
			cascadeBB3[11] = glm::vec4(min_x, min_y, max_z, 1.0f);//bottomLeft_far
			cascadeBB3[12] = glm::vec4(max_x, min_y, max_z, 1.0f);//bottomRight_far
			cascadeBB3[13] = glm::vec4(max_x, min_y, min_z, 1.0f);//bottomRight
			cascadeBB3[14] = glm::vec4(max_x, min_y, max_z, 1.0f);//bottomRight_far
			cascadeBB3[15] = glm::vec4(max_x, max_y, max_z, 1.0f);//topRight_far
		}
		//---------------------------------------*/

		//Transform into light space, and then find again min and max
		for (int index = 0; index < values.size(); ++index) 
		{
			glm::vec4& vertex = C_lightView * values[index];
			min_x = std::fminf(min_x, vertex.x);
			min_y = std::fminf(min_y, vertex.y);
			min_z = std::fminf(min_z, vertex.z);
			max_x = std::fmaxf(max_x, vertex.x);
			max_y = std::fmaxf(max_y, vertex.y);
			max_z = std::fmaxf(max_z, vertex.z);
		}

		/*
		if (i == 0 && InDebugView)
		{
			cascadeBB1[0] = glm::vec4(max_x, max_y, min_z, 1.0f);//topRight
			cascadeBB1[1] = glm::vec4(min_x, max_y, min_z, 1.0f);//topLeft
			cascadeBB1[2] = glm::vec4(min_x, min_y, min_z, 1.0f);//bottomLeft
			cascadeBB1[3] = glm::vec4(max_x, min_y, min_z, 1.0f);//bottomRight
			cascadeBB1[4] = glm::vec4(max_x, max_y, min_z, 1.0f);//topRight
			cascadeBB1[5] = glm::vec4(max_x, max_y, max_z, 1.0f);//topRight_far
			cascadeBB1[6] = glm::vec4(min_x, max_y, max_z, 1.0f);//topLeft_far
			cascadeBB1[7] = glm::vec4(min_x, max_y, min_z, 1.0f);//topLeft
			cascadeBB1[8] = glm::vec4(min_x, max_y, max_z, 1.0f);//topLeft_far
			cascadeBB1[9] = glm::vec4(min_x, min_y, max_z, 1.0f);//bottomLeft_far
			cascadeBB1[10] = glm::vec4(min_x, min_y, min_z, 1.0f);//bottomLeft
			cascadeBB1[11] = glm::vec4(min_x, min_y, max_z, 1.0f);//bottomLeft_far
			cascadeBB1[12] = glm::vec4(max_x, min_y, max_z, 1.0f);//bottomRight_far
			cascadeBB1[13] = glm::vec4(max_x, min_y, min_z, 1.0f);//bottomRight
			cascadeBB1[14] = glm::vec4(max_x, min_y, max_z, 1.0f);//bottomRight_far
			cascadeBB1[15] = glm::vec4(max_x, max_y, max_z, 1.0f);//topRight_far
		}
		else if (i == 1 && InDebugView)
		{
			cascadeBB2[0] = glm::vec4(max_x, max_y, min_z, 1.0f);//topRight
			cascadeBB2[1] = glm::vec4(min_x, max_y, min_z, 1.0f);//topLeft
			cascadeBB2[2] = glm::vec4(min_x, min_y, min_z, 1.0f);//bottomLeft
			cascadeBB2[3] = glm::vec4(max_x, min_y, min_z, 1.0f);//bottomRight
			cascadeBB2[4] = glm::vec4(max_x, max_y, min_z, 1.0f);//topRight
			cascadeBB2[5] = glm::vec4(max_x, max_y, max_z, 1.0f);//topRight_far
			cascadeBB2[6] = glm::vec4(min_x, max_y, max_z, 1.0f);//topLeft_far
			cascadeBB2[7] = glm::vec4(min_x, max_y, min_z, 1.0f);//topLeft
			cascadeBB2[8] = glm::vec4(min_x, max_y, max_z, 1.0f);//topLeft_far
			cascadeBB2[9] = glm::vec4(min_x, min_y, max_z, 1.0f);//bottomLeft_far
			cascadeBB2[10] = glm::vec4(min_x, min_y, min_z, 1.0f);//bottomLeft
			cascadeBB2[11] = glm::vec4(min_x, min_y, max_z, 1.0f);//bottomLeft_far
			cascadeBB2[12] = glm::vec4(max_x, min_y, max_z, 1.0f);//bottomRight_far
			cascadeBB2[13] = glm::vec4(max_x, min_y, min_z, 1.0f);//bottomRight
			cascadeBB2[14] = glm::vec4(max_x, min_y, max_z, 1.0f);//bottomRight_far
			cascadeBB2[15] = glm::vec4(max_x, max_y, max_z, 1.0f);//topRight_far
		}
		else if (i == 2 && InDebugView)
		{
			cascadeBB3[0] = glm::vec4(max_x, max_y, min_z, 1.0f);//topRight
			cascadeBB3[1] = glm::vec4(min_x, max_y, min_z, 1.0f);//topLeft
			cascadeBB3[2] = glm::vec4(min_x, min_y, min_z, 1.0f);//bottomLeft
			cascadeBB3[3] = glm::vec4(max_x, min_y, min_z, 1.0f);//bottomRight
			cascadeBB3[4] = glm::vec4(max_x, max_y, min_z, 1.0f);//topRight
			cascadeBB3[5] = glm::vec4(max_x, max_y, max_z, 1.0f);//topRight_far
			cascadeBB3[6] = glm::vec4(min_x, max_y, max_z, 1.0f);//topLeft_far
			cascadeBB3[7] = glm::vec4(min_x, max_y, min_z, 1.0f);//topLeft
			cascadeBB3[8] = glm::vec4(min_x, max_y, max_z, 1.0f);//topLeft_far
			cascadeBB3[9] = glm::vec4(min_x, min_y, max_z, 1.0f);//bottomLeft_far
			cascadeBB3[10] = glm::vec4(min_x, min_y, min_z, 1.0f);//bottomLeft
			cascadeBB3[11] = glm::vec4(min_x, min_y, max_z, 1.0f);//bottomLeft_far
			cascadeBB3[12] = glm::vec4(max_x, min_y, max_z, 1.0f);//bottomRight_far
			cascadeBB3[13] = glm::vec4(max_x, min_y, min_z, 1.0f);//bottomRight
			cascadeBB3[14] = glm::vec4(max_x, min_y, max_z, 1.0f);//bottomRight_far
			cascadeBB3[15] = glm::vec4(max_x, max_y, max_z, 1.0f);//topRight_far
		}
		//*/


		//Now with the 6 boundary values in world space, we can built a proj matrix for this cascade
		this->GenerateCascadedSpecificView(this->C_LightVIEWi[i], min_x, max_x, min_y, max_y, min_z, max_z);

		//Now with the 6 boundary values in world space, we can built a proj matrix for this cascade
		float maxWidth = std::fminf(max_x - min_x, max_y - min_y);
		float halfSize = maxWidth / 2.0f;
		this->GenerateCascadedProjection(this->C_LightPROJi[i], -halfSize, halfSize, -halfSize, halfSize, min_z, max_z);
		///this->GenerateCascadedProjection(this->C_LightPROJi[i], min_x, max_x, min_y, max_y, min_z, max_z);
	}
}


//This is to see if creating an alligned view for each cascade makes a difference
void DeferredRenderer::GenerateCascadedSpecificView(glm::mat4& view,
	float min_x, float max_x, float min_y, float max_y, float min_z, float max_z)
{
	float x = (min_x + max_x) * 0.5f;
	float y = (min_y + max_y) * 0.5f;
	float z = min_z;
	glm::vec3 cascadedEye = glm::vec3(x, y, z);
	//view = glm::lookAt(cascadedEye, cascadedEye + sun.look, glm::vec3(0, 1, 0));
	view = AuxMath::view(glm::vec4(cascadedEye, 1.0f), sun.look, glm::vec4(0, 1, 0, 0));
}


//Generates the view for the directional light. It only needs a direction, not a position
void DeferredRenderer::GenerateCascadedLightView(glm::mat4& view)
{
	//view = glm::lookAt(glm::vec3(0, 0, 0), sun.look, glm::vec3(0, 1, 0));
	view = AuxMath::view(glm::vec4(0, 0, 0, 1), sun.look, glm::vec4(0, 1, 0, 0));
}


void DeferredRenderer::GenerateCascadedProjection(glm::mat4& proj,
	float min_x, float max_x, float min_y, float max_y, float min_z, float max_z)
{
	//-HalfWidth, HalfWidth, -HalfHeight, halfHeight, near, far
	proj = glm::ortho(min_x, max_x, min_y, max_y, min_z, max_z);
}


void DeferredRenderer::CalculateCascadedSubdivissions()
{
	int M = subdivissions;
	float Corr = 0.5f;
	float n = mainCamera->getNear();
	float f = mainCamera->getFar();
	float f_over_n = f / n;
	float invM = 1.0f / M;

	//If we have n subdivissions, we need three depth values
	//First will always be near. Then we need n-1 to calculate
	//Finally, far is the furthest value
	for (int i = 0; i < M; ++i) 
	{
		float zi = Corr * (n * std::powf(f_over_n, i*invM)) + (1.0f - Corr) * (n + i*(f - n)*invM);
		this->Zi.push_back(-zi);
	}
	this->Zi.push_back(-f);
}


void DeferredRenderer::initCamera()
{
	//Init main camera
	mainCamera = new Camera();

	//Init debug camera
	debugCamera = new Camera(1.0f, 4000.0f, mainCamera->getFOV(), 10, 10);

	//Temporary, erase later somehow
	currentCamera = mainCamera;
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

	//Render target for Ambient Occlusion
	AOBuffer = new RenderTarget();
	RenderTargetDescriptor desc3;
	desc3.colorAttachmentCount = 2;
	desc3.width = VIEWPORT_WIDTH;
	desc3.height = VIEWPORT_HEIGHT;
	desc3.useStencil = false;
	desc3.componentType = GL_FLOAT;			// GL_UNSIGNED_BYTE
	desc3.imageInternalFormat = GL_R16F;	    // 1 float
	desc3.imageFormat = GL_RED;			    // GL_RGBA
	AOBuffer->initFromDescriptor(desc3);

	//Temporary, cascaded buffers setup
	CascadedBuffer01 = new RenderTarget();
	CascadedBuffer02 = new RenderTarget();
	CascadedBuffer03 = new RenderTarget();
	RenderTargetDescriptor desc4;
	desc4.colorAttachmentCount = 0;
	desc4.useStencil = false;
	desc4.componentType = GL_UNSIGNED_BYTE;
	desc4.width = 512;
	desc4.height = 512;
	CascadedBuffer01->initFromDescriptor(desc4);
	CascadedBuffer02->initFromDescriptor(desc4);
	CascadedBuffer03->initFromDescriptor(desc4);
	CascadedBuffers.push_back(CascadedBuffer01);
	CascadedBuffers.push_back(CascadedBuffer02);
	CascadedBuffers.push_back(CascadedBuffer03);
}


//////////////////////////////////////
//////		RANDOM STUFF		//////
//////////////////////////////////////
void DeferredRenderer::loadResources()
{
	shadowShader = new Shader("Shadow.vert", "Shadow.frag");

	momentShadowShader = new Shader("MomentShadow.vert", "MomentShadow.frag");

	geometryPassShader = new Shader("GeometryPass.vert", "GeometryPass.frag");
	geometryPassShader->BindUniformBlock("test_gUBlock", 1);
	
	FSQShader = new Shader("FSQ.vert", "FSQ.frag");
	FSQShader->BindUniformBlock("test_gUBlock", 1);

	DeferredAmbientShader = new Shader("DeferredAmbient.vert", "DeferredAmbient.frag");

	DirectionalLightShader = new Shader("DirectionalLight.vert", "DirectionalLight.frag");
	DirectionalLightShader->BindUniformBlock("test_gUBlock", 1);
	
	DirectionalLightCascadedShader = new Shader("DirectionalLightCascaded.vert", "DirectionalLightCascaded.frag");
	DirectionalLightCascadedShader->BindUniformBlock("test_gUBlock", 1);

	DeferredPointLightShader = new Shader("PointLightFSQ.vert", "PointLightFSQ.frag");
	DeferredPointLightShader->BindUniformBlock("test_gUBlock", 1);

	AOShader = new Shader("AmbientOcclusion.vert", "AmbientOcclusion.frag");
	AOShader->BindUniformBlock("test_gUBlock", 1);

	geometryInstancedShader = new Shader("GeometryInstanced.vert", "GeometryInstanced.frag");
	geometryInstancedShader->BindUniformBlock("test_gUBlock", 1);

	LineShader = new Shader("Line.vert", "Line.frag");
	LineShader->BindUniformBlock("test_gUBlock", 1);

	IBLShader = new Shader("DeferredIBL.vert", "DeferredIBL.frag");
	IBLShader->BindUniformBlock("test_gUBlock", 1);
	IBLShader->BindUniformBlock("SpecularSamples", 0);

	//Compute shader creation
	blurShaderHoriz = new Shader("BlurHoriz.comp");
	blurShaderVert = new Shader("BlurVert.comp");
	blurAOHoriz = new Shader("BlurAOHoriz.comp");
	blurAOVert = new Shader("BlurAOVert.comp");

	//Set weights for moments shadow mapping
	kernelCount = 5;
	SetKernelCount(kernelCount, weights);

	//Set weights for Ambient Occlusion
	AOKernelCount = 10;
	SetKernelCount(AOKernelCount, AOweights);

	//Hammersley low discrepancy rands
	this->specularBlock = new AuxMath::HammersleyBlock();
	AuxMath::genLowDiscrepancyPairs(SPECULAR_SAMPLES, this->specularBlock);

	//FSQ
	FSQ = new Quad(); 
	model = new Model("Sphere.fbx");
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
			Light_Radius[lightCount++] = 2.0f +(rand() % 10);
		}
	}
}


void DeferredRenderer::CalculateLightProjView()
{
	//Light proj-view
	float halfWidth = sun.width/2.f;
	float halfHeight = sun.height/2.f;
	glm::mat4 lightView = AuxMath::view(sun.eye/*currentCamera->getEye()*/, sun.look/*currentCamera->getLook()*/, glm::vec4(0, 1, 0, 0));
	//glm::mat4 lightProj = AuxMath::orthographic(width, height, ar, sun.near, sun.far);
	glm::mat4 lightProj = glm::ortho(-halfWidth, halfWidth, -halfHeight, halfHeight, sun.m_near, sun.m_far);

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


GLuint DeferredRenderer::generateTextureFromSurface(SDL_Surface *surface, std::string key, int mipmapLevels)
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

		// NEW WAY
		for (int i = 0; i <= mipmapLevels; ++i)
		{
			glTexImage2D(GL_TEXTURE_2D, i, mode, (*surface).w, (*surface).h, 0, mode, GL_UNSIGNED_BYTE, 
				(*surface).pixels);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, this->anisoLevel);
		}
		glGenerateMipmap(GL_TEXTURE_2D);

		// OLD WAY
		///glTexImage2D(GL_TEXTURE_2D, 0, mode, (*surface).w, (*surface).h, 0, mode, GL_UNSIGNED_BYTE, (*surface).pixels);
		///glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		///glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		///glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		///glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); 
		///glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, this->anisoLevel);

		++numberOfTexturesLoaded;
		texturesDict[key] = textures[numberOfTexturesLoaded - 1];
		mTexture = texturesDict[key];
	}

	return mTexture;
}


Camera *DeferredRenderer::GetCurrentCamera()
{
	return this->currentCamera;
}


GLuint DeferredRenderer::genHDRTexHandle(HDRImageDesc const& desc, int mipmapLevels)
{
	//First check if the map has the texture
	GLint mTexture = texturesDict[desc.name];

	if (mTexture == 0)
	{
		glGenTextures(1, textures + numberOfTexturesLoaded);
		glBindTexture(GL_TEXTURE_2D, textures[numberOfTexturesLoaded]);
		for (int i = 0; i <= mipmapLevels; ++i) 
		{
			glTexImage2D(GL_TEXTURE_2D, i, GL_RGB16F, desc.width, desc.height, 0, GL_RGB, GL_FLOAT, desc.data);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}
		glGenerateMipmap(GL_TEXTURE_2D);

		///glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		///glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		///glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		///glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		++numberOfTexturesLoaded;
		texturesDict[desc.name] = textures[numberOfTexturesLoaded - 1];
		mTexture = texturesDict[desc.name];
	}

	return mTexture;
}


//Skydome creation
void DeferredRenderer::CreateSkydome(HDRImageDesc const& hdrTexDesc,
	HDRImageDesc const& irradianceDesc)
{
	//Create openGlTexture
	int mipmaps = 5;
	GLuint tex = genHDRTexHandle(hdrTexDesc, mipmaps);
	GLuint irr = genHDRTexHandle(irradianceDesc);

	//Set initial stuff for skydome
	SkyDome *sky = new SkyDome(mipmaps);
	sky->geometry = new PolarPlane(32);
	sky->shader = new Shader("Sky.vert", "Sky.frag");
	sky->shader->BindUniformBlock("test_gUBlock", 1);

	//sky->irradiance = tex;
	//sky->texture = irr;
	sky->irradiance = irr;
	sky->texture = tex;

	//Pass to the camera
	this->initCamera();
	this->mainCamera->SetSkydome(sky);


	//Debug Camera skydome---------------------------
	SkyDome *skyDebug = new SkyDome(mipmaps);
	skyDebug->geometry = new PolarPlane(16);
	skyDebug->irradiance = irr;
	skyDebug->texture = tex;
	skyDebug->shader = new Shader("Sky.vert", "Sky.frag");
	skyDebug->shader->BindUniformBlock("test_gUBlock", 1);
	this->debugCamera->SetSkydome(skyDebug);
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


//////////////////////////////////////////////////////////////////////////////
////                    BONE DEBUG DRAWING                                ////
//////////////////////////////////////////////////////////////////////////////
void DeferredRenderer::DebugDrawSkeleton(DrawData const& data)
{
	//Extract the map
	std::unordered_map<std::string, Bone> const& map = static_cast<std::unordered_map<std::string, Bone>>(*data.BoneMap);

	//Set starting position
	glm::vec3 originPoint = glm::vec3(0);

	//Get the root
	Bone const& root = map.find("RootNode")->second;

	//Call recursive function
	DrawBoneToChildren(map, root, originPoint, data);
}


void DeferredRenderer::DrawBoneToChildren(std::unordered_map<std::string, Bone> const& map, 
	Bone const& node, glm::vec3& origin, DrawData const& data)
{
	//Try drawing the joint as spheres
	std::vector<glm::vec4> points;
	points.push_back(glm::vec4(origin.x, origin.y, origin.z, 1));
	if (node.name == "RootNode")
		this->DrawControlPoints(points, glm::vec3(1, 0, 0));
	else
		this->DrawControlPoints(points);


	//For every child, calculate the final pos, draw line, and call recursively
	for (std::string const& childName : node.children)
	{
		//Get the child
		Bone const& child = map.find(childName)->second;

		//Calculate final position
		glm::vec3 dest = child.accumTransformation * glm::vec4(0, 0, 0, 0.5f);
		glm::vec3 d = (dest - origin);

		//Draw the line
		this->DrawLineSegment(origin, d, data.model);

		//call recursive function for child
		DrawBoneToChildren(map, child, origin + d, data);
	}
}


void DeferredRenderer::DrawLineSegment(glm::vec3 const& orig, 
	glm::vec3 const& d, glm::mat4 const& model) 
{
	float vertices[8] =
	{
		orig.x, orig.y, orig.z, 1.0f,
		orig.x + d.x, orig.y + d.y, orig.z + d.z, 1.0f
	};

	glDisable(GL_DEPTH_TEST);

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

	//Draw
	LineShader->setMat4f("model", model);
	glDrawArrays(GL_LINES, 0, 2);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	LineShader->UnbindShader();
	glDeleteVertexArrays(1, &lineVAO);
	glDeleteBuffers(1, &LineVBO);
}
//----------------------------------------------------------------------------


//////////////////////////////////////////////////////////////////////////////
////    FOR PATH-FOLLOWING HOMEWORK                                       ////
//////////////////////////////////////////////////////////////////////////////
void DeferredRenderer::DrawControlPoints(std::vector<glm::vec4> const& points, 
	glm::vec3 const& color)
{
	for (glm::vec4 const& point : points)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

		//CULLING AND DEPTH
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);

		//USE SHADER AND BIND VAO
		FSQShader->UseShader();
		PointLightSphere->BindVAO();

		//Model for the controlpoint
		float radius = 0.5f;
		glm::mat4 pointLightModel = glm::mat4(1);		
		pointLightModel[3] = point;
		pointLightModel[0][0] = radius;
		pointLightModel[1][1] = radius;
		pointLightModel[2][2] = radius;

		//UNIFORM BINDING
		FSQShader->setVec3f("diffuseColor", color.r, color.g, color.b);
		FSQShader->setMat4f("model", pointLightModel);

		//DRAW
		int faceCount = PointLightSphere->GetFaceCount();
		glDrawElements(GL_TRIANGLES, faceCount * 3, GL_UNSIGNED_INT, 0);

		//TODO - UNBIND SHADER AND MESH
		PointLightSphere->UnbindVAO();
		FSQShader->UnbindShader();
	}
}

void DeferredRenderer::DrawCurve(std::vector<glm::vec4> const& points, 
	glm::vec3 const& color)
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

	GLuint lineVAO, LineVBO = static_cast<unsigned>(-1);
	glGenVertexArrays(1, &lineVAO);
	glGenBuffers(1, &LineVBO);
	LineShader->UseShader();

	LineShader->setVec3f("diffuseColor", color.x, color.y, color.z);
	LineShader->setMat4f("model", glm::mat4(1));

	glBindVertexArray(lineVAO);
	glBindBuffer(GL_ARRAY_BUFFER, LineVBO);
	glBufferData(GL_ARRAY_BUFFER, points.size() * 4 * sizeof(float), &points[0], GL_STATIC_DRAW);
	
	glVertexAttribPointer(0, 4, GL_FLOAT, false, 4 * sizeof(GLfloat), (void*)0);
	glEnableVertexAttribArray(0);

	//Draw
	glDrawArrays(GL_LINE_STRIP, 0, points.size());

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	LineShader->UnbindShader();
	glDeleteVertexArrays(1, &lineVAO);
	glDeleteBuffers(1, &LineVBO);
}
//----------------------------------------------------------------------------


//////////////////////////////////////////////////////////////////////////////
////    FOR DEBUG DRAWING CAMERA STUFF                                    ////
//////////////////////////////////////////////////////////////////////////////
void DeferredRenderer::toggleDebugViewMode()
{
	if (InDebugView)
	{
		InDebugView = false;
		currentCamera = mainCamera;
	}
	else 
	{
		InDebugView = true;
		glm::vec3 eye = currentCamera->getEye();
		glm::vec3 look = currentCamera->getLook();
		debugCamera->setEye(eye);
		debugCamera->setLook(look);
		currentCamera = debugCamera;
	}
}


void DeferredRenderer::toggleVisualCascades() 
{
	visualCascadesVisible = !visualCascadesVisible;
}


void DeferredRenderer::DrawDebugView()
{
	//Draw main camera
	DrawControlPoints({ mainCamera->getEye() }, glm::vec3(0, 0, 1));
	DrawCurve({ mainCamera->getEye(), mainCamera->getEye() + mainCamera->getLook() * 2.0f }, glm::vec3(0, 0, 1));

	//Draw fustrum with separations
	DrawCurve(FustrumVertices1, glm::vec3(1, 1, 0));
	DrawCurve(FustrumVertices2, glm::vec3(1, 1, 0));
	DrawCurve(FustrumVertices3, glm::vec3(1, 1, 0));

	//Draw the light bounding boxes around fustrum
	DrawCurve(cascadeBB1, glm::vec3(1, 1, 1));
	DrawCurve(cascadeBB2, glm::vec3(1, 1, 1));
	DrawCurve(cascadeBB3, glm::vec3(1, 1, 1));
}
//----------------------------------------------------------------------------