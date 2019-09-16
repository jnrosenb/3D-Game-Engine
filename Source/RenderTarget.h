///HEADER STUFF

#pragma once

///INCLUDES
///#include "../External/Includes/GL/glew.h"
#include "../External/Includes/Glad_/Glad/glad.h"
#include <gl/GL.h>
#include <vector>

#define	MAX_COLOR_ATTACHMENTS		16	//OpenLG


struct RenderTargetDescriptor 
{
	int colorAttachmentCount;
	bool useDepth;
	bool useStencil;
	int width;
	int height;

	GLint imageFormat;
	GLint imageInternalFormat;
	GLint mipmapLevel;
	GLenum bufferTarget;
	GLenum componentType;

	//For quickly creating a descriptor
	RenderTargetDescriptor() : colorAttachmentCount(1),
		useDepth(1), useStencil(1), width(400), height(400), imageInternalFormat(GL_RGBA),
		imageFormat(GL_RGBA), mipmapLevel(0), bufferTarget(GL_FRAMEBUFFER), 
		componentType(GL_UNSIGNED_BYTE)
	{ }
};


class RenderTarget 
{

//PUBLIC INTERFACE
public:
	RenderTarget();
	virtual ~RenderTarget();

	virtual void initFromDescriptor(RenderTargetDescriptor const& descriptor);

	//TODO - Map to a different enum so you do not 
	//have opengl in public interface
	void Bind();
	void Unbind();

	int getWidth() const;
	int getHeight() const;

	//Attachment
	void AttachToTexture();
	void AttachToRenderBuffer();

//VARIABLEs
private:
	GLuint fbo;
	GLenum target;
	GLenum componentType;
	GLint mipmapLvl;

	int width;
	int height; 
	int textureCount;
	int usesDepth;
	int useStencil;

public: //TODO - erase this and make a public interface
	//Textures
	GLuint texturesHandles[MAX_COLOR_ATTACHMENTS];
	GLenum attachments[MAX_COLOR_ATTACHMENTS];
	GLuint depthTexture;
};