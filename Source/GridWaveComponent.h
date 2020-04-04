///HEADER STUFF

#pragma once

///INCLUDES
#include "BaseComponent.h"
#include <vector>
#include "../External/Includes/glm/glm.hpp"
#include <cmath>
#include <complex>
#include "Math/FFT2.h"	//Potential replacement for Fourier.h

class Mesh;
class Shader;


class GridWaveComponent : public BaseComponent
{
public:
	friend class Factory;

//PUBLIC INTERFACE
public:
	GridWaveComponent(GameObject *owner);
	virtual ~GridWaveComponent();
	

	virtual GridWaveComponent* clone() override
	{
		return new GridWaveComponent(m_owner);
	}
	virtual void Update(float dt) override;
	///virtual void Draw() override;
	virtual void DeserializeInit() override;
	virtual void Begin() override;

private:
	void InitGrid();
	void GraphicSetup(); 
	void UpdateBuffers();

	//EXPERIMENT
	std::complex<float> PowerSpectrum(glm::vec2 const& k);

	void handleInput(float dt);

public:
	//To compare when using templates
	static COMPONENT_TYPES const comp_class_type = GRID_WAVE;

private:
	//Grid params
	AuxMath::Grid baseGrid;
	int gridWidth;
	int gridHeight;
	float worldWidth;
	float worldHeight;

	//TEMPORAL - For testing purposes
	float k, alpha, phillipAmplitude;
	float totalTime;

	//For drawing the plane mesh
	std::vector<glm::vec4> vertices;
	std::vector<glm::vec4> normals;
	std::vector<glm::vec4> tgs;
	std::vector<glm::vec4> bitgs;
};