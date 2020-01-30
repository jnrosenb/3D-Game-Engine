///HEADER STUFF

#pragma once

///INCLUDES
#include "../External/Includes/glm/glm.hpp"

class SkyDome;

class Camera 
{
//PUBLIC INTERFACE
public:
	Camera();
	Camera(float near, float far, float fov, 
		int width, int height);
	virtual ~Camera();

	virtual void Update(float dt);
	virtual glm::vec4& getLook();
	virtual glm::mat4& getView();
	virtual glm::mat4& getViewInv();
	virtual glm::mat4& getProj();
	virtual glm::vec4& getEye();
	virtual float getNear();
	virtual float getFar();
	virtual float getFOV();
	virtual float getAspectRatio();

	virtual void setEye(glm::vec3 const& eye);
	virtual void setLook(glm::vec3 const& look);

	virtual SkyDome *GetSkydome();
	virtual void SetSkydome(SkyDome *sky);

private:
	virtual void handleInput(float dt);
	virtual void initCamera();
	virtual void UpdateView(const glm::vec3& rup);
	virtual void UpdatePerspective(float dt);
	virtual void UpdateOrthographic(float dt);

//VARIABLES
private:
	//For switching camera mode
	bool flagOrthographic;

	glm::mat4 m_proj;
	glm::mat4 m_view;
	glm::mat4 m_viewInv;

	glm::vec4 m_eye;
	glm::vec4 m_look;

	float m_near, m_far, m_fov, m_aspectRatio;
	int mViewport_width, mViewport_height;

	//Mouse drag values
	int prevx, prevy;
	bool pressedLastFrame;
	int thresshold;

	//Camera holds a skydome
	SkyDome *sky;

	//Perspective test
	float perspectiveAccumulator;
};