//////////////////////////////////////////
// HEADER								//
//										//
// Name: Jose Rosenbluth Chiu			//
//                  					//
//////////////////////////////////////////

#version 330 core

uniform vec3 diffuseColor;

out vec4 frag_color;

//MAIN
void main(void) 
{
	frag_color = vec4(diffuseColor, 1);
}

