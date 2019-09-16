///HEADER STUFF

#include "RendererComponent.h"

//TODO - For now
#include "Model.h"
#include "Sphere.h"
#include "Quad.h"
#include "Shader.h"
#include "TransformComponent.h"
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
}

Render::~Render()
{
	std::cout << "Destroying Render component" << std::endl;

	if (model)
		delete model;
}

/*
Render* clone()
{
	return new Render();
}
//*/


void Render::DeserializeInit()
{
	initShader();

	initModel();
}

void Render::Update(float dt)
{
	// Pass shader (material) and mesh (model) info to GraphicsManager
	Transform *T = this->m_owner->GetComponent<Transform>();

	// Pack it maybe on a struct
	DrawData data;
	data.mesh = mesh;
	data.shader = shader;
	data.model = T->GetModel();
	data.normalsModel = T->GetNormalModel();
	data.diffuseTexture = renderer->GetTexture(this->diffuseTexture);

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
		this->mesh = this->model->meshes[0];
	}
	else 
	{
		if (this->primitive == "quad")
			this->mesh = new Quad();
		else if (this->primitive == "sphere")
			this->mesh = new Sphere(32);
	}
}