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
template <typename F>
concept StringFunction = requires (F f) {
    std::regular_invocable<F>;
    {f()} -> std::convertible_to<std::string_view>;
};

template<std::size_t start_position = 0, typename BodyFunction>
requires std::regular_invocable<BodyFunction, std::string_view>
constexpr void
constexpr_for_token(StringFunction auto input_string_func, StringFunction auto delimiters_func, BodyFunction&& body_func)
{
    constexpr std::string_view input_string = input_string_func();
    constexpr std::string_view delimiters   = delimiters_func();
    constexpr std::size_t start_tok = input_string.find_first_not_of(delimiters, start_position);

    if constexpr (start_tok != std::string::npos) {
        constexpr std::size_t end_tok    = input_string.find_first_of(delimiters, start_tok);
        constexpr std::string_view token = input_string.substr(start_tok == std::string::npos ? 0 : start_tok, end_tok - start_tok);
        body_func(token);
        constexpr_for_token<end_tok>(input_string_func, delimiters_func, std::forward<BodyFunction>(body_func));
    }
}

// ----------------------------------------------------------------------------
int main()
{
    constexpr_for_token([]{return "yes,no,maybe";}, []{return ", ";}, [&](std::string_view s){
        std::cout << s << std::endl;
    });

    return 0;
}
