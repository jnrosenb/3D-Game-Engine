///HEADER STUFF

#include "GridWaveComponent.h"
#include "GameObject.h"

#include "Meshes/DynamicGrid.h"
#include "Math/Stat.h"

// TODO Temporary (while no world exists)
#include "InputManager.h"
extern InputManager *inputMgr;
#include "DeferredRenderer.h"
extern Renderer *renderer;

#include "TransformComponent.h"
#include "RendererComponent.h"
#include "Model.h"


#define EPSILON		0.000001f
#define PI			3.14159265359f
#define TwoPi		2*PI



GridWaveComponent::GridWaveComponent(GameObject *owner) :
	BaseComponent(owner, COMPONENT_TYPES::GRID_WAVE)
{
	G = 9.8f;
	wind = glm::vec2(0);
}

GridWaveComponent::~GridWaveComponent()
{
	std::cout << "Destroying GridWaveComponent Component" << std::endl;
}


void GridWaveComponent::Update(float dt)
{
	//TODO temporal
	totalTime += dt;
	handleInput(dt);

	//Fourier Update
	for (int row = 1; row < gridHeight - 1; ++row)
	{
		for (int col = 1; col < gridWidth - 1; ++col)
		{
			std::complex<float> i = std::sqrt(std::complex<float>(-1.0f));
			glm::vec2 k = AuxMath::frequencyVector(baseGrid, row, col);
			/// glm::vec2 r = AuxMath::SampleGaussian_BoxMuller(1.0f);
			
			float w = glm::length(k) * G;
			 
			/// std::complex<float> P1 = PowerSpectrum(k);
			/// std::complex<float> num = (r.x + i * r.y) * std::sqrt(P1);
			/// std::complex<float> invDen = 1.0f / 1.41421356237f; // std::sqrt(2);
			/// std::complex<float> ukt1 = num * invDen;
			 
			/// std::complex<float> P2 = PowerSpectrum(-k);
			/// std::complex<float> num2 = (r.x + i * r.y) * std::sqrt(P2);
			/// std::complex<float> ukt2 = num2 * invDen;

			std::complex<float> ukt1 = std::get<0>(PowerGrid[gridWidth * row + col]);
			std::complex<float> ukt2 = std::get<1>(PowerGrid[gridWidth * row + col]);

			baseGrid(col, row) = ukt1 * std::exp(i * w*totalTime) + ukt2 * std::exp(i*w*totalTime);
		}
	}

	//IFT
	AuxMath::IFFT2<float> solver((int)std::log2(baseGrid.getWidth()), (int)std::log2(baseGrid.getHeight()));
	solver(baseGrid);


	//Update buffers (gl)
	UpdateBuffers();

	//Update mesh in rendererComp
	Render *rendererComp = m_owner->GetComponent<Render>();
	Model *model = rendererComp->GetModel();
	DynamicGridMesh *oceanGrid = static_cast<DynamicGridMesh*>(model->meshes[0]);
	oceanGrid->UpdateGridAttributes(vertices, normals, tgs, bitgs);
}


void GridWaveComponent::UpdateBuffers()
{
	//Auxiliar variables
	float cellWidth = 1.0f / gridWidth;
	float cellHeight = 1.0f / gridHeight;

	//Setup vertices 
	vertices.clear();
	for (int i = 0; i < gridHeight; ++i)
		for (int j = 0; j < gridWidth; ++j)
			vertices.push_back(glm::vec4(
				(i - gridHeight / 2)*cellWidth, 
				baseGrid(j, i).real()/*grid[j*gridWidth + i].real()*/, 
				(j - gridWidth / 2)*cellHeight, 
				1.0f
			));

	//Setup normals, tgs, bitgs
	normals.clear();
	tgs.clear();
	bitgs.clear();
	for (int i = 0; i < gridHeight; ++i)
	{
		for (int j = 0; j < gridWidth; ++j)
		{
			//CORRECT THE WEIRD BORDER ISSUE
			// When condition is false, corner vertices are been given a wrong default value (unless zero works?)
			//Cache the vertices
			glm::vec3 ctr = vertices[gridWidth*i + j];
			glm::vec3 t_r = (i < (gridHeight - 1) && j < (gridWidth - 1)) ? vertices[gridWidth*(i + 1) + (j + 1)] : glm::vec3(0, 0, 0);
			glm::vec3 t_m = (j < gridWidth - 1) ? vertices[gridWidth*(i)+(j + 1)] : glm::vec3(0, 0, 0);
			glm::vec3 b_m = (j > 0) ? vertices[gridWidth*(i)+(j - 1)] : glm::vec3(0, 0, 0);
			glm::vec3 b_l = (i > 0 && j > 0) ? vertices[gridWidth*(i - 1) + (j - 1)] : glm::vec3(0, 0, 0);
			glm::vec3 m_r = (i < (gridHeight + 1)) ? vertices[gridWidth*(i + 1) + (j)] : glm::vec3(0, 0, 0);
			glm::vec3 m_l = (i > 0) ? vertices[gridWidth*(i - 1) + (j)] : glm::vec3(0, 0, 0);

			//Calculate the normal for each of the adjacent 6 faces
			glm::vec3 normal(0);
			normal += glm::cross(m_l - t_m, ctr - t_m);
			normal += glm::cross(t_m - t_r, ctr - t_r);
			normal += glm::cross(t_r - m_r, ctr - m_r);
			normal += glm::cross(ctr - m_r, b_m - m_r);
			normal += glm::cross(m_l - ctr, b_l - ctr);
			normal += glm::cross(b_l - ctr, b_m - ctr);
			normal = glm::normalize(normal);
			normals.push_back(glm::vec4(normal.x, normal.y, normal.z, 0.0f));

			//Tgs
			glm::vec3 tg = m_r - ctr;
			tgs.push_back(glm::vec4(tg.x, tg.y, tg.z, 0.0f));

			//Bitg
			glm::vec3 bitg = glm::cross(normal, tg);
			bitgs.push_back(glm::vec4(bitg.x, bitg.y, bitg.z, 0.0f));
		}
	}
}


