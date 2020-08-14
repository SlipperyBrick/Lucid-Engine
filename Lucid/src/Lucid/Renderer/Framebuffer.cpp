#include "ldpch.h"

#include "Framebuffer.h"

#include "Lucid/Renderer/Renderer.h"

static GLenum SetFramebufferTextureType(FramebufferTextureType type)
{
	switch (type)
	{
		case FramebufferTextureType::TEXRECT:
		{
			return GL_TEXTURE_RECTANGLE;
		}
		case FramebufferTextureType::TEX2D:
		{
			return GL_TEXTURE_2D;
		}
	}

	LD_CORE_ASSERT(false, "Unknown framebuffer texture format!");

	return 0;
}

template <typename ... Args>
void Framebuffer::DrawBuffers(Args ... args)
{
	std::vector<GLenum> attachments = { (GLenum)(args)... };

	for (size_t i = 0; i < attachments.size(); i++)
	{
		attachments[i] = GL_COLOR_ATTACHMENT0 + attachments[i];
	}

	Renderer::Submit([attachments]()
	{
		glDrawBuffers(attachments.size(), attachments.data());
	});
}

// Explict instantiation of variadic template function
template void Framebuffer::DrawBuffers<int>(int);
template void Framebuffer::DrawBuffers<int, int>(int, int);
template void Framebuffer::DrawBuffers<int, int, int>(int, int, int);

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
	Ref<Framebuffer> instance = this;

	Renderer::Submit([instance]()
	{
		glDeleteFramebuffers(1, &instance->m_RendererID);
	});
}

