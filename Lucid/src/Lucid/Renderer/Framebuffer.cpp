#pragma once

#include "ldpch.h"

#include <glad/glad.h>

#include "Framebuffer.h"

#include "Lucid/Renderer/Renderer.h"

Ref<Framebuffer> Framebuffer::Create(const FramebufferSpecification& spec)
{
	Ref<Framebuffer> result = nullptr;

	result = Ref<Framebuffer>::Create(spec);

	FramebufferPool::GetGlobal()->Add(result);

	return result;
}

FramebufferPool* FramebufferPool::s_Instance = new FramebufferPool;

FramebufferPool::FramebufferPool(uint32_t maxFBs)
{
}

FramebufferPool::~FramebufferPool()
{
}

std::weak_ptr<Framebuffer> FramebufferPool::AllocateBuffer()
{
	return std::weak_ptr<Framebuffer>();
}

void FramebufferPool::Add(Ref<Framebuffer> framebuffer)
{
	m_Pool.push_back(framebuffer);
}

Framebuffer::Framebuffer(const FramebufferSpecification& spec)
	: m_Specification(spec)
{
	Resize(spec.Width, spec.Height, true);
}

Framebuffer::~Framebuffer()
{
	Renderer::Submit([this]()
	{
		glDeleteFramebuffers(1, &m_RendererID);
	});
}

void Framebuffer::Resize(uint32_t width, uint32_t height, bool forceRecreate)
{
	if (!forceRecreate && (m_Specification.Width == width && m_Specification.Height == height))
	{
		return;
	}

	m_Specification.Width = width;
	m_Specification.Height = height;

	Renderer::Submit([this]()
	{
		if (m_RendererID)
		{
			glDeleteFramebuffers(1, &m_RendererID);
			glDeleteTextures(1, &m_ColourAttachment);
			glDeleteTextures(1, &m_DepthAttachment);
		}

		glGenFramebuffers(1, &m_RendererID);
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);

		bool multisample = m_Specification.Samples > 1;

		if (multisample)
		{
			glCreateTextures(GL_TEXTURE_2D_MULTISAMPLE, 1, &m_ColourAttachment);
			glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_ColourAttachment);

			if (m_Specification.Format == FramebufferFormat::RGBA16F)
			{
				glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, m_Specification.Samples, GL_RGBA16F, m_Specification.Width, m_Specification.Height, GL_FALSE);
			}
			else if (m_Specification.Format == FramebufferFormat::RGBA8)
			{
				glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, m_Specification.Samples, GL_RGBA8, m_Specification.Width, m_Specification.Height, GL_FALSE);
			}

			glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
		}
		else
		{
			glCreateTextures(GL_TEXTURE_2D, 1, &m_ColourAttachment);
			glBindTexture(GL_TEXTURE_2D, m_ColourAttachment);

			if (m_Specification.Format == FramebufferFormat::RGBA16F)
			{
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_Specification.Width, m_Specification.Height, 0, GL_RGBA, GL_FLOAT, nullptr);
			}
			else if (m_Specification.Format == FramebufferFormat::RGBA8)
			{
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_Specification.Width, m_Specification.Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
			}

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ColourAttachment, 0);
		}

		if (multisample)
		{
			glCreateTextures(GL_TEXTURE_2D_MULTISAMPLE, 1, &m_DepthAttachment);

			glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_DepthAttachment);
			glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, m_Specification.Samples, GL_DEPTH24_STENCIL8, m_Specification.Width, m_Specification.Height, GL_FALSE);

			glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
		}
		else
		{
			glCreateTextures(GL_TEXTURE_2D, 1, &m_DepthAttachment);

			glBindTexture(GL_TEXTURE_2D, m_DepthAttachment);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, m_Specification.Width, m_Specification.Height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_DepthAttachment, 0);
		}

		if (multisample)
		{
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, m_ColourAttachment, 0);
		}
		else
		{
			glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_ColourAttachment, 0);
		}

		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, m_DepthAttachment, 0);

		LD_CORE_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer is incomplete!");

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	});
}

void Framebuffer::Bind() const
{
	Renderer::Submit([=]()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);

		glViewport(0, 0, m_Specification.Width, m_Specification.Height);
	});
}

void Framebuffer::Unbind() const
{
	Renderer::Submit([=]()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	});
}

void Framebuffer::BindTexture(uint32_t slot) const
{
	Renderer::Submit([=]()
	{
		glBindTextureUnit(slot, m_ColourAttachment);
	});
}