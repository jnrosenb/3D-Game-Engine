///HEADER STUFF

#include <iostream>
#include "ClothComponent.h"
#include "GameObject.h"

#include "Quaternion.h"
#include "Affine.h"
#include "Plane.h"
#include "TransformComponent.h"

// TODO Temporary (while no world exists)
#include "Renderer.h"
extern Renderer *renderer;

// TODO Temporary (while no world exists)
#include "InputManager.h"
extern InputManager *inputMgr;


#define EPSILON			0.0000001f
#define PI				3.14159f
#define G				9.8f


ClothComponent::ClothComponent(GameObject *owner) :
	BaseComponent(owner, COMPONENT_TYPES::CLOTH)
{
}

ClothComponent::~ClothComponent()
{
	std::cout << "Destroying Cloth Component" << std::endl;

	glDeleteVertexArrays(1, &clothVao);
	glDeleteBuffers(vbo_index::NUM, vbo);
	glDeleteBuffers(1, &ebo);
}


void ClothComponent::DeserializeInit()
{
	//Setup stuff like the shader (?) diffuse tex or color, etc
	diffuseColor = glm::vec3(0, 1, 0);
	specularColor = glm::vec3(1, 1, 1);
	xTiling = 1.0;
	yTiling = 1.0;

	//Using cols, rows and d, build the plane object
	BuildClothMesh();

	//Set the right size for the different arrays
	forces.resize(vertices.size(), glm::vec3(0.0f));
}


void ClothComponent::Update(float dt)
{
	//Reset forces (now done after applying force)
	///ResetForces();

	//TODO temporal
	handleInput(dt);

	//For x reason, we'll do this here
	CalculateForces();

	//Integrate
	IntegrateForces(dt);

	//Recalculate normals (for now, here)
	RecalculateNormals();

	//Update gl buffers
	UpdateVertexBuffers();
}


void ClothComponent::LateUpdate(float dt)
{
	//Send data to be drawn
	SendDataToRenderer();
}


void ClothComponent::IntegrateForces(float dt)
{
	//Iterate through all vertices (particles)
	for (int y = 0; y < rows; ++y)
	{
		for (int x = 0; x < cols; ++x)
		{
			glm::vec3& f = forces[y*cols + x];
			glm::vec3 a = f / mass;

			//Using a fixed timestep
			float DT = 0.192f;

			//New integration
			glm::vec4 temp = vertices[y*cols + x];
			vertices[y*cols + x] = vertices[y*cols + x] + (vertices[y*cols + x] - oldPos[y*cols + x])*0.25f + glm::vec4(a, 0.0f) * powf(DT, 2);
			oldPos[y*cols + x] = temp;

			//Reset the force on this particle
			f = glm::vec3(0.0f);
		}
	}
}


void ClothComponent::ResetForces() 
{
	//Iterate through all vertices (particles)
	for (int y = 0; y < rows; ++y)
	{
		for (int x = 0; x < cols; ++x)
		{
			forces[y*cols + x] = glm::vec3(0.0f);
		}
	}
}


void ClothComponent::CalculateForces() 
{
	//Relevant forces per particle are
	// 1- The 12 spring forces. Take care not to get out of bounds
	// 2- Gravity Force
	// 3- Wind force (dot product between wind dir and vertex normal)
	// 4- FOR LATER - restitution forces

	
	//Iterate through all vertices (particles)
	for (int y = 0; y < rows; ++y) 
	{
		for (int x = 0; x < cols; ++x)
		{
			// TODO - remove this! - For sake of experiment, skip four borders
			if (x == 0)// && y == 0 || x == cols-1 && y == 0)
				continue;

			//Get force vector's reference
			glm::vec3& f = forces[y*cols + x];

			//Gravity forces
			Transform *T = m_owner->GetComponent<Transform>();
			glm::vec4 objSpaceMG = glm::inverse(T->GetModel()) * glm::vec4(0.0f, -(mass*G), 0.0f, 0.0f);
			f += glm::vec3(objSpaceMG);

			//Spring forces
			CalculateSpringForces(x, y, f);

			//Damping force

			//Wind force
		}
	}
}


