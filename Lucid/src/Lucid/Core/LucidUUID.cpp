#include "ldpch.h"

#include "LucidUUID.h"

#include <random>

static std::random_device s_RandomDevice;
static std::mt19937 eng(s_RandomDevice());
static std::uniform_int_distribution<uint32_t> s_UniformDistribution;

LucidUUID::LucidUUID()
	: m_UUID(s_UniformDistribution(eng))
{
}

LucidUUID::LucidUUID(uint32_t uuid)
	: m_UUID(uuid)
{
}

LucidUUID::LucidUUID(const LucidUUID& other)
	: m_UUID(other.m_UUID)
{
}