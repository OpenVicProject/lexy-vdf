#pragma once

#include <cmath>
#include <cstdint>
#include <filesystem>
#include <initializer_list>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>

namespace lexy_vdf {
	class KeyValues;

	using KeyType = std::string;
	using KeyObserverType = std::string_view;
	using ValueType = std::variant<std::monostate, std::string, std::int32_t, std::float_t, KeyValues>;

	struct string_hash {
		using is_transparent = void;
		[[nodiscard]] size_t operator()(const char* txt) const {
			return std::hash<std::string_view> {}(txt);
		}
		[[nodiscard]] size_t operator()(std::string_view txt) const {
			return std::hash<std::string_view> {}(txt);
		}
		[[nodiscard]] size_t operator()(std::string& txt) const {
			return std::hash<std::string> {}(txt);
		}
	};

	class KeyValues : public std::unordered_map<KeyType, ValueType, string_hash, std::equal_to<>> {
	public:
		using base_type = std::unordered_map<KeyType, ValueType, string_hash, std::equal_to<>>;
		using copy_pair_type = std::pair<KeyType, ValueType>;

		KeyValues(std::initializer_list<value_type> list);

		KeyValues() = default;
		KeyValues(KeyValues&&) = default;
		KeyValues(KeyValues&) = default;
		KeyValues(const KeyValues&) = default;
		KeyValues& operator=(KeyValues&) = default;
		KeyValues& operator=(const KeyValues&) = default;
		KeyValues& operator=(KeyValues&&) = default;

		static std::unique_ptr<KeyValues> from_buffer(const char* data, std::size_t size);
		static std::unique_ptr<KeyValues> from_buffer(const char* start, const char* end);
		static std::unique_ptr<KeyValues> from_string(const std::string_view string);
		static std::unique_ptr<KeyValues> from_file(std::string_view path);
		static std::unique_ptr<KeyValues> from_file(const std::filesystem::path& path);

		enum class MergeError {
			Success,
			FileMissing,
			ParseFail
		};
		MergeError MergeWith(const std::filesystem::path& p_path);

		KeyValues& AppendKeyValues(const KeyValues& p_key_values);

		std::int32_t GetInt(KeyObserverType p_key, std::int32_t p_default_value = 0) const;
		std::float_t GetFloat(KeyObserverType p_key, std::float_t p_default_value = 0) const;
		std::string_view GetString(KeyObserverType p_key, std::string_view p_default_value = "") const;
		bool GetBool(KeyObserverType p_key, bool p_default_value = false) const;
	};
}