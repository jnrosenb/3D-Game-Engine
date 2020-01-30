//////////////////////////////////////////
// HEADER								//
//										//
// Name: Jose Rosenbluth Chiu			//
//                  					//
//////////////////////////////////////////

#version 330 core

#define MAX_LIGHT_COUNT		8
     
layout(location = 0) in vec4 position;
layout(location = 1) in vec4 normal;
layout(location = 2) in vec2 uv;
layout(location = 3) in mat4 instancedModel; //loc 3-4-5-6

layout(std140) uniform test_gUBlock
{
	mat4 ProjView;							// 0   - 64
	mat4 LightProjView;						// 64   -128
	vec4 eye;								// 128  -144
	int ScreenWidth;
	int ScreenHeight;
};

uniform int xTiling;
uniform int yTiling;

//OUTPUT BLOCK
out VS_OUT
{
	vec4 worldPos;
	vec4 normalIn;
	vec2 texCoords;
};

void main()
{
	worldPos = instancedModel * position;
	gl_Position = ProjView * worldPos;

	vec3 N = normalize(normal.xyz);
	normalIn = instancedModel * vec4(N, 0);

	texCoords = vec2(uv.x * xTiling, uv.y * yTiling);
};