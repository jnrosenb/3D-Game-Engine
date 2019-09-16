//////////////////////////////////////////
// HEADER								//
//										//
// Name: Jose Rosenbluth Chiu			//
//                  					//
//////////////////////////////////////////

#version 330 core
uniform sampler2D GBufferPos;
uniform sampler2D GBufferNormals;
uniform sampler2D GBufferDiffuse;
uniform sampler2D GBufferSpecGloss;

in vec2 uvs;
out vec4 frag_color;

//MAIN
void main(void) 
{
	frag_color = 0.3 * texture(GBufferDiffuse, uvs);
}