void GridWaveComponent::DeserializeInit()
{
	//TODO temporal
	totalTime = 0.0f;

	//Initialize grid
	InitGrid();
}

void GridWaveComponent::Begin() 
{
	//Initialize graphic stuff
	GraphicSetup();
}


void GridWaveComponent::GraphicSetup()
{
	//Auxiliar variables
	float cellWidth = 1.0f / gridWidth;
	float cellHeight = 1.0f / gridHeight;

	//Setup vertices and normals
	UpdateBuffers();

	//Mesh initial setup
	Render *rendererComp = m_owner->GetComponent<Render>();
	Model *model = rendererComp->GetModel();
	delete model->meshes[0];
	model->meshes[0] = new DynamicGridMesh(vertices, normals);
}


void GridWaveComponent::InitGrid()
{
	//Allocate memory
	baseGrid.Init(gridWidth, gridHeight, worldWidth, worldHeight);

	//Init powerGrid
	PowerGrid.resize(gridWidth * gridHeight);

	//Fill it out (Ocean waves method)
	for (int row = 1; row < gridHeight - 1; ++row)
	{
		for (int col = 1; col < gridWidth - 1; ++col)
		{
			std::complex<float> i = std::sqrt(std::complex<float>(-1.0f));
			glm::vec2 k = AuxMath::frequencyVector(baseGrid, row, col);
			glm::vec2 r = AuxMath::SampleGaussian_BoxMuller(1.0f);
			
			float w = glm::length(k) * G;

			std::complex<float> P1 = PowerSpectrum(k);
			std::complex<float> num = (r.x + i * r.y) * std::sqrt(P1);
			std::complex<float> den = 1.41421356237f; // std::sqrt(2);
			std::complex<float> ukt1 = num / den;

			std::complex<float> P2 = PowerSpectrum(-k);
			std::complex<float> num2 = (r.x + i * r.y) * std::sqrt(P2);
			std::complex<float> ukt2 = num2 / den;
			
			baseGrid(col, row) = ukt1 + ukt2;//ukt1 * std::exp(-i*w*0) + ukt2 * std::exp(i*w*0);

			PowerGrid[gridWidth*row + col] = std::make_pair(ukt1, ukt2);
		}
	}

	AuxMath::IFFT2<float> solver((int)std::log2(baseGrid.getWidth()), (int)std::log2(baseGrid.getHeight()));
	solver(baseGrid);
}



std::complex<float> GridWaveComponent::PowerSpectrum(glm::vec2 const& k)
{
	//HARDCODED PARAMS
	float windSpeed = glm::length(wind);
	glm::vec2 windDir = wind / windSpeed;

	float dot = glm::dot(k, windDir);
	std::complex<float> num = phillipAmplitude * std::pow(dot, 2);
	float kmgt = glm::length(k);
	float pow6 = std::pow(kmgt, 6);
	float L = std::pow(windSpeed, 2) / G;
	float e = std::exp(1.0f / (kmgt*kmgt*L*L));
	std::complex<float> denom = pow6 * e;
	
	return num / denom;
}


////////////////////////////////
////	HANDLE INPUT	    ////
////////////////////////////////
void GridWaveComponent::handleInput(float dt)
{
	/// if (inputMgr->getKeyTrigger(SDL_SCANCODE_M))
	/// {
	/// 	this->alpha -= 0.1f;
	/// 	if (alpha < 0)
	/// 		alpha = 2.0f;
	/// 	std::cout << ">> ALPHA CHANGED (gridWaveComp) : " << this->alpha << std::endl;
	/// }
	
	/// if (inputMgr->getKeyTrigger(SDL_SCANCODE_N))
	/// {
	/// 	//Fourier Update
	/// 	float L = 6.0f;
	/// 	AuxMath::DFT_2D(grid, width, height, intermediate, DFTCoeffs, L);
	/// 
	/// 	//intermediate stuff we can do to dft cells
	/// 	for (int row = 0; row < height; ++row)
	/// 	{
	/// 		for (int col = 0; col < width; ++col)
	/// 		{
	/// 			float freq_q = (col < width / 2) ? col / L : (width - col) / L;
	/// 			float freq_p = (row < height / 2) ? row / L : (height - row) / L;
	/// 			float denom = std::sqrt(freq_q*freq_q + freq_p * freq_p);
	/// 			float term = (denom == 0) ? 0.0f : (k / denom);
	/// 			float filter = std::pow(term, alpha / 2.0f);
	/// 			DFTCoeffs[row*width + col] = filter * DFTCoeffs[row*width + col];
	/// 		}
	/// 	}
	/// 
	/// 	AuxMath::IDFT_2D(DFTCoeffs, width, height, intermediate, grid, L);
	/// }
}


