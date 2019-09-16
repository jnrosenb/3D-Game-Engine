//////////////////////////////////////////
// HEADER								//
//										//
// Name: Jose Rosenbluth Chiu			//
//                  					//
//////////////////////////////////////////

#version 330 core
     
layout(location = 0) in vec4 position;

uniform mat4 projView;
uniform mat4 model;

void main()
{
	gl_Position = projView * model * position;
};