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

	void SetClampedRoughness(float r);

public:	//TODO - Change for a public interface
	
	Model *model = NULL;
	std::string modelPath;
	std::string ShaderName;
	glm::vec4 diffuseColor;
	glm::vec4 specularColor;
	bool use_loaded_mesh;
	std::string primitive;
	Mesh *mesh = NULL;
	Shader *shader = NULL;

	//maps
	std::string diffuseTexture;
	std::string roughnessTex;
	std::string metallicTex;
	std::string normalMap;

	int xTiling, yTiling;

	//PBR-IBL params
	float roughness;
	float metallic;
};