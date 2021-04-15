#pragma once
#include "endian.hpp"

extern "C" {
#include <endian.h>
}

namespace Endian {
	template <Endian E>
	struct EndianImpl {
		template <Memcopyable T>
		inline static void host_to_endian(T& value) requires(sizeof(T) == 1);

		template <Memcopyable T>
		inline static void host_to_endian(T& value) requires(sizeof(T) == 2);

		template <Memcopyable T>
		inline static void host_to_endian(T& value) requires(sizeof(T) == 4);

		template <Memcopyable T>
		inline static void host_to_endian(T& value) requires(sizeof(T) == 8);

		template <Memcopyable T>
		inline static void endian_to_host(T& value) requires(sizeof(T) == 1);

		template <Memcopyable T>
		inline static void endian_to_host(T& value) requires(sizeof(T) == 2);

		template <Memcopyable T>
		inline static void endian_to_host(T& value) requires(sizeof(T) == 4);

		template <Memcopyable T>
		inline static void endian_to_host(T& value) requires(sizeof(T) == 8);
	};

	// Identity function for 1-byte values
	template <Endian E>
	template <Memcopyable T>
	inline void EndianImpl<E>::host_to_endian(T& value) requires(sizeof(T) == 1) {}

	template <>
	template <Memcopyable T>
	inline void EndianImpl<BigEndian>::host_to_endian(T& value) requires(sizeof(T) == 2) {
		value = static_cast<T>(htobe16(static_cast<std::uint16_t>(value)));
	}

	template <>
	template <Memcopyable T>
	inline void EndianImpl<BigEndian>::host_to_endian(T& value) requires(sizeof(T) == 4) {
		value = static_cast<T>(htobe32(static_cast<std::uint32_t>(value)));
	}

	template <>
	template <Memcopyable T>
	inline void EndianImpl<BigEndian>::host_to_endian(T& value) requires(sizeof(T) == 8) {
		value = static_cast<T>(htobe64(static_cast<std::uint64_t>(value)));
	}

	template <>
	template <Memcopyable T>
	inline void EndianImpl<LittleEndian>::host_to_endian(T& value) requires(sizeof(T) == 2) {
		value = static_cast<T>(htole16(static_cast<std::uint16_t>(value)));
	}

	template <>
	template <Memcopyable T>
	inline void EndianImpl<LittleEndian>::host_to_endian(T& value) requires(sizeof(T) == 4) {
		value = static_cast<T>(htole32(static_cast<std::uint32_t>(value)));
	}

	template <>
	template <Memcopyable T>
	inline void EndianImpl<LittleEndian>::host_to_endian(T& value) requires(sizeof(T) == 8) {
		value = static_cast<T>(htole64(static_cast<std::uint64_t>(value)));
	}

	// Identity function for 1-byte values
	template <Endian E>
	template <Memcopyable T>
	inline void EndianImpl<E>::endian_to_host(T& value) requires(sizeof(T) == 1) {}

	template <>
	template <Memcopyable T>
	inline void EndianImpl<BigEndian>::endian_to_host(T& value) requires(sizeof(T) == 2) {
		value = static_cast<T>(be16toh(static_cast<std::uint16_t>(value)));
	}

	template <>
	template <Memcopyable T>
	inline void EndianImpl<BigEndian>::endian_to_host(T& value) requires(sizeof(T) == 4) {
		value = static_cast<T>(be32toh(static_cast<std::uint32_t>(value)));
	}

	template <>
	template <Memcopyable T>
	inline void EndianImpl<BigEndian>::endian_to_host(T& value) requires(sizeof(T) == 8) {
		value = static_cast<T>(be64toh(static_cast<std::uint64_t>(value)));
	}

	template <>
	template <Memcopyable T>
	inline void EndianImpl<LittleEndian>::endian_to_host(T& value) requires(sizeof(T) == 2) {
		value = static_cast<T>(le16toh(static_cast<std::uint16_t>(value)));
	}

	template <>
	template <Memcopyable T>
	inline void EndianImpl<LittleEndian>::endian_to_host(T& value) requires(sizeof(T) == 4) {
		value = static_cast<T>(le32toh(static_cast<std::uint32_t>(value)));
	}

	template <>
	template <Memcopyable T>
	inline void EndianImpl<LittleEndian>::endian_to_host(T& value) requires(sizeof(T) == 8) {
		value = static_cast<T>(le64toh(static_cast<std::uint64_t>(value)));
	}

	template <Endian E, Memcopyable T>
	inline void host_to_endian(T& value) {
		EndianImpl<E>::template host_to_endian<T>(value);
	}

	template <Endian E, Memcopyable T>
	inline void endian_to_host(T& value) {
		EndianImpl<E>::template endian_to_host<T>(value);
	}
} // namespace Endian
