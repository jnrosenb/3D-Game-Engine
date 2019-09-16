///HEADER STUFF

#pragma once

///INCLUDES
#include "../External/Includes/glm/glm.hpp"


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
	virtual glm::mat4& getProj();
	virtual glm::vec4& getEye();

private:
	virtual void handleInput(float dt);
	virtual void initCamera();
	virtual void UpdateView(const glm::vec4 &rup);
	virtual void UpdatePerspective();
	virtual void UpdateOrthographic();

//VARIABLES
private:
	//For switching camera mode
	bool flagOrthographic;

	glm::mat4 m_proj;
	glm::mat4 m_view;

	glm::vec4 m_eye;
	glm::vec4 m_look;

	float m_near, m_far, m_fov, m_aspectRatio;
	int mViewport_width, mViewport_height;

	//Mouse drag values
	int prevx, prevy;
	bool pressedLastFrame;
	int thresshold;

};