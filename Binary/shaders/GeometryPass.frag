//////////////////////////////////////////
// HEADER								//
//										//
// Name: Jose Rosenbluth Chiu			//
//                  					//
//////////////////////////////////////////

#version 330 core

//Just in case I dont have this as uniforms
vec3 diffuse_k = vec3(1, 0, 1);
vec3 specular_k = vec3(0, 1, 1);
int gloss = 1000;


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
	diffuse_k = texture(diffuseTexture, texCoords).xyz;

	GBuffer_pos = worldPos;
	GBuffer_normals = normalIn;
	GBuffer_diffk = vec4(diffuse_k, 1.0);
	GBuffer_speck_gloss = vec4(specular_k, 100.0);
}

