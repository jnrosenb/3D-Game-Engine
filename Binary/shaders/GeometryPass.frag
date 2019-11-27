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
uniform sampler2D metallicTexture;
uniform sampler2D roughnessTexture;
uniform sampler2D normalMap;

uniform vec4 diffuseColor;
uniform vec4 specularColor;
uniform float roughness;
uniform float metallic;
uniform int useDiffuseTexture;
uniform int useNormalMap;


//INPUT BLOCK
in VS_OUT
{
	vec4 worldPos;
	vec4 normalIn;
	vec2 texCoords;
	mat3 TBN;
};


//MAIN
void main(void) 
{		
	//WORLD POSITION
	//if (gl_FragCoord.w == 0.0)
	//	discard;
	GBuffer_pos = worldPos;

	//NORMALS
	if (useNormalMap == 1)
	{
		vec4 normal_from_rgb = texture(normalMap, texCoords);
		normal_from_rgb = 2.0 * normal_from_rgb - 1.0; 
		normal_from_rgb = vec4(TBN * normal_from_rgb.rgb, 0);
		GBuffer_normals = normal_from_rgb;//normalIn;
	}
	else
	{
		GBuffer_normals = normalIn;
	}

	//ROUGHNESS - METALLIC
	float metal = metallic;
	float rough = roughness;
	if (metallic == -1)
		metal = texture(metallicTexture, texCoords).x;
	if (roughness == -1)
		rough = texture(roughnessTexture, texCoords).x;
	
	//DIFFUSE AND METALLIC
	if (useDiffuseTexture == 1)
	{
		vec3 diffuse_k = texture(diffuseTexture, texCoords).xyz;
		diffuse_k *= diffuseColor.rgb;
		GBuffer_diffk = vec4(diffuse_k, metal);
	}
	else
	{
		GBuffer_diffk = vec4(diffuseColor.rgb, metal);
	}

	//SPECULAR AND ROUGHNESS
	GBuffer_speck_gloss = vec4(specularColor.xyz, rough);
}