void ClothComponent::CalculateSpringForces(int x, int y, glm::vec3& force)
{
	// In this case, we can check every vertex around the current one
	if (x > 0 && y > 0 && x < cols-1 && y < rows-1)
	{
		//Straigh lines
		force += SpringForce(y, x, y-1, x);
		force += SpringForce(y, x, y + 1, x);
		force += SpringForce(y, x, y, x-1);
		force += SpringForce(y, x, y, x + 1);

		//Diagonals
		force += SpringForce(y, x, y + 1, x + 1);
		force += SpringForce(y, x, y + 1, x - 1);
		force += SpringForce(y, x, y - 1, x + 1);
		force += SpringForce(y, x, y - 1, x - 1);

		//In this case, add the bending forces
		if (x > 1 && y > 1 && x < cols - 2 && y < rows - 2) 
		{
			force += SpringForce(y, x, y - 2, x);
			force += SpringForce(y, x, y + 2, x);
			force += SpringForce(y, x, y, x - 2);
			force += SpringForce(y, x, y, x + 2);
		}
	}
	//Upper-Right corner
	else if (x == cols-1 && y == rows-1)
	{
		//Straigh lines
		force += SpringForce(y, x, y - 1, x);
		force += SpringForce(y, x, y, x - 1);
		//Diagonals
		force += SpringForce(y, x, y - 1, x - 1);
		//Two away
		force += SpringForce(y, x, y - 2, x);
		force += SpringForce(y, x, y, x - 2);
	}
	//Upper-Left corner
	else if (x == 0 && y == rows-1)
	{
		//Straigh lines
		force += SpringForce(y, x, y - 1, x);
		force += SpringForce(y, x, y, x + 1);
		//Diagonals
		force += SpringForce(y, x, y - 1, x + 1);
		//Two away
		force += SpringForce(y, x, y - 2, x);
		force += SpringForce(y, x, y, x + 2);
	}
	//Bottom-Right corner
	else if (x == cols-1 && y == 0)
	{
		//Straigh lines
		force += SpringForce(y, x, y + 1, x);
		force += SpringForce(y, x, y, x - 1);
		//Diagonals
		force += SpringForce(y, x, y + 1, x - 1);
		//Two away
		force += SpringForce(y, x, y + 2, x);
		force += SpringForce(y, x, y, x - 2);
	}
	//Bottom-Left corner
	else if (x == 0 && y == 0)
	{
		//Straigh lines
		force += SpringForce(y, x, y + 1, x);
		force += SpringForce(y, x, y, x + 1);
		//Diagonals
		force += SpringForce(y, x, y + 1, x + 1);
		//Two away
		force += SpringForce(y, x, y + 2, x);
		force += SpringForce(y, x, y, x + 2);
	}
	//Left border
	else if (x == 0)
	{
		//Straigh lines
		force += SpringForce(y, x, y - 1, x);
		force += SpringForce(y, x, y + 1, x);
		force += SpringForce(y, x, y, x + 1);
		//Diagonals
		force += SpringForce(y, x, y + 1, x + 1);
		force += SpringForce(y, x, y - 1, x + 1);
		//Two away
		force += SpringForce(y, x, y, x + 2);
		if (y > 1 && y < rows - 2)
		{
			force += SpringForce(y, x, y - 2, x);
			force += SpringForce(y, x, y + 2, x);
		}
	}
	//Right border
	else if (x == cols-1)
	{
		//Straigh lines
		force += SpringForce(y, x, y - 1, x);
		force += SpringForce(y, x, y + 1, x);
		force += SpringForce(y, x, y, x - 1);

		//Diagonals
		force += SpringForce(y, x, y + 1, x - 1);
		force += SpringForce(y, x, y - 1, x - 1);
		//Two away
		force += SpringForce(y, x, y, x - 2);
		if (y > 1 && y < rows - 2)
		{
			force += SpringForce(y, x, y - 2, x);
			force += SpringForce(y, x, y + 2, x);
		}
	}
	//Bottom border
	else if (y == 0) 
	{
		//Straigh lines
		force += SpringForce(y, x, y + 1, x);
		force += SpringForce(y, x, y, x - 1);
		force += SpringForce(y, x, y, x + 1);

		//Diagonals
		force += SpringForce(y, x, y + 1, x + 1);
		force += SpringForce(y, x, y + 1, x - 1);
		//Two away
		force += SpringForce(y, x, y + 2, x);
		if (x > 1 && x < cols - 2)
		{
			force += SpringForce(y, x, y, x - 2);
			force += SpringForce(y, x, y, x + 2);
		}
	}
	//Top border
	else if (y == rows-1) 
	{
		//Straigh lines
		force += SpringForce(y, x, y - 1, x);
		force += SpringForce(y, x, y, x - 1);
		force += SpringForce(y, x, y, x + 1);

		//Diagonals
		force += SpringForce(y, x, y - 1, x + 1);
		force += SpringForce(y, x, y - 1, x - 1);
		//Two away
		force += SpringForce(y, x, y - 2, x);
		if (x > 1 && x < cols - 2)
		{
			force += SpringForce(y, x, y, x - 2);
			force += SpringForce(y, x, y, x + 2);
		}
	}
}


