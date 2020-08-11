#pragma once

#include <glm/glm.hpp>

enum class FramebufferTextureType
{
	COLOUR = 0,
	DEPTH = 1
};

enum class FramebufferTextureFormat
{
	None = 0,
	RGBA8 = 1,
	RGBA16F = 2,
	RG16F = 3,
	RED8 = 4,
	DEPTH24STENCIL8 = 5
};

enum class FramebufferTextureWrapping
{
	NONE = 0,
	REPEAT = 1,
	MIRROR_REPEAT = 2,
	CLAMP_EDGE = 3,
	CLAMP_BORDER = 4,
	MIRROR_CLAMP_EDGE = 5
};

enum class FramebufferTextureFiltering
{
	NONE = 0,
	NEAREST = 1,
	LINEAR = 2
};

struct FramebufferTextureSpecification
{
	FramebufferTextureType TextureType = FramebufferTextureType::COLOUR;

	FramebufferTextureFormat Format = FramebufferTextureFormat::RGBA8;

	glm::vec2 Wrap{ FramebufferTextureWrapping::REPEAT, FramebufferTextureWrapping::REPEAT };
	FramebufferTextureFiltering MinFilter = FramebufferTextureFiltering::NEAREST;
	FramebufferTextureFiltering MagFilter = FramebufferTextureFiltering::LINEAR;

	uint32_t Samples = 1;
};

struct FramebufferSpecification
{
	uint32_t Width = 1280;
	uint32_t Height = 720;

	glm::vec4 ClearColour;

	std::unordered_map<uint32_t, FramebufferTextureSpecification> m_AttachmentSpecs;

	bool ScreenBufferTarget = false;

	void Attach(const FramebufferTextureSpecification& texture, uint32_t attachmentPoint)
	{
		m_AttachmentSpecs[attachmentPoint] = texture;
	}
};

class Framebuffer : public RefCounted
{

public:

	Framebuffer();
	Framebuffer(const FramebufferSpecification& spec);
	~Framebuffer();

	void Bind() const;
	void Unbind() const;

	void Resize(uint32_t width, uint32_t height, bool forceRecreate = false);

	void BindColourAttachment(uint32_t textureAttachmentIndex = 0, uint32_t textureUnit = 0) const;
	void BindDepthAttachment(uint32_t textureAttachmentIndex = 0, uint32_t textureUnit = 0) const;

	RendererID GetRendererID() const { return m_RendererID; }
	RendererID GetColourAttachmentRendererID(uint32_t attachment = 0) const { return m_ColourAttachments.at(attachment); }
	RendererID GetDepthAttachmentRendererID(uint32_t attachment = 0) const { return m_DepthAttachments.at(attachment); }

	const FramebufferSpecification& GetSpecification() const { return m_Specification; }

	static Ref<Framebuffer> Create();
	static Ref<Framebuffer> Create(const FramebufferSpecification& spec);

private:

	FramebufferSpecification m_Specification;

	std::unordered_map<uint32_t, RendererID> m_ColourAttachments;
	std::unordered_map<uint32_t, RendererID> m_DepthAttachments;

	RendererID m_RendererID = 0;
};

class FramebufferPool final
{

public:

	FramebufferPool(uint32_t maxFBs = 32);
	~FramebufferPool();

	std::weak_ptr<Framebuffer> AllocateBuffer();
	void Add(Ref<Framebuffer> framebuffer);

	std::vector<Ref<Framebuffer>>& GetAll() { return m_Pool; }
	const std::vector<Ref<Framebuffer>>& GetAll() const { return m_Pool; }

	inline static FramebufferPool* GetGlobal() { return s_Instance; }

private:

	std::vector<Ref<Framebuffer>> m_Pool;

	static FramebufferPool* s_Instance;
};