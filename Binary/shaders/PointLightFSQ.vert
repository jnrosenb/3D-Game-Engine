//////////////////////////////////////////
// HEADER								//
//										//
// Name: Jose Rosenbluth Chiu			//
//                  					//
//////////////////////////////////////////

#version 330 core
     
layout(location = 0) in vec4 position;

//Uniforms
uniform mat4 projViewMatrix;
uniform mat4 lightModel;

//Outputs
out vec4 lightWorldPos;

void main()
{
	lightWorldPos = lightModel * position;
	gl_Position = projViewMatrix * lightWorldPos;
};