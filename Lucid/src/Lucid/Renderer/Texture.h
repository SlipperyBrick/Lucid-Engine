#pragma once

#include "Lucid/Core/Base.h"
#include "Lucid/Core/Memory.h"

enum class TextureFormat
{
	None = 0,
	RGB = 1,
	RGBA = 2,
	Float16 = 3
};

enum class TextureWrap
{
	None = 0,
	Clamp = 1,
	Repeat = 2
};

class Texture2D : public RefCounted
{

public:

	Texture2D(TextureFormat format, uint32_t width, uint32_t height, TextureWrap wrap);
	Texture2D(const std::string& path, bool srgb);
	~Texture2D();

	static Ref<Texture2D> Create(TextureFormat format, uint32_t width, uint32_t height, TextureWrap wrap = TextureWrap::Clamp);
	static Ref<Texture2D> Create(const std::string& path, bool srgb = false);

	void Bind(uint32_t slot = 0) const;

	TextureFormat GetFormat() const { return m_Format; }

	uint32_t GetWidth() const { return m_Width; }
	uint32_t GetHeight() const { return m_Height; }

	// Returns the expected number of mips based on image size
	uint32_t GetMipLevelCount() const;

	void Lock();
	void Unlock();

	void Resize(uint32_t width, uint32_t height);
	Memory GetWriteableBuffer();

	const std::string& GetPath() const { return m_FilePath; }

	bool Loaded() const { return m_Loaded; }

	RendererID GetRendererID() const { return m_RendererID; }

	bool operator==(const Texture2D& other) const
	{
		return m_RendererID == ((Texture2D&)other).m_RendererID;
	}

	static uint32_t GetBPP(TextureFormat format);
	static uint32_t CalculateMipMapCount(uint32_t width, uint32_t height);

private:

	RendererID m_RendererID;

	TextureFormat m_Format;
	TextureWrap m_Wrap = TextureWrap::Clamp;

	uint32_t m_Width;
	uint32_t m_Height;

	Memory m_ImageData;

	bool m_IsHDR = false;

	bool m_Locked = false;
	bool m_Loaded = false;

	std::string m_FilePath;
};

class TextureCube : public RefCounted
{

public:

	TextureCube(TextureFormat format, uint32_t width, uint32_t height);
	TextureCube(const std::string& path);
	~TextureCube();

	static Ref<TextureCube> Create(TextureFormat format, uint32_t width, uint32_t height);
	static Ref<TextureCube> Create(const std::string& path);

	void Bind(uint32_t slot = 0) const;

	TextureFormat GetFormat() const { return m_Format; }

	uint32_t GetWidth() const { return m_Width; }
	uint32_t GetHeight() const { return m_Height; }

	// Returns the expected number of mips based on image size
	uint32_t GetMipLevelCount() const;

	const std::string& GetPath() const { return m_FilePath; }

	RendererID GetRendererID() const { return m_RendererID; }

	bool operator==(const Texture2D& other) const
	{
		return m_RendererID == ((TextureCube&)other).m_RendererID;
	}

private:

	RendererID m_RendererID;

	TextureFormat m_Format;

	uint32_t m_Width;
	uint32_t m_Height;

	unsigned char* m_ImageData;

	std::string m_FilePath;
};