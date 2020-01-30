//////////////////////////////////////////
// HEADER								//
//										//
// Name: Jose Rosenbluth Chiu			//
//                  					//
//////////////////////////////////////////

#version 330 core

layout(std140) uniform test_gUBlock
{
	mat4 ProjView;							// 0   - 64
	mat4 LightProjView;						// 64   -128
	vec4 eye;								// 128  -144
	int ScreenWidth;
	int ScreenHeight;
};

//INPUT BLOCK
in VS_OUT
{
	vec4 normalIn;
};

uniform vec4 diffuseColor;

out vec4 frag_color;


//MAIN
void main(void) 
{		
	frag_color = diffuseColor;
}