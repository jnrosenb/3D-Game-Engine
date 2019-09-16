///HEADER STUFF

#pragma once

///INCLUDES


class Shader;


struct MaterialDescriptor 
{
};


class BaseMaterial 
{

//PUBLIC INTERFACE
public:
	BaseMaterial();
	BaseMaterial(BaseMaterial& rhs);
	virtual ~BaseMaterial();

	virtual void initMaterial(MaterialDescriptor const& desc);

//VARIABLEs
private:
	Shader *shader;

	MaterialDescriptor *vars;
};