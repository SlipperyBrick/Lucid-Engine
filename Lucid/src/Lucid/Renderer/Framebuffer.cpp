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

	Ref<Framebuffer> instance = this;

	Renderer::Submit([instance]() mutable
	{
		instance->m_ColourAttachments.clear();
		instance->m_ColourAttachments.resize(instance->m_Specification.BufferCount);

		if (instance->m_RendererID)
		{
			glDeleteFramebuffers(1, &instance->m_RendererID);

			if (instance->m_Specification.BufferCount > 1)
			{
				for (int i = 0; i < instance->m_Specification.BufferCount; i++)
				{
					glDeleteTextures(1, &instance->m_ColourAttachments[i]);
				}
			}
			else
			{
				glDeleteTextures(1, &instance->m_ColourAttachments[0]);
			}

			glDeleteTextures(1, &instance->m_DepthAttachment);
		}

		glGenFramebuffers(1, &instance->m_RendererID);
		glBindFramebuffer(GL_FRAMEBUFFER, instance->m_RendererID);

		bool multisample = instance->m_Specification.Samples > 1;

		if (multisample)
		{
			for (int i = 0; i < instance->m_Specification.BufferCount; i++)
			{
				glCreateTextures(GL_TEXTURE_2D_MULTISAMPLE, 1, &instance->m_ColourAttachments[i]);
				glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, instance->m_ColourAttachments[i]);

				if (instance->m_Specification.Format == FramebufferFormat::RGBA16F)
				{
					glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, instance->m_Specification.Samples, GL_RGBA16F, instance->m_Specification.Width, instance->m_Specification.Height, GL_FALSE);
				}
				else if (instance->m_Specification.Format == FramebufferFormat::RGBA8)
				{
					glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, instance->m_Specification.Samples, GL_RGBA8, instance->m_Specification.Width, instance->m_Specification.Height, GL_FALSE);
				}

				glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, instance->m_ColourAttachments[i], 0);
			}

			glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
		}
		else
		{
			for (int i = 0; i < instance->m_Specification.BufferCount; i++)
			{
				glCreateTextures(GL_TEXTURE_2D, 1, &instance->m_ColourAttachments[i]);
				glBindTexture(GL_TEXTURE_2D, instance->m_ColourAttachments[i]);

				if (instance->m_Specification.Format == FramebufferFormat::RGBA16F)
				{
					glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, instance->m_Specification.Width, instance->m_Specification.Height, 0, GL_RGBA, GL_FLOAT, nullptr);
				}
				else if (instance->m_Specification.Format == FramebufferFormat::RGBA8)
				{
					glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, instance->m_Specification.Width, instance->m_Specification.Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
				}

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

				glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, instance->m_ColourAttachments[i], 0);
			}
		}

		if (multisample)
		{
			glCreateTextures(GL_TEXTURE_2D_MULTISAMPLE, 1, &instance->m_DepthAttachment);

			glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, instance->m_DepthAttachment);
			glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, instance->m_Specification.Samples, GL_DEPTH24_STENCIL8, instance->m_Specification.Width, instance->m_Specification.Height, GL_FALSE);

			glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
		}
		else
		{
			glCreateTextures(GL_TEXTURE_2D, 1, &instance->m_DepthAttachment);
			glBindTexture(GL_TEXTURE_2D, instance->m_DepthAttachment);

			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, instance->m_Specification.Width, instance->m_Specification.Height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);

			glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, instance->m_DepthAttachment, 0);
		}

		if (instance->m_Specification.BufferCount > 1)
		{
			std::vector<GLenum> attachments(instance->m_Specification.BufferCount);

			for (size_t i = 0; i < attachments.size(); i++)
			{
				attachments[i] = GL_COLOR_ATTACHMENT0 + i;
			}

			glDrawBuffers(instance->GetSpecification().BufferCount, attachments.data());
		}

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

void Framebuffer::BindColourAttachment(uint32_t colourAttachmentIndex, uint32_t textureUnit) const
{
	RendererID colourAttachmentID = m_ColourAttachments[colourAttachmentIndex];

	Renderer::Submit([=]()
	{
		glBindTextureUnit(textureUnit, colourAttachmentID);
	});
}

void Framebuffer::BindTexture(uint32_t slot) const
{
	RendererID colourAttachmentID = m_ColourAttachments[slot];

	Renderer::Submit([=]()
	{
		glBindTextureUnit(slot, colourAttachmentID);
	});
}