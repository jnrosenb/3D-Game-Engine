//////////////////////////////////////////
// HEADER								//
//										//
// Name: Jose Rosenbluth Chiu			//
//                  					//
//////////////////////////////////////////

#version 330 core
#define MAX_BONES			100
     
layout(location = 0) in vec4 position;
layout(location = 3) in ivec4 bonesInd;
layout(location = 4) in vec4 boneWgts;

layout(std140) uniform test_gUBlock
{
	mat4 ProjView;							// 0   - 64
	mat4 LightProjView;						// 64   -128
	vec4 eye;								// 128  -144
	int ScreenWidth;
	int ScreenHeight;
};

uniform mat4 model;

void main()
{
	gl_Position = ProjView * model * position;
};