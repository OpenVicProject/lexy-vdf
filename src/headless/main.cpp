#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <ostream>
#include <string>
#include <string_view>
#include <type_traits>
#include <variant>

#include <lexy-vdf/KeyValues.hpp>
#include <lexy-vdf/Parser.hpp>

template<class... Ts>
struct overloaded : Ts... {
	using Ts::operator()...;
};
template<class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

void print_string(std::string_view name) {
	if (std::string_view::npos != name.find_first_of(" \t\n")) {
		std::cout << '"' << name << '"';
		return;
	}
	std::cout << name;
}

void print(const lexy_vdf::KeyValues& kv, int indent = 0) {
	for (const auto& node : kv) {
		std::visit(
			overloaded {
				[](std::monostate) {},
				[&node, &indent](auto&& arg) {
					std::cout << std::string(indent, '\t');
					print_string(node.first);
					std::cout << ": ";
					if constexpr (std::is_same_v<std::string, std::decay_t<decltype(arg)>>) {
						print_string(arg);
						std::cout << std::endl;
						return;
					}
					std::cout << arg << std::endl;
				},
				[&node, &indent](const lexy_vdf::KeyValues& arg) {
					std::cout << std::string(indent, '\t');
					print_string(node.first);
					std::cout << ": {" << std::endl;
					print(arg, indent + 1);
					std::cout << std::string(indent, '\t') << '}' << std::endl;
				} },
			node.second);
	}
}

int print_key_values(const std::string_view path) {
	auto parser = lexy_vdf::Parser::from_file(path);
	if (parser.has_error()) {
		return 1;
	}

	parser.parse();
	if (parser.has_error()) {
		return 2;
	}

	if (parser.has_warning()) {
		for (auto& warning : parser.get_warnings()) {
			std::cerr << "Warning: " << warning.message << std::endl;
		}
	}

	print(*parser.get_key_values());

	return EXIT_SUCCESS;
}

int main(int argc, char** argv) {
	switch (argc) {
		case 2:
			return print_key_values(argv[1]);
		default:
		default_jump:
			std::fprintf(stderr, "usage: %s <filename>\n", argv[0]);
			return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}