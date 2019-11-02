///HEADER STUFF

#include "RendererComponent.h"

//TODO - For now
#include "Model.h"
#include "Sphere.h"
#include "Quad.h"
#include "Polar.h"
#include "Shader.h"
#include "TransformComponent.h"
#include "AnimationComponent.h"
#include "GameObject.h"


//Temporary (while no world exists)
#include "Renderer.h"
#include "ResourceManager.h"
extern Renderer *renderer;
extern ResourceManager *resMgr;


Render::Render(GameObject *owner) :
	BaseComponent(owner, COMPONENT_TYPES::RENDERER)
{
	//In case it is not overrided
	this->ShaderName = "Solid";

	//For some vars which may or may not be defined in json
	SetClampedRoughness(1.0f);
	this->metallic = 0.0f;
	this->specularColor = glm::vec4(1);
	this->diffuseColor = glm::vec4(1);
	xTiling = 1;
	yTiling = 1;
}

Render::~Render()
{
	std::cout << "Destroying Render component" << std::endl;

	if (model)
		delete model;
}


void Render::DeserializeInit()
{
	initShader();

	initModel();
}

void Render::Update(float dt)
{
	// Pass shader (material) and mesh (model) info to GraphicsManager
	Transform *T = this->m_owner->GetComponent<Transform>();
	AnimationComponent *Anim = this->m_owner->GetComponent<AnimationComponent>();

	// Pack it maybe on a struct
	DrawData data;
	data.meshes = &this->model->meshes;
	data.shader = shader;
	data.model = T->GetModel();
	data.normalsModel = T->GetNormalModel();
	
	data.diffuseTexture = renderer->GetTexture(this->diffuseTexture);
	data.roughnessTexture = renderer->GetTexture(this->roughnessTex);
	data.metallicTexture = renderer->GetTexture(this->metallicTex);
	
	data.roughness = this->roughness;
	data.metallic = this->metallic;

	data.xTiling = this->xTiling;
	data.yTiling = this->yTiling;
	
	data.diffuseColor = this->diffuseColor;
	data.specularColor = this->specularColor;

	//Bones experiment SHITTY WAY
	if (Anim) 
	{
		data.BoneTransformations = &(Anim->BoneTransformations);
		data.boneCount = this->model->boneMap.size();

		data.BoneMap = &(this->model->boneMap);
	}
	else
	{
		data.BoneTransformations = 0;
		data.boneCount = 0;
	}

	// Pass it to renderer's queue
	renderer->QueueForDraw(data);

	// TODO - there should be a check for transparency flag
	//        on material, to see to which queue to send
}

void Render::initShader()
{
	//TODO - Make this paths relative!!
	this->shader = resMgr->loadShader(this->ShaderName);
	//TODO - Remove later from here
	this->shader->BindUniformBlock("test_ubo", 0); 
	// TODO - shaders should receive a map (string,int) to decide which uniform blocks they will bind

	if (this->shader->IsValid())
		std::cout << "Shader handle created correctly!" << std::endl;

}

void Render::initModel()
{
	if (use_loaded_mesh) 
	{
		std::string const abs_path_prefix = "C:\\Users\\Jose\\Desktop\\OpenGl_Framework\\Assets\\Models\\";
		this->model = new Model(abs_path_prefix + modelPath);

		// Dirty way for now. If the loaded stuff 
		// has animation info, we can create a animationComp
		if (model && model->animMap.size() > 0) 
			SetupAnimationComponent();
	}
	else 
	{
		this->model = new Model();
		if (this->primitive == "quad")
			this->model->meshes.push_back(new Quad());
		else if (this->primitive == "sphere")
			this->model->meshes.push_back(new Sphere(32));
		else if (this->primitive == "polar")
			this->model->meshes.push_back(new PolarPlane(64));
	}
}



void Render::SetClampedRoughness(float r)
{
	if (r < 0.0001f)
		this->roughness = 0.0001f;
	else if (r > 1.0f)
		this->roughness = 1.0f;
	else
		this->roughness = r;
}



//Since it is difficult to setup the whole fbx - separation between mesh and animations, this will be
//done automatically. If the model loads animations, a animationComp will be created and passed the relevant info
void Render::SetupAnimationComponent()
{
	AnimationComponent *animComp = this->m_owner->AddComponent<AnimationComponent>();
	if (animComp)
	{
		animComp->PassAnimationInfo();
	}
}