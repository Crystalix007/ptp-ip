#pragma once
#include "serialisation.hpp"

#include "endian.tpp"

#include <cassert>
#include <concepts>
#include <cstring>
#include <iostream>
#include <string>
#include <utility>

namespace Serialisation {
	namespace Internal {
		template <Byte B>
		void from_bytes(std::vector<std::span<B>>& pointers, B* const bytes, const std::size_t size) {
			pointers.push_back({ bytes, size });
		}

		template <Byte B>
		void from_bytes(std::vector<std::span<B>>& pointers, B* const bytes, const std::size_t size,
		                const std::integral auto... sizes) {
			pointers.push_back({ bytes, size });
			Internal::from_bytes(pointers, bytes + size, sizes...);
		}

		template <std::integral T>
		std::size_t byte_count(const T& /*unused*/) {
			return sizeof(T);
		}

		template <Enum T>
		std::size_t byte_count(const T& /*unused*/) {
			return sizeof(std::underlying_type_t<T>);
		}

		template <Container T>
		std::size_t byte_count(const T& value) requires Memcopyable<typename T::value_type> {
			return value.size() * sizeof(typename T::value_type);
		}

		template <typename T>
		std::size_t byte_count(const std::basic_string<T>& value) {
			return (value.size() + 1) * sizeof(T);
		}

		template <typename... T>
		std::size_t byte_count(const T&... values) {
			return (Internal::byte_count(values) + ... + 0);
		}

		template <Endian E, Byte B, Memcopyable T>
		inline std::size_t copy_bytes(B* buffer, T value) {
			Endian::host_to_endian<E, T>(value);
			std::memcpy(buffer, &value, sizeof(T));
			return sizeof(T);
		}

		template <Endian E, Byte B, Container T>
		inline std::size_t copy_bytes(B* buffer, const T& value) {
			for (std::size_t i = 0; i < value.size(); i++) {
				buffer += copy_bytes<E, B>(buffer, value[i]);
			}

			return value.size() * sizeof(typename T::value_type);
		}

		template <Endian E, Byte B, String T>
		inline std::size_t copy_bytes(B* buffer, const T& value) {
			for (std::size_t i = 0; i < value.size(); i++) {
				buffer += copy_bytes<E, B>(buffer, value[i]);
			}

			*reinterpret_cast<typename T::value_type*>(buffer) =
			    static_cast<typename T::value_type>('\0');

			return (value.size() + 1) * sizeof(typename T::value_type);
		}

		template <Endian E, Byte B, typename T1, typename... Tn>
		inline void copy_bytes(B* buffer, const T1& value,
		                       const Tn&... values) requires(sizeof...(values) > 0) {
			const std::size_t parameterSize = Internal::copy_bytes<E, B, T1>(buffer, value);
			Internal::copy_bytes<E, B>(buffer + parameterSize, values...);
		}

		template <Endian E, Byte B, Memcopyable T>
		inline std::size_t assign_from_bytes(const B* bytes, std::size_t bytesLength, T& value) {
			assert(bytesLength >= sizeof(T));
			std::memcpy(&value, bytes, sizeof(value));
			Endian::endian_to_host<E>(value);
			return sizeof(value);
		}

		template <Endian E, Byte B, std::integral T, std::size_t size>
		inline std::size_t assign_from_bytes(const B* const bytes, std::size_t bytesLength,
		                                     std::array<T, size>& value) {
			assert(bytesLength >= value.size() * sizeof(T));
			const T* const elemBytes = reinterpret_cast<const T*>(bytes);
			const T* const elemBytesEnd = reinterpret_cast<const T*>(bytes + bytesLength);

			for (const T* elemPointer = elemBytes; elemPointer != elemBytesEnd; elemPointer++) {
				const B* const bytePointer = reinterpret_cast<const B*>(elemPointer);
				T& t = value[elemPointer - elemBytes];
				assign_from_bytes<E, B>(bytePointer, sizeof(T), t);
			}

			return size * sizeof(T);
		}

		template <Endian E, Byte B, Memcopyable T>
		inline std::size_t assign_from_bytes(const B* const bytes, std::size_t bytesLength,
		                                     std::vector<T>& value) {
			assert((bytesLength % sizeof(T)) == 0);
			const T* const elemBytes = reinterpret_cast<const T*>(bytes);
			const T* const elemBytesEnd = reinterpret_cast<const T*>(bytes + bytesLength);
			value = std::vector<T>{};
			value.reserve(elemBytesEnd - elemBytes);

			for (const T* elemPointer = elemBytes; elemPointer != elemBytesEnd; elemPointer++) {
				const B* const bytePointer = reinterpret_cast<const B*>(elemPointer);
				T t;
				assign_from_bytes<E, B>(bytePointer, sizeof(T), t);
				value.push_back(t);
			}

			return bytesLength;
		}

		template <Endian E, Byte B, std::integral T>
		inline std::size_t assign_from_bytes(const B* const bytes, std::size_t bytesLength,
		                                     std::basic_string<T>& value) {
			const T* const wchars = reinterpret_cast<const T*>(bytes);

			value = std::basic_string<T>{};

			for (const T* elemPointer = wchars; *elemPointer != static_cast<T>('\0'); elemPointer++) {
				const B* const bytePointer = reinterpret_cast<const B*>(elemPointer);
				T c;
				assign_from_bytes<E, B>(bytePointer, sizeof(c), c);
				value.push_back(c);
			}

			// Include sentinel character
			return (value.size() + 1) * sizeof(T);
		}

		template <Endian E, Byte B, typename T, typename... Tn>
		inline void
		assign_from_bytes(const B* const bytes, std::size_t bytesLength, std::vector<T>& value,
		                  Tn&... values) = delete; // std::vector must be assigned as last parameter

		template <Endian E, Byte B, typename T1, typename... Tn>
		void assign_from_bytes(const B* bytes, std::size_t bytesLength, T1& value,
		                       Tn&... values) requires(sizeof...(values) > 0) {
			std::size_t parameterSize = Internal::assign_from_bytes<E, B>(bytes, bytesLength, value);
			Internal::assign_from_bytes<E, B>(bytes + parameterSize, bytesLength - parameterSize,
			                                  values...);
		}
	} // namespace Internal

	template <Endian E, Byte B>
	std::vector<B> to_bytes(const Copyable auto&... values) {
		const std::size_t totalBytes = Internal::byte_count(values...);
		std::vector<B> bytes(totalBytes);

		Internal::copy_bytes<E, B>(bytes.data(), values...);

		return bytes;
	}

	template <Byte B>
	std::vector<std::span<B>> from_bytes(const B* const bytes, std::size_t bytesLength,
	                                     const std::integral auto... sizes) {
		assert((sizes + ... + 0) == bytesLength);
		std::vector<std::span<B>> pointers{};

		Internal::from_bytes<B>(pointers, bytes, sizes...);

		return pointers;
	}

	template <Endian E, Byte B>
	void assign_from_bytes(const B* const bytes, const std::size_t bytesLength,
	                       Copyable auto&... values) {
		Internal::assign_from_bytes<E, B>(bytes, bytesLength, values...);
	}

	template <Endian E, Byte B>
	void assign_from_bytes(const std::vector<B> bytes, Copyable auto&... values) {
		assign_from_bytes<E, B>(bytes.data(), bytes.size(), values...);
	}
} // namespace Serialisation
