#pragma once

#include <cmath>
#include <concepts>
#include <initializer_list>
#include <memory>
#include <optional>
#include <string>
#include <type_traits>
#include <variant>

#include <lexy-vdf/KeyValues.hpp>
#include <lexy-vdf/Parser.hpp>

#include <lexy/callback.hpp>
#include <lexy/callback/adapter.hpp>
#include <lexy/callback/bind.hpp>
#include <lexy/callback/fold.hpp>
#include <lexy/dsl.hpp>
#include <lexy/dsl/whitespace.hpp>
#include <lexy/grammar.hpp>

#include "lexy-vdf/ParseWarning.hpp"

#include "detail/Warnings.hpp"

namespace lexy_vdf::grammar {
	struct KeyValueStatement;

	enum class ConditionalType {
		Not,
		And,
		Or
	};

	struct EmplaceFile {
		std::string file;
	};

	static constexpr auto whitespace_specifier = lexy::dsl::unicode::blank / lexy::dsl::unicode::newline;
	static constexpr auto comment_specifier = LEXY_LIT("//") >> lexy::dsl::until(lexy::dsl::newline).or_eof();

	struct PlainValue {
		static constexpr auto rule = lexy::dsl::identifier(lexy::dsl::unicode::xid_start_underscore, lexy::dsl::unicode::xid_continue);
		static constexpr auto value = lexy::as_string<std::string>;
	};

	struct StringValue {
		static constexpr auto escaped_symbols = lexy::symbol_table<char> //
													.map<'"'>('"')
													.map<'\''>('\'')
													.map<'\\'>('\\')
													.map<'/'>('/')
													.map<'b'>('\b')
													.map<'f'>('\f')
													.map<'n'>('\n')
													.map<'r'>('\r')
													.map<'t'>('\t');
		static constexpr auto rule = [] {
			// Arbitrary code points that aren't control characters.
			auto c = -lexy::dsl::unicode::control;

			// Escape sequences start with a backlash.
			// They either map one of the symbols,
			// or a Unicode code point of the form uXXXX.
			auto escape = lexy::dsl::backslash_escape //
							  .symbol<escaped_symbols>();
			return lexy::dsl::quoted(c, escape);
		}();

		static constexpr auto value = lexy::as_string<std::string>;
	};

	struct FloatValue : lexy::token_production {
		static constexpr auto rule = [] {
			auto integer_part = lexy::dsl::sign + lexy::dsl::digits<>;

			auto fraction = lexy::dsl::period >> lexy::dsl::digits<>;
			auto exponent = lexy::dsl::lit_c<'e'> / lexy::dsl::lit_c<'E'> >> lexy::dsl::sign + lexy::dsl::digits<>;

			// We want either a fraction with an optional exponent, or an exponent.
			auto real_part = fraction >> lexy::dsl::if_(exponent) | exponent;
			auto real_number = lexy::dsl::token(integer_part + real_part);
			return lexy::dsl::capture(real_number);
		}();
		static constexpr auto value =
			lexy::as_string<std::string> |
			lexy::callback_with_state<std::float_t>([](Parser::State&, std::string&& str) {
				return std::stod(str);
			});
	};

	struct IntegerValue : lexy::token_production {
		static constexpr auto rule = LEXY_LIT("0x") >> lexy::dsl::integer<int, lexy::dsl::hex> | lexy::dsl::integer<int>;
		static constexpr auto value = lexy::forward<std::int32_t>;
	};

	struct IncludeStatement {
		static constexpr auto rule = (LEXY_LIT("#include") | LEXY_LIT("#base")) >> lexy::dsl::p<StringValue>;
		static constexpr auto value =
			lexy::as_string<std::string> |
			lexy::callback_with_state<EmplaceFile>(
				[](Parser::State&, auto&& include) {
					return EmplaceFile { LEXY_MOV(include) };
				});
	};

	struct ListValue {
		static constexpr auto rule = lexy::dsl::curly_bracketed.list(lexy::dsl::recurse_branch<KeyValueStatement> | lexy::dsl::p<IncludeStatement>);
		static constexpr auto value =
			lexy::fold_inplace<KeyValues>(
				std::initializer_list<KeyValues::value_type> {},
				[](Parser::State&, KeyValues& values, KeyValues::copy_pair_type kv) {
					if (kv.second.index() == 0) return;
					values.emplace(kv.first, kv.second);
				},
				[](Parser::State& state, KeyValues& values, auto file) {
					if (auto warning = warnings::merge_check(file.file, values.MergeWith(file.file)); warning)
						state.parse_warnings->push_back(warning.value());
				},
				[](KeyValues& values, KeyValues::copy_pair_type kv) {
					if (kv.second.index() == 0) return;
					values.emplace(kv.first, kv.second);
				},
				[](KeyValues& values, EmplaceFile file) {
					values.MergeWith(file.file);
				});
	};

	struct ConditionalExpression : public lexy::expression_production {
		struct ExpectedConditionalOperand {
			static constexpr auto name = "expected conditional operand";
		};

		static constexpr auto atom = [] {
			auto paren = lexy::dsl::parenthesized(lexy::dsl::recurse<ConditionalExpression>);
			auto value = lexy::dsl::no_whitespace(LEXY_LIT("$") >> lexy::dsl::p<PlainValue>);

			return paren | value | lexy::dsl::error<ExpectedConditionalOperand>;
		}();

		struct Not : lexy::dsl::prefix_op {
			static constexpr auto op = lexy::dsl::op<ConditionalType::Not>(LEXY_LIT("!"));
			using operand = lexy::dsl::atom;
		};

