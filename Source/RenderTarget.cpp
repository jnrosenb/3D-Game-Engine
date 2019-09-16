///HEADER STUFF

#include "RenderTarget.h"
#include <iostream>


RenderTarget::RenderTarget()
{
	//Generate the framebufferObj
	glGenFramebuffers(1, &this->fbo);

	for(int i = 0; i < MAX_COLOR_ATTACHMENTS; ++i)
		attachments[i] = GL_COLOR_ATTACHMENT0 + i;
}

RenderTarget::~RenderTarget() 
{
	if (this->usesDepth)
		glDeleteTextures(1, &depthTexture);

	glDeleteTextures(textureCount, texturesHandles);

	//Delete the handle
	glDeleteFramebuffers(1, &this->fbo);
}

void RenderTarget::initFromDescriptor(RenderTargetDescriptor const& desc) 
{
	this->target = desc.bufferTarget;
	this->width = desc.width;
	this->height = desc.height;
	this->mipmapLvl = desc.mipmapLevel;
	this->componentType = desc.componentType;

	this->textureCount = desc.colorAttachmentCount; //Normally 0 or 1
	this->usesDepth = desc.useDepth;
	this->useStencil = desc.useStencil;

	glBindFramebuffer(target, this->fbo);

	if (textureCount >= 0) 
	{
		glGenTextures(textureCount, texturesHandles);
		for (int i = 0; i < textureCount; ++i)
		{
			glBindTexture(GL_TEXTURE_2D, texturesHandles[i]);

			glTexImage2D(GL_TEXTURE_2D, mipmapLvl, desc.imageInternalFormat,
				width, height, 0, desc.imageFormat, componentType, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			//Make the attachment
			glFramebufferTexture2D(target, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, texturesHandles[i], mipmapLvl);
			glBindTexture(GL_TEXTURE_2D, 0);
		}

		glDrawBuffers(textureCount, attachments);
	}
	else 
	{
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
	}

	if (usesDepth && useStencil) 
	{
		glGenTextures(1, &depthTexture);
		glBindTexture(GL_TEXTURE_2D, depthTexture);
		glTexImage2D(GL_TEXTURE_2D, mipmapLvl, GL_DEPTH24_STENCIL8,
			width, height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		//Make the attachment
		glFramebufferTexture2D(target, GL_DEPTH24_STENCIL8, GL_TEXTURE_2D, depthTexture, mipmapLvl);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	else if (usesDepth) 
	{
		glGenTextures(1, &depthTexture);
		glBindTexture(GL_TEXTURE_2D, depthTexture);
		glTexImage2D(GL_TEXTURE_2D, mipmapLvl, GL_DEPTH_COMPONENT24,
			width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

		//Make the attachment
		glFramebufferTexture2D(target, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, mipmapLvl);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	//Unbind texture
	glBindTexture(GL_TEXTURE_2D, 0);

	//Check for completition
	glBindFramebuffer(target, this->fbo);
	if (glCheckFramebufferStatus(target) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR - Framebuffer state is not completed" << std::endl;
	glBindFramebuffer(target, 0);
}

void RenderTarget::Bind()
{
	glBindFramebuffer(this->target, this->fbo);

	GLenum status = glCheckFramebufferStatus(target);
	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "ERROR - Framebuffer state is not completed" << std::endl;
		switch (status)
		{
		case 0:
			std::cout << "ERROR - Framebuffer state is not completed" << std::endl;
			break;
		case GL_FRAMEBUFFER_UNDEFINED:
			std::cout << "ERROR - Framebuffer state is not completed" << std::endl;
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
			std::cout << "ERROR - Framebuffer state is not completed" << std::endl;
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
			std::cout << "ERROR - Framebuffer state is not completed" << std::endl;
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
			std::cout << "ERROR - Framebuffer state is not completed" << std::endl;
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
			std::cout << "ERROR - Framebuffer state is not completed" << std::endl;
			break;
		case GL_FRAMEBUFFER_UNSUPPORTED:
			std::cout << "ERROR - Framebuffer state is not completed" << std::endl;
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
			std::cout << "ERROR - Framebuffer state is not completed" << std::endl;
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
			std::cout << "ERROR - Framebuffer state is not completed" << std::endl;
			break;
		};
	}
}

int RenderTarget::getWidth() const 
{
	return this->width;
}

int RenderTarget::getHeight() const
{
	return this->height;
}

void RenderTarget::Unbind()
{
	glBindFramebuffer(this->target, 0);
}


void RenderTarget::AttachToTexture()
{
}

void RenderTarget::AttachToRenderBuffer()
{
}