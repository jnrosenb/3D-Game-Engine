///HEADER STUFF

#pragma once

///Includes
#include <iostream>
#include "Camera.h"
#include "Affine.h"

#include "InputManager.h"

///MACROS
#define EPSILON			0.000001f
#define	DEF_WIDTH		1280
#define DEF_HEIGHT		720
#define DEF_NEAR		0.25f
#define DEF_FAR			1000.0f		//Units still not defined
#define DEF_FOV			75.0f		//In degrees for now

//GLOBALS
glm::vec4 const rup = {0, 1, 0, 0};
//For now, camera will handle all input
extern InputManager *inputMgr;



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
	m_eye = glm::vec4(0, 0, 5, 1);
	m_look = glm::vec4(0, 0, -1, 0);

	//Temporarily do it like this
	m_aspectRatio = static_cast<double>(mViewport_width) / mViewport_height;

	//Mouse drag stuff
	pressedLastFrame = false;
	thresshold = 0;
}


//DESTRUCTOR
Camera::~Camera()
{
}


void Camera::Update(float dt)
{
	handleInput(dt);

	UpdateView(rup);

	if (this->flagOrthographic)
		UpdateOrthographic();
	else
		UpdatePerspective();
}


glm::vec4& Camera::getLook()
{
	return m_look;
}


glm::mat4& Camera::getView()
{
	return m_view;
}


glm::vec4& Camera::getEye() 
{
	return m_eye;
}


glm::mat4& Camera::getProj()
{
	return m_proj;
}


void Camera::UpdateView(const glm::vec4 &rup)
{
	glm::vec4 right, up, u, v, n;
	right = AuxMath::cross(m_look, rup);
	up = AuxMath::cross(right, m_look);

	float rightLength = sqrt(pow(right.x, 2) + pow(right.y, 2) + pow(right.z, 2));
	float upLength = sqrt(pow(up.x, 2) + pow(up.y, 2) + pow(up.z, 2));
	float lookLength = sqrt(pow(m_look.x, 2) + pow(m_look.y, 2) + pow(m_look.z, 2));
	if (fabs(rightLength) <= EPSILON || fabs(upLength) <= EPSILON || fabs(lookLength) <= EPSILON)
	{
		//ERROR, up or right are zero
		std::cout << "Error in view method. Look or rup have zero length" << std::endl;
		m_view = glm::mat4();
	}
	u = { right.x / rightLength, right.y / rightLength, right.z / rightLength, 0 };
	v = { up.x / upLength, up.y / upLength, up.z / upLength, 0 };
	n = { -m_look.x / lookLength, -m_look.y / lookLength, -m_look.z / lookLength, 0 };

	glm::vec3 LtP = {
		-(u.x * m_eye.x) - (u.y * m_eye.y) - (u.z * m_eye.z),
		-(v.x * m_eye.x) - (v.y * m_eye.y) - (v.z * m_eye.z),
		-(n.x * m_eye.x) - (n.y * m_eye.y) - (n.z * m_eye.z),
	};

	m_view =  { u.x, v.x, n.x, 0,
				u.y, v.y, n.y, 0,
				u.z, v.z, n.z, 0,
				LtP.x, LtP.y, LtP.z, 1 };
}


void Camera::UpdatePerspective()
{
	if (fabs(m_near) <= EPSILON || fabs(m_aspectRatio) <= EPSILON || fabs(m_near - m_far) <= EPSILON)
	{
		//TODO see what to return in these cases
		std::cout << "Error in perspective method." << std::endl;
		m_proj = glm::mat4();
	}

	float PI = 4.0f * atan(1.0f);
	float radians = (m_fov * PI) / 180.0f;
	float W = 2 * m_near * (float)tan(radians / 2.0f);
	float H = W / m_aspectRatio;

	float A = (m_near + m_far) / (m_near - m_far);
	float B = (2 * m_near * m_far) / (m_near - m_far);
	float C = (2 * m_near) / W;
	float D = (2 * m_near) / H;
	
	m_proj = {
		C, 0, 0, 0,
		0, D, 0, 0,
		0, 0, A, -1,
		0, 0, B, 0
	};
}


void Camera::UpdateOrthographic()
{
	float w = 20.0f; //TODO - CHANGE - FOR NOW, MAGIC NUMBER
	float h = w / m_aspectRatio;

	float A = -2 / (m_far - m_near);
	float B = (-m_near - m_far) / (m_far - m_near);

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
	float moveSpeed = 50.0f * dt;

	glm::vec4 right = AuxMath::cross(m_look, rup);
	if (inputMgr->getKeyPress(SDL_SCANCODE_RIGHT) || inputMgr->getKeyPress(SDL_SCANCODE_D))
	{
		this->m_eye += moveSpeed * right;
	}
	if (inputMgr->getKeyPress(SDL_SCANCODE_LEFT) || inputMgr->getKeyPress(SDL_SCANCODE_A))
	{
		this->m_eye -= moveSpeed * right;
	}
	if (inputMgr->getKeyPress(SDL_SCANCODE_UP) || inputMgr->getKeyPress(SDL_SCANCODE_W))
	{
		this->m_eye += moveSpeed * m_look;
	}
	if (inputMgr->getKeyPress(SDL_SCANCODE_DOWN) || inputMgr->getKeyPress(SDL_SCANCODE_S))
	{
		this->m_eye -= moveSpeed * m_look;
	}
	if (inputMgr->getKeyPress(SDL_SCANCODE_PAGEDOWN))
	{
		this->m_eye += moveSpeed * rup;
	}
	if (inputMgr->getKeyPress(SDL_SCANCODE_PAGEUP))
	{
		this->m_eye -= moveSpeed * rup;
	}

	//TOGGLE PROJECTION MODE
	if (inputMgr->getKeyTrigger(SDL_SCANCODE_P))
	{
		this->flagOrthographic = !this->flagOrthographic;
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
			right = AuxMath::cross(m_look, rup);
			up = AuxMath::cross(right, m_look);
			glm::normalize(right);
			glm::normalize(up);

			float angle = deltax * 1.0f * ROT_SPEED;
			m_look = AuxMath::rotate(angle, up) * m_look;
			angle = deltay * 1.0f * ROT_SPEED;
			m_look = AuxMath::rotate(angle, right) * m_look;
		}

		pressedLastFrame = true;
		prevx = x;
		prevy = y;
	}
	if (inputMgr->getLeftClickRelease())
		pressedLastFrame = false;
}