		struct And : lexy::dsl::infix_op_left {
			static constexpr auto op = lexy::dsl::op<ConditionalType::And>(LEXY_LIT("&&"));
			using operand = Not;
		};

		struct Or : lexy::dsl::infix_op_left {
			static constexpr auto op = lexy::dsl::op<ConditionalType::Or>(LEXY_LIT("||"));
			using operand = And;
		};

		using operation = Or;

		static constexpr auto value =
			lexy::callback_with_state<bool>(
				lexy::forward<bool>,
				[](auto) { return true; },
				[](Parser::State& state, bool val) { return val; },
				[](Parser::State& state, std::string&& val) {
					return state.conditionals.find(val) != state.conditionals.end();
				},
				[](Parser::State& state, auto&& lhs, ConditionalType&& type, auto&& rhs) {
					using left_t = std::decay_t<decltype(lhs)>;
					using right_t = std::decay_t<decltype(rhs)>;
					if constexpr (std::is_same_v<left_t, bool>) {
						if (!lhs && type == ConditionalType::And) return false;
						if (lhs && type == ConditionalType::Or) return true;
					}
					if constexpr (std::is_same_v<right_t, bool>) {
						if (!rhs && type == ConditionalType::And) return false;
						if (rhs && type == ConditionalType::Or) return true;
					}
					bool result;
					if constexpr (std::is_same_v<left_t, bool>) {
						result = lhs;
					} else {
						result = state.conditionals.find(lhs) != state.conditionals.end();
					}

					if constexpr (std::is_same_v<right_t, bool>) {
						switch (type) {
							case ConditionalType::And: return result && rhs;
							case ConditionalType::Or: return result || rhs;
							default: break;
						}
					} else {
						switch (type) {
							case ConditionalType::And: return result && (state.conditionals.find(rhs) != state.conditionals.end());
							case ConditionalType::Or: return result || (state.conditionals.find(rhs) != state.conditionals.end());
							default: break;
						}
					}
					return result;
				},
				[](auto&& lhs, ConditionalType&& type, auto&& rhs) {
					using left_t = std::decay_t<decltype(lhs)>;
					using right_t = std::decay_t<decltype(rhs)>;
					if constexpr (std::is_same_v<left_t, bool>) {
						if (!lhs && type == ConditionalType::And) return false;
						if (lhs && type == ConditionalType::Or) return true;
					}
					if constexpr (std::is_same_v<right_t, bool>) {
						if (!rhs && type == ConditionalType::And) return false;
						if (rhs && type == ConditionalType::Or) return true;
					}
					bool result;
					if constexpr (std::is_same_v<left_t, bool>) {
						result = lhs;
					} else {
						result = true;
					}

					if constexpr (std::is_same_v<right_t, bool>) {
						switch (type) {
							case ConditionalType::And: return result && rhs;
							case ConditionalType::Or: return result || rhs;
							default: break;
						}
					} else {
						switch (type) {
							case ConditionalType::And: return result && true;
							case ConditionalType::Or: return result || true;
							default: break;
						}
					}
					return result;
				},
				[](Parser::State& state, ConditionalType&&, auto&& rhs) {
					using right_t = std::decay_t<decltype(rhs)>;
					if constexpr (std::is_same_v<right_t, bool>) {
						return !rhs;
					} else {
						return state.conditionals.find(rhs) == state.conditionals.end();
					}
				},
				[](ConditionalType&&, auto&& rhs) {
					using right_t = std::decay_t<decltype(rhs)>;
					if constexpr (std::is_same_v<right_t, bool>) {
						return !rhs;
					} else {
						return false;
					}
				});
	};

	struct ConditionalAttribute {
		static constexpr auto rule = lexy::dsl::square_bracketed(lexy::dsl::p<ConditionalExpression>);
		static constexpr auto value = lexy::forward<bool>;
	};

	struct KeyExpression {
		static constexpr auto rule = lexy::dsl::p<PlainValue> | lexy::dsl::p<StringValue>;
		static constexpr auto value = lexy::forward<std::string>;
	};

	struct ValueExpression {
		static constexpr auto rule =
			lexy::dsl::p<ListValue> |
			lexy::dsl::p<FloatValue> |
			lexy::dsl::p<IntegerValue> |
			lexy::dsl::p<PlainValue> |
			lexy::dsl::p<StringValue>;
		static constexpr auto value = lexy::forward<ValueType>;
	};

	struct KeyValueStatement {
		static constexpr auto rule = lexy::dsl::p<KeyExpression> >> lexy::dsl::p<ValueExpression> + lexy::dsl::opt(lexy::dsl::p<ConditionalAttribute>);
		static constexpr auto value = lexy::callback<KeyValues::copy_pair_type>(
			[](auto&& key, auto&& value, lexy::nullopt = {}) {
				return KeyValues::copy_pair_type(LEXY_MOV(key), LEXY_MOV(value));
			},
			[](auto&& key, auto&& value, bool conditional) -> KeyValues::copy_pair_type {
				if (conditional)
					return KeyValues::copy_pair_type(LEXY_MOV(key), LEXY_MOV(value));
				return KeyValues::copy_pair_type();
			});
	};

	struct File {
		static constexpr auto whitespace = comment_specifier | whitespace_specifier;
		static constexpr auto rule = lexy::dsl::terminator(lexy::dsl::eof).list(lexy::dsl::p<IncludeStatement> | lexy::dsl::p<KeyValueStatement>);
		static constexpr auto value =
			ListValue::value >>
			lexy::callback<KeyValues*>(
				[](KeyValues&& kv) {
					KeyValues* result = new KeyValues();
					kv.swap(*result);
					return result;
				});
	};
}