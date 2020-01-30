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
#include "Shapes.h"


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

	//Data that will be drawn into the scene
	virtual void QueueForDraw(DrawData& data) override;
	virtual void QueueForDrawAlpha(DrawData& data) override;
	virtual void QueueForDrawInstanced(DrawInstanceData& data) override;
	virtual void QueueForDebugDraw(DrawDebugData& data) override;

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
	void DrawBoundingBox(AABB const& AABB, glm::vec3 const& color);

	//Camera stuff
	void toggleDebugViewMode(); 
	void toggleVisualCascades();

	//Toggle debug draw
	void toggleDebugPass() 
	{
		DrawDebugPass = !DrawDebugPass;
	}

	//AO-Controller
	void toggleAO() 
	{
		useAO = !useAO;
	}


	//PRIVATE METHODS
private:
	DeferredRenderer(DeferredRenderer& rhs);

	//Bone debug drawing
	void DebugDrawSkeleton(DrawData const& data);
	void DrawBoneToChildren(std::unordered_map<std::string, Bone> const& map,
		Bone const& node, glm::vec3& origin, DrawData const& data);
	void DrawLineSegment(glm::vec3 const& orig, glm::vec3 const& dest,
		glm::mat4 const& model);

	//INIT METHODS
	virtual void init();
	virtual void initCamera();
	virtual void initFrameBuffers();
	virtual void initCascadedParams();

	//PASSES
	void GeometryPass();
	void AmbientOcclusionPass();
	void AmbientIBLPass();
	void SkydomePass();
	void FilteredShadowPass();
	void CascadedShadowPass();
	void MultiplePointLightPass(glm::mat4& projView);
	void AmbientLightPass();// Replaced by IBL pass

	//Aux methods
	void loadResources();
	void CalculateLightProjView();

	//Uniform buffer object (LATER TAKE OUT OF HERE)
	void initUniformBufferObjects();

	//Cascaded shadow map stuff
	void CalculateCascadedSubdivissions();
	void GenerateCascadedFustrums();
	void GenerateCascadedLightView(glm::mat4& view); 
	void GenerateCascadedSpecificView(glm::mat4& view, float min_x, 
		float max_x, float min_y, float max_y, float min_z, float max_z);
	void GenerateCascadedProjection(glm::mat4& proj, float min_x, 
		float max_x, float min_y, float max_y, float min_z, float max_z);

	//IBL preparation
	void prepareSpecularRandomPairs();
	AuxMath::HammersleyBlock *specularBlock;

	//Debug view mode
	void DrawDebugView();

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
	Shader *momentShadowShader;
	Shader *geometryPassShader;
	Shader *pointLightShader;
	Shader *FSQShader;
	Shader *geometryInstancedShader;
	Shader *DeferredPointLightShader;
	Shader *DeferredAmbientShader;
	Shader *DirectionalLightShader;
	Shader *LineShader;
	Shader *IBLShader;
	Shader *AOShader;
	Shader *DirectionalLightCascadedShader;

	//AO
	bool useAO;
	bool DrawDebugPass;

	//Cascaded shadow maps
	int subdivissions;
	bool useCascadedShadows;
	std::vector<float> Zi;
	glm::mat4 C_lightView;
	std::vector<glm::mat4> C_LightVIEWi;
	std::vector<glm::mat4> C_LightPROJi;

	//SHADOW MAP STUFF
	glm::mat4 lightProjView;

	//Elements to be drawn
	std::vector<DrawData> graphicQueue;
	std::vector<DrawInstanceData> instanceQueue;
	std::vector<DrawDebugData> debugQueue;
	std::vector<DrawData> graphicQueueAlpha; //TODO - Change to a std::set or something

	//First person camera
	Camera *mainCamera; 
	Camera *debugCamera;
	Camera *currentCamera; //Future version. Will hold the current camera to render scene
	bool InDebugView;
	bool visualCascadesVisible;
	std::vector<glm::vec4> FustrumVertices1;
	std::vector<glm::vec4> FustrumVertices2;
	std::vector<glm::vec4> FustrumVertices3;
	std::vector<glm::vec4> cascadeBB1;
	std::vector<glm::vec4> cascadeBB2;
	std::vector<glm::vec4> cascadeBB3;

	//UBO
	GLuint ubo_test;
	GLuint ubo_weights;
	GLuint ubo_IBLSpecular;
	GLuint ubo_bones;

	//Framebuffer
	RenderTarget *GeometryBuffer;
	RenderTarget *ShadowBuffer;
	RenderTarget *AOBuffer;
	//for now, cascaded will need to use one buffer per stuff, until I make it work. Then Ill switch to array textures
	RenderTarget *CascadedBuffer01;
	RenderTarget *CascadedBuffer02;
	RenderTarget *CascadedBuffer03;
	std::vector<RenderTarget*> CascadedBuffers;
};