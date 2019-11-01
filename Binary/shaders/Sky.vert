//////////////////////////////////////////
// HEADER								//
//										//
// Name: Jose Rosenbluth Chiu			//
//                  					//
//////////////////////////////////////////

#version 330 core

layout(location = 0) in vec4 position;
layout(location = 1) in vec4 normal;
layout(location = 2) in vec2 texcoords;

layout(std140) uniform test_gUBlock
{
	mat4 ProjView;							// 0   - 64
	mat4 LightProjView;						// 64   -128
	vec4 eye;								// 128  -144
};

uniform mat4 model;

out VS_OUT
{
	vec4 nml;
	vec2 uv;
};


void main()
{
	vec4 pos = ProjView * model * position;
	gl_Position = pos.xyww;
	nml = normal;
	uv = texcoords;
};