// Specification has changed (a format has changed, resize of the framebuffer, etc) .. create everything from scratch with the new framebuffer spec
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
		// If a framebuffer exists, delete all its associated textures and the framebuffer
		if (instance->m_RendererID)
		{
			// Delete colour attachments
			for (auto& [attachmentPoint, textureID] : instance->m_ColourAttachments)
			{
				glDeleteTextures(1, &textureID);
			}

			// Delete depth attachments
			for (auto& [attachmentPoint, textureID] : instance->m_DepthAttachments)
			{
				glDeleteTextures(1, &textureID);
			}

			// Clear both maps
			instance->m_ColourAttachments.clear();
			instance->m_DepthAttachments.clear();

			glDeleteFramebuffers(1, &instance->m_RendererID);
		}

		glCreateFramebuffers(1, &instance->m_RendererID);
		glBindFramebuffer(GL_FRAMEBUFFER, instance->m_RendererID);

		for (auto& [attachmentPoint, textureSpec] : instance->m_Specification.m_AttachmentSpecs)
		{
			bool depth = textureSpec.TextureUsage == FramebufferTextureUsage::DEPTH;
			bool multisample = textureSpec.Samples > 1;

			GLenum type = SetFramebufferTextureType(instance->GetSpecification().m_AttachmentSpecs.at(attachmentPoint).TextureType);

			if (depth)
			{
				RendererID& textureID = instance->m_DepthAttachments[attachmentPoint];

				// Multisample depth texture
				if (multisample)
				{
					glCreateTextures(type, 1, &textureID);
					glBindTexture(type, textureID);

					glTexStorage2DMultisample(type, textureSpec.Samples, GL_DEPTH24_STENCIL8, instance->m_Specification.Width, instance->m_Specification.Height, GL_FALSE);

					glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, textureID, 0);
				}

				// Standard depth texture
				else
				{
					glCreateTextures(type, 1, &textureID);
					glBindTexture(type, textureID);

					glTexImage2D(type, 0, GL_DEPTH24_STENCIL8, instance->m_Specification.Width, instance->m_Specification.Height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);

					// Set depth texture minification filtering
					if (textureSpec.MinFilter == FramebufferTextureFiltering::LINEAR)
					{
						glTexParameteri(type, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
					}
					else if (textureSpec.MinFilter == FramebufferTextureFiltering::NEAREST)
					{
						glTexParameteri(type, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
					}

					// Set depth texture magnification filtering
					if (textureSpec.MagFilter == FramebufferTextureFiltering::LINEAR)
					{
						glTexParameteri(type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
					}
					else if (textureSpec.MagFilter == FramebufferTextureFiltering::NEAREST)
					{
						glTexParameteri(type, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
					}

					glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, textureID, 0);
				}
			}
			else
			{
				RendererID& textureID = instance->m_ColourAttachments[attachmentPoint];

				// Multisample texture
				if (multisample)
				{
					glCreateTextures(type, 1, &textureID);
					glBindTexture(type, textureID);

					if (textureSpec.Format == FramebufferTextureFormat::RGBA16F)
					{
						glTexImage2DMultisample(type, textureSpec.Samples, GL_RGBA16F, instance->m_Specification.Width, instance->m_Specification.Height, GL_FALSE);
					}
					else if (textureSpec.Format == FramebufferTextureFormat::RGBA8)
					{
						glTexStorage2DMultisample(type, textureSpec.Samples, GL_RGBA8, instance->m_Specification.Width, instance->m_Specification.Height, GL_FALSE);
					}
					else if (textureSpec.Format == FramebufferTextureFormat::RED8)
					{
						glTexStorage2DMultisample(type, textureSpec.Samples, GL_RED, instance->m_Specification.Width, instance->m_Specification.Height, GL_FALSE);
					}
				}

				// Standard texture
				else
				{
					glCreateTextures(type, 1, &textureID);
					glBindTexture(type, textureID);

					// Set texture format
					if (textureSpec.Format == FramebufferTextureFormat::RGBA16F)
					{
						glTexImage2D(type, 0, GL_RGBA16F, instance->m_Specification.Width, instance->m_Specification.Height, 0, GL_RGBA, GL_FLOAT, nullptr);
					}
					else if (textureSpec.Format == FramebufferTextureFormat::RGBA8)
					{
						glTexImage2D(type, 0, GL_RGBA, instance->m_Specification.Width, instance->m_Specification.Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
					}
					else if (textureSpec.Format == FramebufferTextureFormat::RED8)
					{
						glTexImage2D(type, 0, GL_RED, instance->m_Specification.Width, instance->m_Specification.Height, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
					}
					else if (textureSpec.Format == FramebufferTextureFormat::RG16F)
					{
						glTexImage2D(type, 0, GL_RG16F, instance->m_Specification.Width, instance->m_Specification.Height, 0, GL_RGBA, GL_FLOAT, nullptr);
					}
					else if (textureSpec.Format == FramebufferTextureFormat::RG16F_RECT)
					{
						glTexImage2D(type, 0, GL_RG32F, instance->m_Specification.Width, instance->m_Specification.Height, 0, GL_RGB, GL_FLOAT, nullptr);
					}
					else if (textureSpec.Format == FramebufferTextureFormat::RGB8F_RECT)
					{
						glTexImage2D(type, 0, GL_RGB, instance->m_Specification.Width, instance->m_Specification.Height, 0, GL_RGB, GL_FLOAT, nullptr);
					}
					else if (textureSpec.Format == FramebufferTextureFormat::RGBA8F_RECT)
					{
						glTexImage2D(type, 0, GL_RGBA, instance->m_Specification.Width, instance->m_Specification.Height, 0, GL_RGBA, GL_FLOAT, nullptr);
					}

					// Set texture minification filtering
					if (textureSpec.MinFilter == FramebufferTextureFiltering::LINEAR)
					{
						glTexParameteri(type, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
					}
					else if (textureSpec.MinFilter == FramebufferTextureFiltering::NEAREST)
					{
						glTexParameteri(type, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
					}

					// Set texture magnification filtering
					if (textureSpec.MagFilter == FramebufferTextureFiltering::LINEAR)
					{
						glTexParameteri(type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
					}
					else if (textureSpec.MagFilter == FramebufferTextureFiltering::NEAREST)
					{
						glTexParameteri(type, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
					}
				}

				glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachmentPoint, textureID, 0);
			}
		}

		// Set draw buffers
		if (instance->m_ColourAttachments.size() > 1)
		{
			std::vector<GLenum> attachments(instance->m_ColourAttachments.size());

			for (size_t i = 0; i < attachments.size(); i++)
			{
				attachments[i] = GL_COLOR_ATTACHMENT0 + i;
			}

			glDrawBuffers(instance->m_ColourAttachments.size(), attachments.data());
		}

		LD_CORE_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer is incomplete!");

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	});
}

void Framebuffer::Clear(float r, float g, float b, float a)
{
	Renderer::Submit([=]()
	{
		glClearColor(r, g, b, a);

		glClear(GL_COLOR_BUFFER_BIT);
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

void Framebuffer::BindColourAttachment(uint32_t attachmentIndex, uint32_t textureUnit) const
{
	RendererID colourAttachmentID = m_ColourAttachments.at(attachmentIndex);

	Renderer::Submit([=]()
	{
		glBindTextureUnit(textureUnit, colourAttachmentID);
	});
}

void Framebuffer::BindDepthAttachment(uint32_t attachmentIndex, uint32_t textureUnit) const
{
	RendererID depthAttachmentID = m_DepthAttachments.at(attachmentIndex);

	Renderer::Submit([=]()
	{
		glBindTextureUnit(textureUnit, depthAttachmentID);
	});
}