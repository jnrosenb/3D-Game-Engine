//////////////////////////////////////////
// HEADER								//
//										//
// Name: Jose Rosenbluth Chiu			//
//                  					//
//////////////////////////////////////////

#version 330 core

#define MAX_LIGHT_COUNT	8
     
layout(location = 0) in vec4 position;
layout(location = 1) in vec4 normal;
layout(location = 2) in vec2 uv;

layout(std140) uniform test_ubo
{
	mat4 ProjView;							// 0   - 64
	mat4 LightProjView;						// 64   -128
	vec4 eye;								// 128  -144
	vec4 lightColor[MAX_LIGHT_COUNT];		// 144  -272
	vec4 lightPos[MAX_LIGHT_COUNT];			// 272 - 400
	float lightRadius[MAX_LIGHT_COUNT];		// 400 - 528
};

uniform mat4 model;
uniform mat4 normalModel;

//OUTPUT BLOCK
out VS_OUT
{
	vec2 texCoords;
	vec4 normalIn;
	vec4 view_dir;
	vec4 PositionLightSpace;
	vec4 lightToWorld[MAX_LIGHT_COUNT];
};

void main()
{
	vec4 worldPos = model * position;
	gl_Position = ProjView * worldPos;
	normalIn = normalModel * normal;
	texCoords = uv;
	view_dir = eye - worldPos;

	//Shadows
	PositionLightSpace = LightProjView * worldPos;

	for (int i = 0; i < MAX_LIGHT_COUNT; ++i)
	{
		lightToWorld[i] = lightPos[i] - worldPos;
	}
};