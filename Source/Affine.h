#pragma once

#include "../External/Includes/glm/glm.hpp"
#include "../External/Includes/glm/gtc/matrix_transform.hpp"

namespace AuxMath 
{
	//TODO - Make these functions more efficient. Dont make them return glm::mat4

	glm::mat4 scale(float r);

	glm::mat4 scale(float rx, float ry, float rz);

	glm::mat4 translate(const glm::vec4& v);

	glm::mat4 rotate(float t, const glm::vec4& v);

	glm::mat4 rotate(float degree, glm::vec3 const& axis);

	glm::mat4 transpose3x3(const glm::mat4& A);

	glm::vec4 cross(const glm::vec4 &u, const glm::vec4 &v);

	glm::mat4 view(const glm::vec4 &eye, const glm::vec4 &look, const glm::vec4 &rup);

	glm::mat4 perspective(float fov, float aspect, float near, float far);

	glm::mat4 perspective(float fov, float aspect, float near);

	glm::mat4 orthographic(float width, float height, float aspect, float near, float far);
}
