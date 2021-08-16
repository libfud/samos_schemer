#ifndef SAMOS_RESULT_HPP
#define SAMOS_RESULT_HPP

#include <variant>

namespace samos::result {

template<typename T, typename E>
class Result {
public:
    static auto ok(const T& ok) -> Result<T, E>
    {
        return Result<T, E>{std::variant<T, E>{std::in_place_index<0>, ok}};
    }

    static auto ok(T&& ok) -> Result<T, E>
    {
        return Result<T, E>{std::variant<T, E>{std::in_place_index<0>, std::move(ok)}};
    }

    static auto err(const E& error) -> Result<T, E>
    {
        return Result<T, E>{std::variant<T, E>{std::in_place_index<1>, error}};
    }

    static auto err(E&& error) -> Result<T, E>
    {
        return Result<T, E>{std::variant<T, E>{std::in_place_index<1>, std::move(error)}};
    }

    auto is_ok() -> bool
    {
        return(value.index() == 0);
    }

    auto is_err() -> bool
    {
        return(value.index() == 1);
    }

    auto get_ok() -> T
    {
        return std::get<0>(value);
    }

    auto get_err() -> E
    {
        return std::get<1>(value);
    }

private:
    explicit Result(std::variant<T, E>&& v) : value(std::move(v))
    {
    }

    std::variant<T, E> value;
};

} // namespace samos::result

#endif // SAMOS_RESULT_HPP
