///HEADER STUFF

#pragma once

///INCLUDES
#include "BaseComponent.h"
#include <vector>
#include <string>
#include "Mesh.h"
#include "../External/Includes/glm/glm.hpp"



class ClothComponent : public BaseComponent
{
public:
	friend class Factory;

//PUBLIC INTERFACE
public:
	ClothComponent(GameObject *owner);
	virtual ~ClothComponent();

	virtual ClothComponent* clone() override
	{
		return new ClothComponent(m_owner);
	}
	virtual void Update(float dt) override;
	virtual void LateUpdate(float dt) override;
	virtual void DeserializeInit() override;

private:
	void SendDataToRenderer();
	void CalculateForces();
	void UpdateVertexBuffers();
	void ResetForces();
	void IntegrateForces(float dt);
	void CalculateSpringForces(int x, int y, 
		glm::vec3& force);
	void BuildClothMesh(); 
	void RecalculateNormals();
	glm::vec3 SpringForce(int i, int j, 
		int i2, int j2);
	void Init();

	//Temporary TODO REMOVE
	void handleInput(float dt);

public:
	//To compare when using templates
	static COMPONENT_TYPES const comp_class_type = CLOTH;

private:
	//Physical data. this all should be serialized soon
	float mass;
	float d;
	float stretch;
	float ks;
	float kw;
	int cols;
	int rows; 

	//TODO - serialize
	glm::vec3 diffuseColor;
	glm::vec3 specularColor; 
	int xTiling;
	int yTiling;

	//Arrays of stored data
	std::vector <glm::vec4> vertices;
	std::vector <glm::vec4> normals;
	std::vector <glm::vec2> texCoords;
	std::vector <Mesh::Face> faces;
	std::vector <glm::ivec4> boneIndices;
	std::vector <glm::vec4> boneWeights;

	//VAO, VBO, EBO
	GLuint clothVao;
	GLuint vbo[vbo_index::NUM];
	GLuint ebo;

	//For rigidbody
	std::vector <glm::vec4> oldPos;
	std::vector <glm::vec3> forces;
	std::vector<size_t> inactiveVertices;
};