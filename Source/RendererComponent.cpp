///HEADER STUFF

#include "ResourceManager.h"
extern ResourceManager *resMgr;

#include "RendererComponent.h"

//TODO - For now
#include "Model.h"
#include "Shader.h"
#include "TransformComponent.h"
#include "AnimationComponent.h"
#include "GameObject.h"


//Temporary (while no world exists)
#include "Renderer.h"
extern Renderer *renderer;


Render::Render(GameObject *owner) :
	BaseComponent(owner, COMPONENT_TYPES::RENDERER)
{
	//In case it is not overrided (NOT USED IN DEFERRED)
	this->ShaderName = "Solid";

	//For some vars which may or may not be defined in json
	SetClampedRoughness(1.0f);
	this->metallic = 0.0f;
	this->specularColor = glm::vec4(1);
	this->diffuseColor = glm::vec4(1);
	xTiling = 1;
	yTiling = 1;
	useDiffuseTexture = false;
	useNormalMap = false;
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

void Render::Begin()
{
}

void Render::Update(float dt)
{
}


void Render::Draw()
{
	// Pass shader (material) and mesh (model) info to GraphicsManager
	Transform *T = this->m_owner->GetComponent<Transform>();
	AnimationComponent *Anim = this->m_owner->GetComponent<AnimationComponent>();

	// Pack it maybe on a struct
	DrawData data = {};
	data.meshes = &this->model->meshes;
	data.shader = shader;
	data.model = T->GetModel();
	data.normalsModel = T->GetNormalModel();
	
	data.diffuseTexture = renderer->GetTexture(this->diffuseTexture);
	data.roughnessTexture = renderer->GetTexture(this->roughnessTex);
	data.metallicTexture = renderer->GetTexture(this->metallicTex);
	data.normalMap = renderer->GetTexture(this->normalMap);
	
	data.useDiffuseTexture = static_cast<int>(useDiffuseTexture);
	data.useNormalMap = static_cast<int>(useNormalMap);

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
	if (useAlpha)
		renderer->QueueForDrawAlpha(data);
	else
		renderer->QueueForDraw(data);
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
		this->model = new Model(modelPath);

		// Dirty way for now. If the loaded stuff 
		// has animation info, we can create a animationComp
		if (model && model->animMap.size() > 0) 
			SetupAnimationComponent();
	}
	else 
	{
		this->model = new Model(true, this->primitive);
	}
}


Model *Render::GetModel() const 
{
	return this->model;
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