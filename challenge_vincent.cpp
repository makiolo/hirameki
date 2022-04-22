#include <iostream>
#include <string>
#include <functional>
#include <type_traits>

class fun
{
public:
	template <typename Tuple, typename T>
	constexpr auto operator()(Tuple&& lambdas, T&& input) const
	{
	    return _unpack_tuple_a(std::forward<T>(input), std::forward<Tuple>(lambdas), std::make_index_sequence<std::tuple_size<std::remove_reference_t<Tuple>>::value>{});
	}
protected:
	template <typename T, typename Tuple, std::size_t... S>
	constexpr auto _unpack_tuple_a(T&& input, Tuple&& top, std::index_sequence<S...>) const
	{
		return _unpack_tuple_b(std::forward<T>(input), std::get<S>(std::forward<Tuple>(top))...);
	}

    template <typename T, typename... Args>
    constexpr auto _unpack_tuple_b(T&& input, Args&&... args) const
    {
        return apply_fun_rec(input, std::forward<Args>(args)...);
    }

    template <typename T, typename U>
    constexpr auto apply_fun(T&& input, U&& x) const
    {
        return x(input); // apply functor
    }

    template <typename T, typename U, typename... Args>
    constexpr auto apply_fun_rec(T&& input, U&& parm, Args&&... args) const
    {
        if constexpr (std::is_convertible_v<U, std::function<void(T&&)> >)
        {
            return apply_fun(std::forward<T>(input), std::forward<U>(parm));
        }
        else
        {
            return apply_fun_rec(std::forward<T>(input), std::forward<Args>(args)...);
        }
    }

    template <typename T, typename U>
    constexpr auto apply_fun_rec(T&& input, U&& parm) const
    {
        if constexpr (std::is_convertible_v<U, std::function<void(T&&)> >)
        {
            return apply_fun(std::forward<T>(input), std::forward<U>(parm));
        }
    }
};

int main()
{
    auto lambdas = std::make_tuple(
    [](float data) {
        std::cout << "float = " << data << std::endl;
        return 1;
    }, 
    [](std::string data) {
        std::cout << "string = " << data << std::endl;
        return "pepe";
    });

    fun foo;
    std::cout << "ret = " << foo(lambdas, 1.2f) << std::endl;
    std::cout << "ret = " << foo(lambdas, "hola") << std::endl;
    return 0;
}
