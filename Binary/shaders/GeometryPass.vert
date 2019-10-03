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
// (3), (4) are tgs and bi_tgs
layout(location = 3) in ivec4 bonesInd;
layout(location = 4) in vec4 boneWgts;


layout(std140) uniform test_gUBlock
{
	mat4 ProjView;							// 0   - 64
	mat4 LightProjView;						// 64   -128
	vec4 eye;								// 128  -144
};

uniform mat4 model;
uniform mat4 normalModel;
uniform mat4 BoneTransf[MAX_BONES]; //MAX_BONES 100

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
	if(bonesInd[0] == -1)
	{
		localPos = position;
	}
	else
	{
		for (int i = 0; i < 4; i++)
		{
			int index = bonesInd[i];
			if (index == -1) break;

			vec4 boneshit = BoneTransf[index] * position;
			localPos += boneWgts[i] * boneshit;
		}
	}

	worldPos = model * localPos;
	gl_Position = ProjView * worldPos;
	
	normalIn = normalModel * normal;
	texCoords = uv;
	view_dir = eye - worldPos;
};