#pragma once

#include <array>
#include <memory>
#include <span>
#include <type_traits>
#include <vector>

#include "concepts.hpp"
#include "endian.hpp"

namespace Serialisation {
	using Endian::Endian;

	// All functions take the transmission endianness, not the machine endianness.
	// The machine endianness is provided via #include <endian.h>

	template <Endian E, Byte T>
	std::vector<T> to_bytes(const Copyable auto&... values);

	template <Byte T>
	std::vector<std::span<T>> from_bytes(const T* bytes, std::size_t bytesLength,
	                                     std::size_t sizes...);

	template <Endian E, Byte T>
	void assign_from_bytes(const T* bytes, std::size_t bytesLength, Copyable auto&... values);

	template <Endian E, Byte T>
	void assign_from_bytes(std::vector<T> bytes, Copyable auto&... values);
} // namespace Serialisation
