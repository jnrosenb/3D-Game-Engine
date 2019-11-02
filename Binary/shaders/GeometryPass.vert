//////////////////////////////////////////
// HEADER								//
//										//
// Name: Jose Rosenbluth Chiu			//
//                  					//
//////////////////////////////////////////

#version 330 core

#define MAX_LIGHT_COUNT		8
#define MAX_BONES			100

     
layout(location = 0) in vec4 position;
layout(location = 1) in vec4 normal;
layout(location = 2) in vec2 uv;
layout(location = 3) in ivec4 bonesInd;
layout(location = 4) in vec4 boneWgts;
// (5), (6) will be tgs and bitgs


layout(std140) uniform test_gUBlock
{
	mat4 ProjView;							// 0   - 64
	mat4 LightProjView;						// 64   -128
	vec4 eye;								// 128  -144
};

uniform mat4 model;
uniform mat4 normalModel;
uniform mat4 BoneTransf[MAX_BONES]; //MAX_BONES 100

uniform int xTiling;
uniform int yTiling;

//OUTPUT BLOCK
out VS_OUT
{
	vec4 worldPos;
	vec4 normalIn;
	vec4 view_dir;
	vec2 texCoords;
};

void main()
{
	vec4 localPos = vec4(0, 0, 0, 1);
	vec4 localN = vec4(0, 0, 0, 1);
	if(bonesInd[0] == -1)
	{
		localPos = position;
		localN = normal;
	}
	if(bonesInd[0] != -1)
	{
		localPos += boneWgts[0] * BoneTransf[bonesInd.x] * position;
		localN += boneWgts[0] * BoneTransf[bonesInd.x] * normal;
	}
	if(bonesInd[1] != -1)
	{
		localPos += boneWgts[1] * BoneTransf[bonesInd.y] * position;
		localN += boneWgts[1] * BoneTransf[bonesInd.y] * normal;
	}
	if(bonesInd[2] != -1)
	{
		localPos += boneWgts[2] * BoneTransf[bonesInd.z] * position;
		localN += boneWgts[2] * BoneTransf[bonesInd.z] * normal;
	}
	if(bonesInd[3] != -1)
	{
		localPos += boneWgts[3] * BoneTransf[bonesInd.w] * position;
		localN += boneWgts[3] * BoneTransf[bonesInd.w] * normal;
	}
	//localPos.w = 1;
	//localN.w = 1;

	worldPos = model * localPos;
	gl_Position = ProjView * worldPos;
	
	normalIn = normalModel * localN;
	texCoords = vec2(uv.x * xTiling, uv.y * yTiling);
	view_dir = eye - worldPos;
};