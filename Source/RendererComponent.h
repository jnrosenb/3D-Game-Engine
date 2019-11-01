///HEADER STUFF

#pragma once

///INCLUDES
#include "BaseComponent.h"

#include "../External/Includes/glm/glm.hpp"

class Model;
class Mesh;
class Shader;

class Render : public BaseComponent
{
public:
	friend class Factory;

//PUBLIC INTERFACE
public:
	Render(GameObject *owner);
	virtual ~Render();

	virtual Render* clone() override
	{
		return new Render(m_owner);
	}
	virtual void Update(float dt) override;
	virtual void DeserializeInit() override;


	//For now, animation component will be set here
	void SetupAnimationComponent();


public:
	//To compare when using templates
	static COMPONENT_TYPES const comp_class_type = RENDERER;

private:
	virtual void initShader();
	virtual void initModel();

public:	//TODO - Change for a public interface
	//TODO - Replace for model
	Model *model = NULL;
	std::string modelPath;
	std::string ShaderName;
	std::string diffuseTexture;
	glm::vec4 diffuseColor;
	bool use_loaded_mesh;
	std::string primitive;
	Mesh *mesh = NULL;
	//TODO - Replace for material
	Shader *shader = NULL;
};