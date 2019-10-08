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

uniform mat4 model;
uniform mat4 boneTransform;

void main()
{
	//gl_Position = vec4(boneTransform[3][0], boneTransform[3][1], boneTransform[3][2], 1);
	//gl_Position = ProjView * position;// * vec4(boneTransform[3][0], boneTransform[3][1], boneTransform[3][2], 1);
	gl_Position = ProjView * model * vec4(boneTransform[3][0], boneTransform[3][1], boneTransform[3][2], 1);
};