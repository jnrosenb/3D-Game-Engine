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


uniform mat4 BoneTransf[MAX_BONES]; //MAX_BONES 100
uniform mat4 LightProjView;
uniform mat4 model;


void main()
{
	vec4 localPos = vec4(0, 0, 0, 1);
	if(bonesInd[0] == -1)
		localPos = position;
	if(bonesInd[0] != -1)
		localPos += boneWgts[0] * BoneTransf[bonesInd.x] * position;
	if(bonesInd[1] != -1)
		localPos += boneWgts[1] * BoneTransf[bonesInd.y] * position;
	if(bonesInd[2] != -1)
		localPos += boneWgts[2] * BoneTransf[bonesInd.z] * position;
	if(bonesInd[3] != -1)
		localPos += boneWgts[3] * BoneTransf[bonesInd.w] * position;
		
	gl_Position = LightProjView * model * localPos;
};