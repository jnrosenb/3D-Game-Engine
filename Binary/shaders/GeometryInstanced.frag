//////////////////////////////////////////
// HEADER								//
//										//
// Name: Jose Rosenbluth Chiu			//
//                  					//
//////////////////////////////////////////

#version 330 core


layout (location = 0) out vec4 GBuffer_pos;
layout (location = 1) out vec4 GBuffer_normals;
layout (location = 2) out vec4 GBuffer_diffk;
layout (location = 3) out vec4 GBuffer_speck_gloss; 


layout(std140) uniform test_gUBlock
{
	mat4 ProjView;							// 0   - 64
	mat4 LightProjView;						// 64   -128
	vec4 eye;								// 128  -144
	int ScreenWidth;
	int ScreenHeight;
};

uniform sampler2D diffuseTexture;
uniform vec4 diffuseColor;
uniform int useDiffuseTexture;


//INPUT BLOCK
in VS_OUT
{
	vec4 worldPos;
	vec4 normalIn;
	vec2 texCoords;
};


//MAIN
void main(void) 
{		
	//WORLD POSITION
	GBuffer_pos = worldPos;
	GBuffer_normals = normalIn;
	
	//DIFFUSE AND METALLIC
	if (useDiffuseTexture == 1)
	{
		vec3 diffuse_k = texture(diffuseTexture, texCoords).xyz;
		diffuse_k *= diffuseColor.rgb;
		GBuffer_diffk = vec4(diffuse_k, 0.0f);
	}
	else
	{
		GBuffer_diffk = vec4(diffuseColor.rgb, 0.0f);
	}

	//SPECULAR AND ROUGHNESS
	GBuffer_speck_gloss = vec4(1.0f, 1.0f, 1.0f, 1.0f);
}