void ClothComponent::BuildClothMesh() 
{
	//Not the most ideal, but will do for now

	//Vertices initial setting. There should be cols rigidbodies 
	//time rows bodies, each separated by rest distance d
	float mesh_halfWidth = stretch * 0.5f * (cols - 1) * d;
	float mesh_halfHeight = stretch * 0.5 * (rows - 1) * d;
	for (int i = 0; i < rows; ++i)
	{
		float y = mesh_halfHeight - i * d;
		for (int j = 0; j < cols; ++j)
		{
			float x = j * d - mesh_halfWidth;
			int index = (cols)*i + j;
			vertices.push_back(glm::vec4(x, y, 0.0f, 1.0f));
			oldPos.push_back(glm::vec4(x, y, 0.0f, 1.0f));
		}
	}

	//Normals setup
	for (int i = 0; i < vertices.size(); ++i)
	{
		normals.push_back(glm::vec4(0.0f, 0.0f, 1.0f, 0.0f));
	}

	//Faces setup
	//The minus one on the limits is so it doesnt create extra faces
	//(Extra faces on the bottom and right of the mesh plane)
	for (int i = 0; i < rows - 1; ++i)
	{
		for (int j = 0; j < cols - 1; ++j)
		{
			Mesh::Face poly01;
			poly01[0] = (cols)*(i) + (j);
			poly01[2] = (cols)*(i + 1) + (j + 1);
			poly01[1] = (cols)*(i) + (j + 1);
			faces.push_back(poly01);

			Mesh::Face poly02;
			poly02[1] = (cols)*(i + 1) + (j + 1);
			poly02[0] = (cols)*(i) + (j);
			poly02[2] = (cols)*(i + 1) + (j);
			faces.push_back(poly02);
		}
	}

	//Set UV coords
	for (int i = 0; i < vertices.size(); ++i)
	{
		float x = vertices[i].x;
		float y = vertices[i].y;
		glm::vec2 stdVertex = glm::vec2(x / mesh_halfWidth, y / mesh_halfHeight);
		glm::vec2 uv = 0.5f * (stdVertex + glm::vec2(1.0f, 1.0f));
		texCoords.push_back(uv);
	}

	//Set all bone indices to -1
	for (int i = 0; i < vertices.size(); ++i)
	{
		glm::ivec4 ind = glm::ivec4(-1, -1, -1, -1);
		this->boneIndices.push_back(ind);
	}

	//Init opengl buffers
	this->Init();
}


void ClothComponent::Init()
{
	//Generate VAO, VBO and EBO
	glGenVertexArrays(1, &this->clothVao);
	glGenBuffers(vbo_index::NUM, this->vbo);
	glGenBuffers(1, &this->ebo);

	//Bins the VAO
	glBindVertexArray(this->clothVao);

	//Allocate in gpu
	glBindBuffer(GL_ARRAY_BUFFER, this->vbo[VERTICES]);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * 4 * sizeof(GLfloat), &vertices[0][0], GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, this->vbo[NORMALS]);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * 4 * sizeof(GLfloat), &normals[0][0], GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, this->vbo[TEXCOORDS]);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * 2 * sizeof(GLfloat), &texCoords[0][0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, this->vbo[BONE_INDICES]);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * 4 * sizeof(GLint), &boneIndices[0][0], GL_STATIC_DRAW);
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, faces.size() * sizeof(Mesh::Face), &faces[0][0], GL_STATIC_DRAW);

	//PASS ATTRIBUTES AND ENABLE
	glBindBuffer(GL_ARRAY_BUFFER, this->vbo[VERTICES]);
	glVertexAttribPointer(0, 4, GL_FLOAT, false, 4 * sizeof(GLfloat), (void*)0);

	glBindBuffer(GL_ARRAY_BUFFER, this->vbo[NORMALS]);
	glVertexAttribPointer(1, 4, GL_FLOAT, false, 4 * sizeof(GLfloat), (void*)0);

	glBindBuffer(GL_ARRAY_BUFFER, this->vbo[TEXCOORDS]);
	glVertexAttribPointer(2, 2, GL_FLOAT, false, 2 * sizeof(GLfloat), (void*)0);

	glBindBuffer(GL_ARRAY_BUFFER, this->vbo[BONE_INDICES]);
	glVertexAttribIPointer(3, 4, GL_INT, 4 * sizeof(GLint), (void*)0);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);

	//Unbind Everything
	glBindVertexArray(0);
}


