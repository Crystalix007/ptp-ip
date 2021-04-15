#pragma once

#include <concepts>
#include <cstdint>
#include <string>
#include <type_traits>

template <typename T>
concept String = std::derived_from<T, std::basic_string<typename T::value_type>>;

template <typename T>
concept Container = requires(T t, std::size_t i) {
	{ t.size() }
	->std::same_as<std::size_t>;
	{ t[i] }
	->std::same_as<typename T::value_type&>;
}
&&!String<T>;

template <typename T>
concept Enum = std::is_enum_v<T>&& std::is_integral_v<std::underlying_type_t<T>>;

template <typename T>
concept Memcopyable = std::integral<T> || Enum<T>;

template <typename T>
concept Copyable = Memcopyable<T> || Container<T> || String<T>;

template <typename T>
concept Byte = std::is_same_v<T, std::int8_t> || std::is_same_v<T, std::uint8_t> ||
               (sizeof(char) == 1 && (std::is_same_v<T, char> || std::is_same_v<T, unsigned char>));
