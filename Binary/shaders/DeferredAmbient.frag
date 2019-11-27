//////////////////////////////////////////
// HEADER								//
//										//
// Name: Jose Rosenbluth Chiu			//
//                  					//
//////////////////////////////////////////

#version 330 core
uniform sampler2D geoDepthBuffer;
uniform sampler2D GBufferDiffuse;

//Ambient occlusion terms
uniform sampler2D AOTexture;
uniform int useAO;

in vec2 uvs;
out vec4 frag_color;

//MAIN
void main(void) 
{
	float depth = texture(geoDepthBuffer, uvs).x;
	if (depth >= 1.0)
		discard;

	vec3 diffuse_color = vec3(1, 1, 1) * texture(GBufferDiffuse, uvs).xyz;
	vec3 ambient = 0.5 * diffuse_color;
	if (useAO == 1)
	{
		float S = texture2D(AOTexture, uvs).r;
		ambient *= S;
	}
}