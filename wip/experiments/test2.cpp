// g++ --std=c++20 -Og -ggdb tokenizer.cpp -o tokenizer
#include <concepts>
#include <functional>
#include <string>
#include <string_view>
#include <cassert>
#include <iostream>
#include <utility>
using namespace std::literals;

// ----------------------------------------------------------------------------
template<std::size_t start_position = 0, typename BodyFunction>
requires std::regular_invocable<BodyFunction, std::string_view>
constexpr void
constexpr_for_token(std::string_view input_string, std::string_view delimiters, BodyFunction&& body_func)
{
    constexpr std::size_t start_tok = input_string.find_first_not_of(delimiters, start_position);

    if constexpr (start_tok != std::string::npos) {
        constexpr std::size_t end_tok    = input_string.find_first_of(delimiters, start_tok);
        constexpr std::string_view token = input_string.substr(start_tok == std::string::npos ? 0 : start_tok, end_tok - start_tok);
        body_func(token);
        constexpr_for_token<end_tok>(input_string, delimiters, std::forward<BodyFunction>(body_func));
    }
}

// ----------------------------------------------------------------------------
int main()
{
    constexpr_for_token("yes,no,maybe", ", ", [](std::string_view s){
        std::cout << s << std::endl;
    });

    return 0;
}
