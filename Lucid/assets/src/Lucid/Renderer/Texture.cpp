#pragma once

#include "ldpch.h"

#include "Texture.h"

#include <glad/glad.h>

#include <stb_image/stb_image.h>

#include "Lucid/Renderer/Renderer.h"

static GLenum SetTextureFormat(TextureFormat format)
{
	switch (format)
	{
		case TextureFormat::RGB:
		{
			return GL_RGB;
		}
		case TextureFormat::RGBA:
		{
			return GL_RGBA;
		}
		case TextureFormat::Float16:
		{
			return GL_RGBA16F;
		}
	}

	LD_CORE_ASSERT(false, "Unknown texture format!");

	return 0;
}

Ref<Texture2D> Texture2D::Create(TextureFormat format, uint32_t width, uint32_t height, TextureWrap wrap)
{
	return CreateRef<Texture2D>(format, width, height, wrap);
}

Ref<Texture2D> Texture2D::Create(const std::string& path, bool srgb)
{
	return CreateRef<Texture2D>(path, srgb);
}

Ref<TextureCube> TextureCube::Create(TextureFormat format, uint32_t width, uint32_t height)
{
	return CreateRef<TextureCube>(format, width, height);
}

Ref<TextureCube> TextureCube::Create(const std::string& path)
{
	return CreateRef<TextureCube>(path);
}

Texture2D::Texture2D(TextureFormat format, uint32_t width, uint32_t height, TextureWrap wrap)
	: m_Format(format), m_Width(width), m_Height(height), m_Wrap(wrap)
{
	auto self = this;

	Renderer::Submit([this]()
	{
		glGenTextures(1, &m_RendererID);
		glBindTexture(GL_TEXTURE_2D, m_RendererID);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		GLenum wrap = m_Wrap == TextureWrap::Clamp ? GL_CLAMP_TO_EDGE : GL_REPEAT;

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);
		glTextureParameterf(m_RendererID, GL_TEXTURE_MAX_ANISOTROPY, RendererCapabilities::GetCapabilities().MaxAnisotropy);

		glTexImage2D(GL_TEXTURE_2D, 0, SetTextureFormat(m_Format), m_Width, m_Height, 0, SetTextureFormat(m_Format), GL_UNSIGNED_BYTE, nullptr);

		glBindTexture(GL_TEXTURE_2D, 0);
	});

	m_ImageData.Allocate(width * height * Texture2D::GetBPP(m_Format));
}

Texture2D::Texture2D(const std::string& path, bool srgb)
	: m_FilePath(path)
{
	int width;
	int height;
	int channels;

	if (stbi_is_hdr(path.c_str()))
	{
		LD_CORE_INFO("Loading HDR texture {0}, srgb={1}", path, srgb);

		m_ImageData.Data = (byte*)stbi_loadf(path.c_str(), &width, &height, &channels, 0);

		m_IsHDR = true;

		m_Format = TextureFormat::Float16;
	}
	else
	{
		LD_CORE_INFO("Loading texture {0}, srgb={1}", path, srgb);

		m_ImageData.Data = stbi_load(path.c_str(), &width, &height, &channels, srgb ? STBI_rgb : STBI_rgb_alpha);

		LD_CORE_ASSERT(m_ImageData.Data, "Could not read image!");

		m_Format = TextureFormat::RGBA;
	}

	if (!m_ImageData.Data)
	{
		return;
	}

	m_Loaded = true;

	m_Width = width;
	m_Height = height;

	Renderer::Submit([=]()
	{
		if (srgb)
		{
			glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);

			int levels = Texture2D::CalculateMipMapCount(m_Width, m_Height);

			glTextureStorage2D(m_RendererID, levels, GL_SRGB8, m_Width, m_Height);
			glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, levels > 1 ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
			glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, GL_RGB, GL_UNSIGNED_BYTE, m_ImageData.Data);
			glGenerateTextureMipmap(m_RendererID);
		}
		else
		{
			glGenTextures(1, &m_RendererID);
			glBindTexture(GL_TEXTURE_2D, m_RendererID);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

			GLenum internalFormat = SetTextureFormat(m_Format);

			// HDR equates to GL_RGB for now
			GLenum format = srgb ? GL_SRGB8 : (m_IsHDR ? GL_RGB : SetTextureFormat(m_Format));
			GLenum type = internalFormat == GL_RGBA16F ? GL_FLOAT : GL_UNSIGNED_BYTE;

			glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, m_Width, m_Height, 0, format, type, m_ImageData.Data);
			glGenerateMipmap(GL_TEXTURE_2D);

			glBindTexture(GL_TEXTURE_2D, 0);
		}

		stbi_image_free(m_ImageData.Data);
	});
}

Texture2D::~Texture2D()
{
	Renderer::Submit([this]()
	{
		glDeleteTextures(1, &m_RendererID);
	});
}

void Texture2D::Bind(uint32_t slot) const
{
	Renderer::Submit([this, slot]()
	{
		glBindTextureUnit(slot, m_RendererID);
	});
}

void Texture2D::Lock()
{
	m_Locked = true;
}

void Texture2D::Unlock()
{
	m_Locked = false;

	Renderer::Submit([this]()
	{
		glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, SetTextureFormat(m_Format), GL_UNSIGNED_BYTE, m_ImageData.Data);
	});
}

void Texture2D::Resize(uint32_t width, uint32_t height)
{
	LD_CORE_ASSERT(m_Locked, "Texture must be locked!");

	m_ImageData.Allocate(width * height * Texture2D::GetBPP(m_Format));

	m_ImageData.ZeroInitialize();
}

