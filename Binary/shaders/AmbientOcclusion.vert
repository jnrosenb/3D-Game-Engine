//////////////////////////////////////////
// HEADER								//
//										//
// Name: Jose Rosenbluth Chiu			//
//                  					//
//////////////////////////////////////////

#version 330 core
 
 //VS attributes
layout(location = 0) in vec4 position;
layout(location = 2) in vec2 texCoords;

void main()
{
	gl_Position = position;
};