//////////////////////////////////////////
// HEADER								//
//										//
// Name: Jose Rosenbluth Chiu			//
//                  					//
//////////////////////////////////////////

#version 330 core
     
layout(location = 0) in vec4 position;

layout(std140) uniform test_gUBlock
{
	mat4 ProjView;							// 0   - 64
	mat4 LightProjView;						// 64   -128
	vec4 eye;								// 128  -144
};

//Uniforms
//uniform mat4 projViewMatrix;
uniform mat4 lightModel;

//Outputs
out vec4 fragmentWorldPos;

void main()
{
	fragmentWorldPos = lightModel * position;
	gl_Position = ProjView * fragmentWorldPos;
};