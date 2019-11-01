//////////////////////////////////////////
// HEADER								//
//										//
// Name: Jose Rosenbluth Chiu			//
//                  					//
//////////////////////////////////////////

#version 330 core
uniform sampler2D geoDepthBuffer;
uniform sampler2D GBufferDiffuse;

in vec2 uvs;
out vec4 frag_color;

//MAIN
void main(void) 
{
	float depth = texture(geoDepthBuffer, uvs).x;
	if (depth >= 1.0)
		discard;

	frag_color = vec4(0.3 * texture(GBufferDiffuse, uvs).xyz, 1);
}