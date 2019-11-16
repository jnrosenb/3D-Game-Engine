///HEADER STUFF

#pragma once

///INCLUDES
///#include "../External/Includes/GL/glew.h"
#include "../External/Includes/SDL2/SDL_surface.h"
#include "../External/Includes/Glad_/Glad/glad.h"
#include <gl/GL.h>
#include "../External/Includes/glm/glm.hpp"
#include <vector>
#include <map>
#include <unordered_map>
#include "Renderer.h"


#define VIEWPORT_WIDTH						1280
#define VIEWPORT_HEIGHT						720
#define MAX_NUMBER_TEXTURES					512			//Arbitrary value


//Forward decl
class Model;
class Camera;
class RenderTarget;
struct HDRImageDesc;
struct HammersleyBlock;


//RENDERER CLASS
class DeferredRenderer : public Renderer
{
public:
	friend class Factory;

	//PUBLIC INTERFACE
public:
	DeferredRenderer();
	virtual ~DeferredRenderer();

	//Initialization
	virtual void init();
	virtual void initCamera();
	virtual void initFrameBuffers();

	//Data that will be drawn into the scene
	virtual void QueueForDraw(DrawData& data);
	virtual void QueueForDrawAlpha(DrawData& data);

	void loadResources();
	void CalculateLightProjView();

	//Used to be able to get the camera (for now, until there is a camera manager)
	Camera *GetCurrentCamera();

	//Skydome creation
	virtual void CreateSkydome(HDRImageDesc const& hdrTexDesc,
		HDRImageDesc const& irradianceDesc) override;

	//TEXTURE LOADING
	GLuint generateTextureFromSurface(SDL_Surface *surface, std::string key, 
		int mipmapLevels = 0);
	GLuint genHDRTexHandle(HDRImageDesc const& desc, int mipmapLevels = 0);
	GLuint GetTexture(std::string key);

	virtual void Update(float dt);
	virtual void Draw();


	//ONLY USED FOR THE PATH-FOLLOW HOMEWORK
	void DrawControlPoints(std::vector<glm::vec4> const& points, 
		glm::vec3 const& color = glm::vec3(0, 1, 0));
	void DrawCurve(std::vector<glm::vec4> const& points, 
		glm::vec3 const& color);


	//PRIVATE METHODS
private:
	DeferredRenderer(DeferredRenderer& rhs);

	//Bone debug drawing
	void DebugDrawSkeleton(DrawData const& data);
	void DrawBoneToChildren(std::unordered_map<std::string, Bone> const& map,
		Bone const& node, glm::vec3& origin, DrawData const& data);
	void DrawLineSegment(glm::vec3 const& orig, glm::vec3 const& dest,
		glm::mat4 const& model);

	//PASSES
	void GeometryPass();
	void AmbientLightPass();
	void AmbientIBLPass();
	void SkydomePass();
	void FilteredShadowPass();
	void MultiplePointLightPass(glm::mat4& projView);

	//Uniform buffer object (LATER TAKE OUT OF HERE)
	void initUniformBufferObjects();

	//IBL preparation
	void prepareSpecularRandomPairs();
	AuxMath::HammersleyBlock *specularBlock;

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

	//Deferred FSQ
	Model *model;
	Mesh *FSQ;
	Mesh *PointLightSphere; //Deleted by model

	//Deferred shaders
	Shader *shadowShader;
	Shader *geometryPassShader;
	Shader *pointLightShader;
	Shader *FSQShader;
	Shader *DeferredPointLightShader;
	Shader *DeferredAmbientShader;
	Shader *DirectionalLightShader;
	Shader *LineShader;
	Shader *IBLShader;

	//SHADOW MAP STUFF
	glm::mat4 lightProjView;

	//Elements to be drawn
	std::vector<DrawData> graphicQueue;
	std::vector<DrawData> graphicQueueAlpha; //TODO - Change to a std::set or something

	//First person camera
	Camera *currentCamera; //Future version. Will hold the current camera to render scene

	//UBO
	GLuint ubo_test;
	GLuint ubo_weights;
	GLuint ubo_IBLSpecular;
	GLuint ubo_bones;

	//Framebuffer
	RenderTarget *GeometryBuffer;
	RenderTarget *ShadowBuffer;
};