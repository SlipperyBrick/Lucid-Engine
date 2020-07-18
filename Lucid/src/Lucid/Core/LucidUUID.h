#pragma once

#include <xhash>

#include "Lucid/Core/Base.h"

class LucidUUID
{

public:

	LucidUUID();
	LucidUUID(uint32_t uuid);
	LucidUUID(const LucidUUID& other);

	operator uint32_t () { return m_UUID; }
	operator const uint32_t() const { return m_UUID; }

private:

	uint32_t m_UUID;
};
namespace std
{
	template <>
	struct hash<LucidUUID>
	{
		std::size_t operator()(const LucidUUID& uuid) const
		{
			return hash<uint32_t>()((uint32_t)uuid);
		}
	};
}