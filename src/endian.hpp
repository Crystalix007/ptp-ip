#pragma once

#include "concepts.hpp"

namespace Endian {
	// Endianess tags
	struct LittleEndian;
	struct BigEndian;

	template <typename T>
	struct is_endian : public std::false_type {}; // NOLINT(readability-identifier-naming)

	template <>
	struct is_endian<LittleEndian> : public std::true_type {
	}; // NOLINT(readability-identifier-naming)

	template <>
	struct is_endian<BigEndian> : public std::true_type {}; // NOLINT(readability-identifier-naming)

	template <typename T>
	const constexpr bool is_endian_v = is_endian<T>::value; // NOLINT(readability-identifier-naming)

	template <typename T>
	concept Endian = is_endian_v<T>;

	template <Endian E, Memcopyable T>
	inline void host_to_endian(T& value);

	template <Endian E, Memcopyable T>
	inline void endian_to_host(T& value);
} // namespace Endian