Memory Texture2D::GetWriteableBuffer()
{
	LD_CORE_ASSERT(m_Locked, "Texture must be locked!");

	return m_ImageData;
}

uint32_t Texture2D::GetBPP(TextureFormat format)
{
	switch (format)
	{
		case TextureFormat::RGB:
		{
			return 3;
		}
		case TextureFormat::RGBA:
		{
			return 4;
		}
	}

	return 0;
}

uint32_t Texture2D::CalculateMipMapCount(uint32_t width, uint32_t height)
{
	uint32_t levels = 1;

	while ((width | height) >> levels)
	{
		levels++;
	}

	return levels;
}

uint32_t Texture2D::GetMipLevelCount() const
{
	return Texture2D::CalculateMipMapCount(m_Width, m_Height);
}

TextureCube::TextureCube(TextureFormat format, uint32_t width, uint32_t height)
{
	m_Width = width;
	m_Height = height;
	m_Format = format;

	uint32_t levels = Texture2D::CalculateMipMapCount(width, height);

	Renderer::Submit([=]()
	{
		glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_RendererID);

		glTextureStorage2D(m_RendererID, levels, SetTextureFormat(m_Format), width, height);

		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, levels > 1 ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		// glTextureParameterf(m_RendererID, GL_TEXTURE_MAX_ANISOTROPY, 16);
	});
}

TextureCube::TextureCube(const std::string& path)
	: m_FilePath(path)
{
	int width;
	int height;
	int channels;

	stbi_set_flip_vertically_on_load(false);

	m_ImageData = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb);

	m_Width = width;
	m_Height = height;
	m_Format = TextureFormat::RGB;

	uint32_t faceWidth = m_Width / 4;
	uint32_t faceHeight = m_Height / 3;

	LD_CORE_ASSERT(faceWidth == faceHeight, "Non-square faces!");

	std::array<unsigned char*, 6 > faces;

	for (size_t i = 0; i < faces.size(); i++)
	{
		// 3 BPP
		faces[i] = new unsigned char[faceWidth * faceHeight * 3];
	}

	int faceIndex = 0;

	for (size_t i = 0; i < 4; i++)
	{
		for (size_t y = 0; y < faceHeight; y++)
		{
			size_t yOffset = y + faceHeight;

			for (size_t x = 0; x < faceWidth; x++)
			{
				size_t xOffset = x + i * faceWidth;

				faces[faceIndex][(x + y * faceWidth) * 3 + 0] = m_ImageData[(xOffset + yOffset * m_Width) * 3 + 0];
				faces[faceIndex][(x + y * faceWidth) * 3 + 1] = m_ImageData[(xOffset + yOffset * m_Width) * 3 + 1];
				faces[faceIndex][(x + y * faceWidth) * 3 + 2] = m_ImageData[(xOffset + yOffset * m_Width) * 3 + 2];
			}
		}

		faceIndex++;
	}

	for (size_t i = 0; i < 3; i++)
	{
		// Skip the middle face
		if (i == 1)
		{
			continue;
		}

		for (size_t y = 0; y < faceHeight; y++)
		{
			size_t yOffset = y + i * faceHeight;

			for (size_t x = 0; x < faceWidth; x++)
			{
				size_t xOffset = x + faceWidth;

				faces[faceIndex][(x + y * faceWidth) * 3 + 0] = m_ImageData[(xOffset + yOffset * m_Width) * 3 + 0];
				faces[faceIndex][(x + y * faceWidth) * 3 + 1] = m_ImageData[(xOffset + yOffset * m_Width) * 3 + 1];
				faces[faceIndex][(x + y * faceWidth) * 3 + 2] = m_ImageData[(xOffset + yOffset * m_Width) * 3 + 2];
			}
		}

		faceIndex++;
	}

	Renderer::Submit([=]()
	{
		glGenTextures(1, &m_RendererID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID);

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		glTextureParameterf(m_RendererID, GL_TEXTURE_MAX_ANISOTROPY, RendererCapabilities::GetCapabilities().MaxAnisotropy);

		auto format = SetTextureFormat(m_Format);

		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, format, faceWidth, faceHeight, 0, format, GL_UNSIGNED_BYTE, faces[2]);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, format, faceWidth, faceHeight, 0, format, GL_UNSIGNED_BYTE, faces[0]);

		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, format, faceWidth, faceHeight, 0, format, GL_UNSIGNED_BYTE, faces[4]);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, format, faceWidth, faceHeight, 0, format, GL_UNSIGNED_BYTE, faces[5]);

		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, format, faceWidth, faceHeight, 0, format, GL_UNSIGNED_BYTE, faces[1]);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, format, faceWidth, faceHeight, 0, format, GL_UNSIGNED_BYTE, faces[3]);

		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

		glBindTexture(GL_TEXTURE_2D, 0);

		for (size_t i = 0; i < faces.size(); i++)
		{
			delete[] faces[i];
		}

		stbi_image_free(m_ImageData);
	});
}

TextureCube::~TextureCube()
{
	auto self = this;

	Renderer::Submit([this]()
	{
		glDeleteTextures(1, &m_RendererID);
	});
}

void TextureCube::Bind(uint32_t slot) const
{
	Renderer::Submit([this, slot]()
	{
		glBindTextureUnit(slot, m_RendererID);
	});
}

uint32_t TextureCube::GetMipLevelCount() const
{
	return Texture2D::CalculateMipMapCount(m_Width, m_Height);
}