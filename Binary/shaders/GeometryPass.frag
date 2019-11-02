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
};


uniform sampler2D diffuseTexture;
uniform sampler2D metallicTexture;
uniform sampler2D roughnessTexture;

uniform vec4 diffuseColor;
uniform vec4 specularColor;
uniform float roughness;
uniform float metallic;


//INPUT BLOCK
in VS_OUT
{
	vec4 worldPos;
	vec4 normalIn;
	vec4 view_dir;
	vec2 texCoords;
};


//MAIN
void main(void) 
{		
	vec3 diffuse_k = texture(diffuseTexture, texCoords).xyz;
	diffuse_k += diffuseColor.rgb;

	GBuffer_pos = worldPos;
	GBuffer_normals = normalIn;
	
	float metal = max(metallic, texture(metallicTexture, texCoords).x);
	float rough = max(roughness, texture(roughnessTexture, texCoords).x);

	GBuffer_diffk = vec4(diffuse_k, metal);
	GBuffer_speck_gloss = vec4(specularColor.xyz, rough);
}

