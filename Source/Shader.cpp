///HEADER STUFF

#pragma once

///Includes
#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include "Shader.h"

///#include <windows.h>
///std::string getexepath()
///{
///	char result[MAX_PATH];
///	return std::string(result, GetModuleFileName(NULL, result, MAX_PATH));
///}

///Signatures
std::string loadFile(const char *path);


//CONSTRUCTOR
Shader::Shader(char const *vertexPath, char const *fragmentPath)
{
	std::cout << "Shader Constructor - " << vertexPath << " - " << fragmentPath << std::endl;

	int value;
	char infolog[1024];
	std::string shaderString;

	// Compile vertex shader
	/*
	const char *vertex_shader_text =
	"#version 330 core\n\
     layout(location = 0) in vec4 position;\n\
     layout(location = 1) in vec4 normal;\n\
	 layout(std140) uniform viewProj\n\
	 {\n\
		mat4 ProjView;		// 0  - 64		\n\
		float abc;			// 64 - 68		\n\
		vec3  lightColor;	// 68 - 84		\n\
	 };\n\
     uniform mat4 model_matrix;\n\
     uniform mat4 normal_matrix;\n\
     uniform vec4 color;\n\
     out vec4 vcolor;\n\
     void main()\n\
	 {\n\
       //gl_Position = ProjView * model_matrix * position;\n\
       gl_Position = ProjView * position;\n\
       vcolor = vec4(1, 0, 0, 1);\n\
     }";
	//*/
	shaderString = loadFile(vertexPath);
	const char *vertex_shader_text = shaderString.c_str();
	GLuint vshader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vshader, 1, &vertex_shader_text, 0);
	glCompileShader(vshader);
	glGetShaderiv(vshader, GL_COMPILE_STATUS, &value);
	if (!value)
	{
		std::cerr << "vertex shader failed to compile.\nError described next: " << std::endl;
		glGetShaderInfoLog(vshader, 1024, 0, infolog);
		std::cerr << infolog << std::endl;
	}

	// Compile fragment shader
	/*
	const char *fragment_shader_text =
	 "#version 330 core\n\
     in vec4 vcolor;\
     out vec4 frag_color;\
	 \
     void main(void)\
	 {\
       frag_color = vcolor;\
     }";
	//*/
	shaderString = loadFile(fragmentPath);
	const char *fragment_shader_text = shaderString.c_str();
	GLuint fshader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fshader, 1, &fragment_shader_text, 0);
	glCompileShader(fshader);
	glGetShaderiv(fshader, GL_COMPILE_STATUS, &value);
	if (!value)
	{
		std::cerr << "fragment shader failed to compile.\nError described next: " << std::endl;
		glGetShaderInfoLog(fshader, 1024, 0, infolog);
		std::cerr << infolog << std::endl;
	}

	// (I.C) link shaders
	this->programId = glCreateProgram();
	glAttachShader(this->programId, fshader);
	glAttachShader(this->programId, vshader);
	glLinkProgram(this->programId);
	glGetProgramiv(this->programId, GL_LINK_STATUS, &value);
	if (!value)
	{
		std::cerr << "PROGRAM failed to link.\nError described next: " << std::endl;
		glGetProgramInfoLog(this->programId, 1024, 0, infolog);
		std::cerr << infolog << std::endl;
	}

	glDeleteShader(fshader);
	glDeleteShader(vshader);


	// TODO - Find right place for this
	//BIND UNIFORM BLOCK TO INDEX (if present)
	///this->BindUniformBlock("test_ubo", 0);	//need some sort of mapping so the '0' isnt hardcoded
}


//DESTRUCTOR
Shader::~Shader() 
{
	glDeleteProgram(this->programId);
}


GLuint Shader::GetCachedUniformLocation(std::string const& name)
{
	if (UniformLocationMap.find(name) != UniformLocationMap.end())
		return UniformLocationMap[name];

	GLuint location = glGetUniformLocation(this->programId, name.c_str());
	if (static_cast<GLint>(location) != -1) 
	{
		UniformLocationMap[name] = location;
		return location;
	}
	return -1;
}


//Return the id
GLint Shader::GetId() const
{
	return this->programId;
}


//Checks if the shader is valid
bool Shader::IsValid() const
{
	return this->programId != -1;
}


//Tell openGL to use this shader
void Shader::UseShader()
{
	glUseProgram(this->programId);
}


void Shader::UnbindShader() 
{
	glUseProgram(0);
}


// Uniform data passing functions (CHANGE LATER TO CONST FUNCTIONS)
void Shader::setFloat(const std::string &name, float value)
{
	GLuint location = GetCachedUniformLocation(name);
	glUniform1f(location, value);
}

void Shader::setVec3f(const std::string &name, float x, float y, float z)
{
	GLuint location = GetCachedUniformLocation(name);
	glUniform3f(location, x, y, z);
}

void Shader::setVec4f(const std::string &name, float x, float y, float z, float w)
{
	GLuint location = GetCachedUniformLocation(name);
	glUniform4f(location, x, y, z, w);
}

void Shader::setMat4f(const std::string &name, glm::mat4& matrix) 
{
	GLuint location = GetCachedUniformLocation(name);
	glUniformMatrix4fv(location, 1, GL_FALSE, &matrix[0][0]);
}


//Texture uniform binding
void Shader::setTexture(std::string const& name, GLuint texture, int unit)
{
	GLuint location = GetCachedUniformLocation(name);

	//Map the shader's sampler with the unit value
	glUniform1i(location, unit);
	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(GL_TEXTURE_2D, texture);
}


//Uniform block binding
void Shader::BindUniformBlock(std::string const& name, int bind_index)
{
	GLint uniformBlockLocation = glGetUniformBlockIndex(this->programId, name.c_str());

	if (uniformBlockLocation != -1)
	{
		glUniformBlockBinding(this->programId, bind_index, static_cast<GLuint>(uniformBlockLocation));
	}
	else
		std::cout << "ERROR retrieving uniform block location!" << std::endl;
}


//Global function for loading a text file. Returns it in a string
std::string loadFile(const char *path)
{
	std::string full_path = ShaderDir + path;
	std::string out, line;

	std::ifstream in(full_path);
	if (in.is_open())
	{
		getline(in, line);
		while (in)
		{
			out += line + "\n";
			getline(in, line);
		}
		in.close();
		return out;
	}
	else
	{
		std::cout << "Failed to open file!" << std::endl;
		return "";
	}
}

