///HEADER STUFF

#pragma once

///Includes
#include <iostream>
#include "Camera.h"
#include "Affine.h"
#include "Renderer.h"

//For now, camera will handle all input
#include "InputManager.h"
extern InputManager *inputMgr;

///MACROS
#define PI				3.1415926535f
#define EPSILON			0.000001f
#define	DEF_WIDTH		1280
#define DEF_HEIGHT		720
#define DEF_NEAR		0.1f
#define DEF_FAR			1000.0f		//Units still not defined
#define DEF_FOV			55.0f		//In degrees for now

//GLOBALS
glm::vec3 const rup = {0, 1, 0};
#include "DeferredRenderer.h"
extern Renderer *renderer;



//CONSTRUCTOR
Camera::Camera() : m_near(DEF_NEAR), m_far(DEF_FAR), 
	m_fov(DEF_FOV), mViewport_width(DEF_WIDTH), 
	mViewport_height(DEF_HEIGHT)
{
	//TODO
	initCamera();
}


Camera::Camera(float near, float far, float fov,
	int width, int height) : m_near(near), m_far(far),
	m_fov(fov), mViewport_width(width), mViewport_height(height)
{
	//TODO
	initCamera();
}


void Camera::initCamera() 
{
	this->flagOrthographic = false;

	//Look and eye
	m_eye = glm::vec4(0, 5, 15, 1);
	m_look = glm::vec4(0, -0.25, -1, 0);
	m_look = glm::normalize(m_look);

	//Temporarily do it like this
	m_aspectRatio = static_cast<double>(mViewport_width) / mViewport_height;

	//Mouse drag stuff
	pressedLastFrame = false;
	thresshold = 0;

	//Perspective test
	perspectiveAccumulator = 0.0f;
}


//DESTRUCTOR
Camera::~Camera()
{
	delete sky;
}


void Camera::Update(float dt)
{
	handleInput(dt);

	UpdateView(rup);

	if (this->flagOrthographic)
		UpdateOrthographic(dt);
	else
		UpdatePerspective(dt);

	this->sky->model = AuxMath::translate(this->m_eye);
}


glm::vec4& Camera::getLook()
{
	return m_look;
}


glm::mat4& Camera::getView()
{
	return m_view;
}


glm::mat4& Camera::getViewInv()
{
	return m_viewInv;
}


glm::vec4& Camera::getEye() 
{
	return m_eye;
}

void Camera::setEye(glm::vec3 const& eye)
{
	this->m_eye = glm::vec4(eye, 1.0f);
}

void Camera::setLook(glm::vec3 const& look)
{
	this->m_look = glm::vec4(look, 0.0f);
}

float Camera::getNear()
{
	return this->m_near;
}

float Camera::getFar()
{
	return this->m_far;
}

float Camera::getFOV()
{
	return this->m_fov;
}

float Camera::getAspectRatio()
{
	return this->m_aspectRatio;
}

glm::mat4& Camera::getProj()
{
	return m_proj;
}


void Camera::SetSkydome(SkyDome *sky)
{
	this->sky = sky;
}

SkyDome *Camera::GetSkydome() 
{
	return this->sky;
}


void Camera::UpdateView(const glm::vec3 &rup)
{
	glm::vec3 look, right, up, u, v, n;
	look = glm::vec3(m_look.x, m_look.y, m_look.z);
	right = glm::cross(look, rup);
	up = glm::cross(right, look);

	//normalize the stuff
	u = glm::normalize(right);
	v = glm::normalize(up);
	n = -look;
	
	//Not sure if keeping this - This error will happen if m_look is parallel to rup
	if (u == glm::vec3(0) || v == glm::vec3(0) || n == glm::vec3(0))
	{
		//ERROR, up or right are zero
		std::cout << "ERROR in view method. Look or rup have zero length" << std::endl;
		m_view = glm::mat4();
		m_viewInv = glm::mat4();
	}

	//This is the transposed cam R (world to cam, so transposed of the camera's model R, which goes from cam to world) 
	//applied on top of the inverse T (world to cam, so -eye)
	glm::vec3 LtP = {
		(u.x * -m_eye.x) + (u.y * -m_eye.y) + (u.z * -m_eye.z),
		(v.x * -m_eye.x) + (v.y * -m_eye.y) + (v.z * -m_eye.z),
		(n.x * -m_eye.x) + (n.y * -m_eye.y) + (n.z * -m_eye.z),
	};

	m_view =  { 
		u.x, v.x, n.x, 0,
		u.y, v.y, n.y, 0,
		u.z, v.z, n.z, 0,
		LtP.x, LtP.y, LtP.z, 1 
	};

	m_viewInv = {
		u.x, u.y, u.z, 0,
		v.x, v.y, v.z, 0,
		n.x, n.y, n.z, 0,
		m_eye.x, m_eye.y, m_eye.z, 1
	};
}


void Camera::UpdatePerspective(float dt)
{
	if (fabs(m_near) <= EPSILON || fabs(m_aspectRatio) <= EPSILON || fabs(m_near - m_far) <= EPSILON)
	{
		//TODO see what to return in these cases
		std::cout << "Error in perspective method." << std::endl;
		m_proj = glm::mat4();
	}

	float radians = (m_fov * PI) / 180.0f;
	float W = 2 * m_near * std::tanf(radians / 2.0f);
	float H = W / m_aspectRatio;

	float A = (m_near + m_far) / (m_near - m_far);
	float B = (2 * m_near * m_far) / (m_near - m_far);
	float C = (2 * m_near) / W;
	float D = (2 * m_near) / H;
	
	//Perspective test
	perspectiveAccumulator += dt * 0.1f;

	m_proj = {
		C, 0, 0, 0,
		0, D, 0, 0,
		0, 0, A, -1,
		0, 0, B, 0
	};
}


