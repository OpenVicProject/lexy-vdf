#pragma once

#include <memory>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <lexy-vdf/KeyValues.hpp>
#include <lexy-vdf/ParseWarning.hpp>
#include <lexy-vdf/detail/BasicParser.hpp>

namespace lexy_vdf {

	class Parser final : public detail::BasicParser {
	public:
		struct State {
			std::unordered_set<std::string> conditionals;
			std::vector<ParseWarning>* parse_warnings;
		};

		Parser();

		static Parser from_buffer(const char* data, std::size_t size);
		static Parser from_buffer(const char* start, const char* end);
		static Parser from_string(const std::string_view string);
		static Parser from_file(std::string_view path);
		static Parser from_file(const std::filesystem::path& path);

		constexpr Parser& load_from_buffer(const char* data, std::size_t size);
		constexpr Parser& load_from_buffer(const char* start, const char* end);
		constexpr Parser& load_from_string(std::string_view string);
		constexpr Parser& load_from_file(const char* path);
		constexpr Parser& load_from_file(const char* path, const Parser& root);
		Parser& load_from_file(const std::filesystem::path& path);

		constexpr Parser& load_from_file(const detail::Has_c_str auto& path) {
			return load_from_file(path.c_str());
		}

		bool parse();

		const KeyValues* get_key_values();
		KeyValues* release_key_values();

		const State get_parse_state() const;

		void add_condition(std::string_view conditional);
		bool remove_condition(std::string_view conditional);

		Parser(Parser&&);
		Parser& operator=(Parser&&);

		~Parser();

	private:
		class BufferHandler;
		std::unique_ptr<BufferHandler> _buffer_handler;
		std::unique_ptr<KeyValues> _key_values;
		State _parser_state;

		template<typename... Args>
		constexpr void _run_load_func(detail::LoadCallback<BufferHandler, Args...> auto func, Args... args);
	};
}