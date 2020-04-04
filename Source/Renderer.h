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
#include "Math/Stat.h"
#include "Shader.h"
#include "Mesh.h"


#define VIEWPORT_WIDTH						1280
#define VIEWPORT_HEIGHT						720
#define MAX_LIGHT_COUNT						2000	//Arbitrary value
#define MAX_NUMBER_TEXTURES					512		//Arbitrary value
#define MAX_PARTICLES						100000	//Arbitrary value


//Forward decl
class Camera;
class RenderTarget;
class Bone;
struct HDRImageDesc;


//Lights (UNUSED FOR NOW)
struct DirectionalLight
{
	glm::vec4 eye;
	glm::vec4 color;
	glm::vec4 look;
	float m_near;
	float m_far;
	int width, height;
	float shadowIntensity;

	//Intensity of light coded in alpha of light color ***

	DirectionalLight() : eye(glm::vec4(0.0f, 10.0f, -10.0f, 1)), 
		color(glm::vec4(1.f, 1.f, 1.f, 1.f)), look(glm::vec4(0.0f, -1.0f, -1.0f, 0)), 
		m_near(0.1f), m_far(1000.0f), width(40), height(30), shadowIntensity(1.f)
	{}

	DirectionalLight(float x, float y, float z, float r, float g, float b,
		float dx, float dy, float dz, float n, float f) : 
		eye(glm::vec4(x, y, z, 1)), color(glm::vec4(r, g, b, 1.f)),
		look(glm::vec4(dx, dy, dz, 0)), m_near(n), m_far(f)
	{}
};


//Struct that represents a skydome
struct SkyDome 
{
	Mesh *geometry;
	Shader *shader;
	GLuint texture;
	GLuint irradiance;
	int specularMipmap;
	glm::mat4 model;

	SkyDome(int SpecMipmaps) : geometry(0), shader(0),
		texture(-1), irradiance(-1), 
		model(glm::mat4(1)), specularMipmap(SpecMipmaps)
	{}

	~SkyDome()
	{
		delete shader;
		delete geometry;
	}

	void Draw(/*GLuint geoDepth*/) 
	{
		//BINDING
		shader->UseShader();
		geometry->BindVAO();

		//UNIFORM BINDING
		shader->setMat4f("model", model);
		shader->setTexture("skyMap", texture, 0);

		//DRAW
		int faceCount = geometry->GetFaceCount();
		glDrawElements(GL_TRIANGLES, faceCount * 3, GL_UNSIGNED_INT, 0);

		//TODO - UNBIND SHADER AND MESH
		geometry->UnbindVAO();
		shader->UnbindShader();
	}
};


//Struct with the data needed by renderer to draw everything
struct DrawData 
{
	glm::mat4 model;
	glm::mat4 normalsModel;
	glm::vec4 diffuseColor;
	glm::vec4 specularColor;
	std::vector<Mesh*> *meshes;
	Shader *shader;

	//Maps
	GLuint diffuseTexture;
	GLuint roughnessTexture;
	GLuint metallicTexture;
	GLuint normalMap;

	//Future flag (for now, bools)
	int useDiffuseTexture;
	int useNormalMap;
	int castShadow;

	//PBR PARAMS
	float roughness;
	float metallic;

	//tiling
	int xTiling;
	int yTiling;

	unsigned boneCount;
	std::unordered_map<std::string, Bone> *BoneMap; //Only for debug drawing, temporary

	//Bones experiment
	std::vector<glm::mat4> *BoneTransformations;
};


//For debug drawing or similar
struct DrawDebugData
{
	glm::mat4 model;
	glm::vec4 diffuseColor;
	Mesh *mesh;
	//std::vector<Mesh*> *meshes;
	Shader *shader;

	/// unsigned boneCount;
	/// std::unordered_map<std::string, Bone> *BoneMap; //Only for debug drawing, temporary
	//Bones experiment
	std::vector<glm::mat4> *BoneTransformations;
};


//Struct with the data needed by renderer to draw instanced objects
struct DrawInstanceData
{
	std::vector<glm::mat4> *modelMatrices;

	glm::vec4 diffuseColor;
	glm::vec4 specularColor;
	std::vector<Mesh*> *meshes;

	//Maps
	GLuint diffuseTexture;

	//TEMPORARY MEASURE
	GLuint TempVBO;

	//Future flag (for now, bools)
	int useDiffuseTexture;

	//tiling
	int xTiling;
	int yTiling;

	//Only used for instancing
	int instanceCount;
};



/////////////////////////////////////////////
////        RENDERER CLASS               ////
/////////////////////////////////////////////
class Renderer 
{
public:
	friend class Factory;

//PUBLIC INTERFACE
public:
	Renderer() : lightCount(0)
	{
		//ANISO TODO - find how to move from here to init
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &fLargest_aniso);
		anisoLevel = 0.0f;

		//MSAA
		msaa = true;
		glEnable(GL_MULTISAMPLE);
	}
	virtual ~Renderer() {}

	//Initialization
	virtual void init() = 0;
	virtual void initCamera() = 0;
	virtual void initFrameBuffers() = 0;

	//Data that will be drawn into the scene
	virtual void QueueForDraw(DrawData& data) = 0;
	virtual void QueueForDrawAlpha(DrawData& data) = 0;
	virtual void QueueForDrawInstanced(DrawInstanceData& data) = 0;
	virtual void QueueForDebugDraw(DrawDebugData& data) {}

	//Skydome creation
	virtual void CreateSkydome(HDRImageDesc const& texPath,
		HDRImageDesc const& irrPath) 
	{
		//TODO - check wether make it abstract
	}

	//ANISO
	float GetMaxAnisotropicLevel() const 
	{
		return this->fLargest_aniso;
	}

	//TEXTURE LOADING
	virtual GLuint generateTextureFromSurface(SDL_Surface *surface, 
		std::string key, int mipmaps = 0) = 0;
	virtual GLuint GetTexture(std::string key) = 0;

	//MSAA
	void ToggleMSAA() 
	{
		if (msaa)
			glDisable(GL_MULTISAMPLE);
		else
			glEnable(GL_MULTISAMPLE);
		msaa = !msaa;
	}
	bool IsMSAAOn() const
	{
		return msaa;
	}

	virtual void Update(float dt) = 0;
	virtual void Draw() = 0;

//LATER MOVE TO PROTECTED
public:

	//ANISO
	float anisoLevel;

	//Bone debug drawing
	bool DrawSkin;
	bool DrawSkeleton;

	//Compute shader for blurring
	void SetKernelCount(int newVal, std::vector<float>& w)
	{
		OnChangingKernelCount(w, newVal);
	}

//VARIABLES
protected:

	//Compute shader for blurring
	void OnChangingKernelCount(std::vector<float>& w, int &count)
	{
		w.clear();
		int kernel_radius = (count - 1) * 0.5f;
		AuxMath::genGaussianWeights(kernel_radius, w);
	}
	
	//Compute shaders
	Shader *blurShaderHoriz;
	Shader *blurShaderVert;
	Shader *blurAOHoriz;
	Shader *blurAOVert;

	//Weights for moments shadow mapping
	std::vector<float> weights;
	int kernelCount;

	//Weights for AO
	std::vector<float> AOweights;
	int AOKernelCount;

	//LIGHTS
	glm::vec4 Light_Colors[MAX_LIGHT_COUNT];
	glm::vec4 Light_Positions[MAX_LIGHT_COUNT];
	float Light_Radius[MAX_LIGHT_COUNT];
	int lightCount;
	DirectionalLight sun;

	//ANISO
	GLfloat fLargest_aniso;

	//MSAA
	bool msaa;
};