//////////////////////////////////////////
// HEADER								//
//										//
// Name: Jose Rosenbluth Chiu			//
//                  					//
//////////////////////////////////////////

#version 330 core

layout (location = 0) out vec4 ShadowBuffer_Moments;

in float NDC_depth;

//MAIN
void main(void) 
{	
	float z = gl_FragCoord.z;
	ShadowBuffer_Moments.x = z;
	ShadowBuffer_Moments.y = z * ShadowBuffer_Moments.x;
	ShadowBuffer_Moments.z = z * ShadowBuffer_Moments.y;
	ShadowBuffer_Moments.w = z * ShadowBuffer_Moments.z;
}