void ClothComponent::UpdateVertexBuffers()
{
	//Allocate in gpu
	glBindBuffer(GL_ARRAY_BUFFER, this->vbo[VERTICES]);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * 4 * sizeof(GLfloat), &vertices[0][0], GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, this->vbo[NORMALS]);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * 4 * sizeof(GLfloat), &normals[0][0], GL_DYNAMIC_DRAW);
	
	//Unbind vertex buffer
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void ClothComponent::SendDataToRenderer() 
{
	// Pass shader (material) and mesh (model) info to GraphicsManager
	Transform *T = this->m_owner->GetComponent<Transform>();

	// Pack it maybe on a struct
	DrawData data = {};

	//Only clothing uses these stuff
	data.vao = this->clothVao;
	data.faceSize = this->faces.size();
	data.isCloth = true;
	
	//Other common data
	data.model = T->GetModel();
	data.normalsModel = T->GetNormalModel();
	///data.diffuseTexture = renderer->GetTexture(this->diffuseTexture);
	data.useDiffuseTexture = false;
	data.xTiling = this->xTiling;
	data.yTiling = this->yTiling;
	data.diffuseColor = glm::vec4(this->diffuseColor, 1);
	data.specularColor = glm::vec4(this->specularColor, 1);
	data.BoneTransformations = 0;
	data.boneCount = 0;
	data.roughness = 1.0f;
	data.metallic = 0.0f;

	// Pass it to renderer's queue
	renderer->QueueForDraw(data);
}


void ClothComponent::RecalculateNormals() 
{
	for (auto face : faces) 
	{
		glm::vec3 A = static_cast<glm::vec3>(vertices[face[0]]);
		glm::vec3 B = static_cast<glm::vec3>(vertices[face[1]]);
		glm::vec3 C = static_cast<glm::vec3>(vertices[face[2]]);

		//Normal for A - CxB
		glm::vec3 cross = glm::normalize(glm::cross(C, B));
		normals[face[0]] = glm::vec4(cross, 0.0f);

		//Normal for B - AxC
		cross = glm::normalize(glm::cross(A, C));
		normals[face[1]] = glm::vec4(cross, 0.0f);

		//Normal for C - BxA
		cross = glm::normalize(glm::cross(B, A));
		normals[face[2]] = glm::vec4(cross, 0.0f);
	}
}


glm::vec3 ClothComponent::SpringForce(int i, int j, int i2, int j2) 
{
	glm::vec3 A = vertices[(i)*cols + j];
	glm::vec3 B = vertices[(i2)*cols + j2];
	
	glm::vec3 dif = (B - A);
	float len = glm::length(dif);
	float restLen = sqrtf(powf(i2-i, 2) + powf(j2-j, 2)) * d;

	if (abs(len - restLen) <= EPSILON) 
	{
		return glm::vec3(0.0f);
	}
	else 
	{
		float coeff = ks * (len - restLen) * 0.5f;
		glm::vec3 scaledNorm = coeff * glm::normalize(dif);
		return scaledNorm;
	}
}


////////////////////////////////
////	HANDLE INPUT	    ////
////////////////////////////////
void ClothComponent::handleInput(float dt)
{
	float moveSpeed = 100.0f * dt;

	Transform *T = m_owner->GetComponent<Transform>();
	glm::vec4 objSpaceRIGHT = glm::inverse(T->GetModel()) * glm::vec4(moveSpeed, 0.0f, 0.0f, 0.0f);
	glm::vec4 objSpaceUP = glm::inverse(T->GetModel()) * glm::vec4(0.0f, moveSpeed, 0.0f, 0.0f);

	if (inputMgr->getKeyPress(SDL_SCANCODE_RIGHT))
	{
		for (int y = 0; y < rows; ++y)
			this->vertices[y*cols] = this->vertices[y*cols] + objSpaceRIGHT;
	}
	if (inputMgr->getKeyPress(SDL_SCANCODE_LEFT))
	{
		for (int y = 0; y < rows; ++y)
			this->vertices[y*cols] = this->vertices[y*cols] - objSpaceRIGHT;
	}
	if (inputMgr->getKeyPress(SDL_SCANCODE_UP))
	{
		for (int y = 0; y < rows; ++y)
			this->vertices[y*cols] = this->vertices[y*cols] + objSpaceUP;
	}
	if (inputMgr->getKeyPress(SDL_SCANCODE_DOWN))
	{
		for (int y = 0; y < rows; ++y)
			this->vertices[y*cols] = this->vertices[y*cols] - objSpaceUP;
	}
}