void Camera::UpdateOrthographic(float dt)
{
	float w = 20.0f; //TODO - CHANGE - FOR NOW, MAGIC NUMBER
	float h = w / m_aspectRatio;

	float A = -2 / (m_far - m_near);
	float B = (-m_near - m_far) / (m_far - m_near);

	//Perspective test
	perspectiveAccumulator += dt * 0.1f;

	m_proj = {
		2 / w, 0, 0, 0,
		0, 2 / h, 0, 0,
		0, 0, A, 0,
		0, 0, B, 1
	};
}


////////////////////////////////
////		HANDLE INPUT	////
////////////////////////////////
#define ROT_SPEED	0.075f
void Camera::handleInput(float dt)
{
	float moveSpeed = 100.0f * dt;

	glm::vec4 right = glm::normalize(AuxMath::cross(m_look, glm::vec4(0, 1, 0, 0)));
	if (/*inputMgr->getKeyPress(SDL_SCANCODE_RIGHT) ||  */inputMgr->getKeyPress(SDL_SCANCODE_D))
	{
		this->m_eye += moveSpeed * right;
	}
	if (/*inputMgr->getKeyPress(SDL_SCANCODE_LEFT) || */ inputMgr->getKeyPress(SDL_SCANCODE_A))
	{
		this->m_eye -= moveSpeed * right;
	}
	if (/*inputMgr->getKeyPress(SDL_SCANCODE_UP) || */ inputMgr->getKeyPress(SDL_SCANCODE_W))
	{
		this->m_eye += moveSpeed * m_look;
	}
	if (/*inputMgr->getKeyPress(SDL_SCANCODE_DOWN) || */ inputMgr->getKeyPress(SDL_SCANCODE_S))
	{
		this->m_eye -= moveSpeed * m_look;
	}
	/*
	if (inputMgr->getKeyPress(SDL_SCANCODE_PAGEDOWN))
	{
		this->m_eye += moveSpeed * rup;
	}
	if (inputMgr->getKeyPress(SDL_SCANCODE_PAGEUP))
	{
		this->m_eye -= moveSpeed * rup;
	}
	//*/

	//TOGGLE PROJECTION MODE
	if (inputMgr->getKeyTrigger(SDL_SCANCODE_P))
	{
		this->flagOrthographic = !this->flagOrthographic;
	}


	//TOGGLE SKELETON DRAWING
	if (inputMgr->getKeyTrigger(SDL_SCANCODE_Q))
	{
		if (renderer->anisoLevel > 0.0f)
			renderer->anisoLevel--;
		std::cout << "ANISO LEVEL NOW" << renderer->anisoLevel << std::endl;
	}
	if (inputMgr->getKeyTrigger(SDL_SCANCODE_E))
	{
		if (renderer->anisoLevel < renderer->GetMaxAnisotropicLevel())
			renderer->anisoLevel++;
		std::cout << "ANISO LEVEL NOW" << renderer->anisoLevel << std::endl;
	}

	if (inputMgr->getKeyTrigger(SDL_SCANCODE_TAB))
	{
		DeferredRenderer *rend = static_cast<DeferredRenderer*>(renderer);
		rend->toggleDebugPass();
		std::cout << "DEBUG DRAW TOGGLED!" << std::endl;
	}

	if (inputMgr->getKeyTrigger(SDL_SCANCODE_F8))
	{
		DeferredRenderer *rend = static_cast<DeferredRenderer*>(renderer);
		rend->toggleDebugViewMode();
		std::cout << "DEBUG VIEW MODE TOGGLED!" << std::endl;
	}
	if (inputMgr->getKeyTrigger(SDL_SCANCODE_F9))
	{
		DeferredRenderer *rend = static_cast<DeferredRenderer*>(renderer);
		rend->toggleVisualCascades();
		std::cout << "VISUAL CASCADES TOGGLED!" << std::endl;
	}


	//MSAA ON
	if (inputMgr->getKeyTrigger(SDL_SCANCODE_T))
	{
		renderer->ToggleMSAA();
		std::cout << "MSAA ON: " << renderer->IsMSAAOn() << std::endl;
	}



	//Increase, decrease deferredRenderer kernel size
	if (inputMgr->getKeyTrigger(SDL_SCANCODE_I))
	{
		///std::cout << "KERNEL COUNT NOW" << renderer->GetKernelCount() << std::endl;
	}
	if (inputMgr->getKeyTrigger(SDL_SCANCODE_O))
	{
		///std::cout << "KERNEL COUNT NOW : " << renderer->GetKernelCount() << std::endl;
	}

	//MOUSE DRAG STUFF
	if (inputMgr->getLeftClickPress())
	{
		int x = inputMgr->getMouseX();
		int y = inputMgr->getMouseY();

		if (pressedLastFrame)
		{
			//Compare against
			int deltax = x - prevx;
			int deltay = y - prevy;

			glm::vec4 right, up;
			right = AuxMath::cross(m_look, glm::vec4(0, 1, 0, 0));
			up = AuxMath::cross(right, m_look);
			glm::normalize(right);
			glm::normalize(up);

			float angle = deltax * 1.0f * ROT_SPEED;
			m_look = AuxMath::rotate(angle, up) * m_look;
			angle = deltay * 1.0f * ROT_SPEED;
			m_look = AuxMath::rotate(angle, right) * m_look;
			m_look = glm::normalize(m_look);
		}

		pressedLastFrame = true;
		prevx = x;
		prevy = y;
	}
	if (inputMgr->getLeftClickRelease())
		pressedLastFrame = false;
}
