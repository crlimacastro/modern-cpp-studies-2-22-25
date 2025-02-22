#include <concepts>
#include <expected>
#include <charconv>
#include <span>
#include <ranges>
#include <string>
#include <print>
#include <vector>
#include <numeric>
#include <unordered_map>
#include <tuple>
#include <functional>
#include <thread>
#include <chrono>

template <typename t>
concept arithmetic = std::integral<t> || std::floating_point<t>;
using arithmetic_t = double_t;

template <arithmetic t>
auto parse(std::string_view str) -> std::expected<t, std::errc>
{
    t value;
    auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), value);
    if (ec == std::errc())
    {
        return value;
    }
    return std::unexpected(ec);
}

template <arithmetic t>
auto sum(std::span<t> args) -> t
{
    return std::accumulate(args.begin(), args.end(), t(0));
}

template <arithmetic t>
auto avg(std::span<t> args) -> t
{
    return sum(args) / static_cast<double_t>(args.size());
}

template <arithmetic t>
auto fib(t n) -> t
{
    return n <= 1 ? 1 : fib(n - 1) + fib(n - 2);
}

std::unordered_map<arithmetic_t, arithmetic_t> fib_memo_cache{};
auto fib_memo(arithmetic_t n) -> arithmetic_t
{
    if (fib_memo_cache.find(n) == fib_memo_cache.end())
        fib_memo_cache[n] = n <= 1 ? 1 : fib_memo(n - 1) + fib_memo(n - 2);
    return fib_memo_cache[n];
}

auto slow_func(size_t seconds) -> size_t
{
    std::this_thread::sleep_for(std::chrono::seconds(seconds));
    return seconds;
}

template <typename... Ts>
struct tuple_hash;

template <typename T, typename... Ts>
struct tuple_hash<T, Ts...> {
    auto operator()(const std::tuple<T, Ts...>& t) const -> size_t {
        size_t hash_value = std::hash<T>{}(std::get<0>(t));
        if constexpr (sizeof...(Ts) > 0) {
            hash_value ^= tuple_hash<Ts...>{}(std::tuple<Ts...>{std::get<Ts>(t)...}) + 0x9e3779b9 + (hash_value << 6) + (hash_value >> 2);
        }
        return hash_value;
    }
};

// 

/// @brief Creates a function that memoizes the return values of a function using a cache for an arbitrary number of arguments.
/// @tparam return_t The return type.
/// @tparam ...args_t The types of the individual arguments to the function.
/// @param func The function to memoize.
/// @return A functor that memoizes the return values of the function passed in.
template <typename return_t, typename... args_t>
auto memo(std::invocable<args_t...> auto&& func) {
    std::unordered_map<std::tuple<args_t...>, return_t, tuple_hash<args_t...>> cache{};
    return [func = std::forward<decltype(func)>(func), cache = std::move(cache)](args_t... args) mutable -> return_t {
        std::tuple<args_t...> key = std::make_tuple(args...);
        if (cache.find(key) == cache.end()) {
            cache[key] = func(args...);
        }
        return cache[key];
    };
}

auto test_sum(const int argc, const char **argv)
{
    const auto args = std::span(argv, argc)
        | std::ranges::views::drop(1)
        | std::ranges::views::transform([](const auto arg) { return std::string_view(arg); })
        | std::ranges::views::transform([](const auto arg) { return *parse<arithmetic_t>(arg); })
        | std::ranges::to<std::vector>();
    std::println("{}", sum(std::span<const arithmetic_t>(args)));
}

auto test_avg(const int argc, const char **argv)
{
    const auto args = std::span(argv, argc)
        | std::ranges::views::drop(1)
        | std::ranges::views::transform([](const auto arg) { return std::string_view(arg); })
        | std::ranges::views::transform([](const auto arg) { return *parse<arithmetic_t>(arg); })
        | std::ranges::to<std::vector>();
    std::println("{}", avg(std::span<const arithmetic_t>(args)));
}

auto test_fib_memo(const int argc, const char **argv)
{
    const auto arg = std::span(argv, argc)
        | std::ranges::views::drop(1)
        | std::ranges::views::transform([](const auto arg) { return std::string_view(arg); })
        | std::ranges::views::transform([](const auto arg) { return *parse<arithmetic_t>(arg); })
        | std::ranges::views::take(1);
    if (arg.size() < 1)
        return;
    std::println("{}", fib_memo(arg[0]));
}

auto test_memo(const int argc, const char **argv)
{
    const auto args = std::span(argv, argc)
        | std::ranges::views::drop(1)
        | std::ranges::views::transform([](const auto arg) { return std::string_view(arg); })
        | std::ranges::views::transform([](const auto arg) { return *parse<arithmetic_t>(arg); })
        | std::ranges::to<std::vector>();
    auto slow_func_memo = memo<size_t, size_t>(slow_func);
    // first time slow_func_memo is called it will calculate evaluate the slow operation
    slow_func_memo(args[0]);
    // subsequent calls will return the cached value
    slow_func_memo(args[0]);
    slow_func_memo(args[0]);
    slow_func_memo(args[0]);
    slow_func_memo(args[0]);
    slow_func_memo(args[0]);
    slow_func_memo(args[0]);
    std::println("{}", slow_func_memo(args[0]));
}

auto main(const int argc, const char **argv) -> int
{
    // test_sum(argc, argv);
    // test_avg(argc, argv);
    // test_fib_memo(argc, argv);
    test_memo(argc, argv);
}
