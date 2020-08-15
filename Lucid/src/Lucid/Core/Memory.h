#pragma once

#include "ldpch.h"

// Struct that handles allocation, initalisation and writing/copying of memory (mainly used for allocating memory of function pointers to render command queue)
struct Memory
{
	byte* Data;
	uint32_t Size;

	Memory()
		: Data(nullptr), Size(0) {}

	Memory(byte* data, uint32_t size)
		: Data(data), Size(size) {}

	static Memory Copy(void* data, uint32_t size)
	{
		Memory buffer;
		buffer.Allocate(size);

		memcpy(buffer.Data, data, size);

		return buffer;
	}

	void Allocate(uint32_t size)
	{
		delete[] Data;
		Data = nullptr;

		if (size == 0)
		{
			return;
		}

		Data = new byte[size];
		Size = size;
	}

	void ZeroInitialize()
	{
		if (Data)
		{
			memset(Data, 0, Size);
		}
	}

	void Write(void* data, uint32_t size, uint32_t offset = 0)
	{
		LD_CORE_ASSERT(offset + size <= Size, "Buffer overflow!");
		memcpy(Data + offset, data, size);
	}

	operator bool() const
	{
		return Data;
	}

	byte& operator[](int index)
	{
		return Data[index];
	}

	byte operator[](int index) const
	{
		return Data[index];
	}

	template<typename T>
	T* As()
	{
		return (T*)Data;
	}

	inline uint32_t GetSize() const { return Size; }
};