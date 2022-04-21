#include <iostream>
#include <string>
#include <functional>
#include <type_traits>

// C++17
template <class F, class... Args>
struct is_invocable
{
    template <class U>
    static auto test(U* p) -> decltype((*p)(std::declval<Args>()...), void(), std::true_type());
    template <class U>
    static auto test(...) -> decltype(std::false_type());

    static constexpr bool value = decltype(test<F>(0))::value;
};

class fun
{
public:
	template <typename T, typename... Args>
	void operator()(T&& input, Args&&... args) const
	{
	    bool executed = false;
	    std::function<void(T&&)>* candidate = nullptr;
		apply_fun_rec(executed, candidate, std::forward<Args>(args)...);
		if(executed)
		{
		    candidate->operator()(std::forward<T>(input));
		}
	}

protected:
    template <typename T, typename U>
	void apply_fun(bool& executed, std::function<void(T&&)>* candidate, U&& x) const
	{
	    if(!executed)
	    {
	        candidate = std::bind(x);
		    executed = true;
	    }
	}

	template <typename T, typename U, typename... Args>
	void apply_fun_rec(bool& executed, std::function<void(T&&)>* candidate, U&& parm, Args&&... args) const
	{
		apply_fun(executed, candidate, std::forward<U>(parm));
		apply_fun_rec(executed, candidate, std::forward<Args>(args)...);
	}

	template <typename T, typename U>
	void apply_fun_rec(bool& executed, std::function<void(T&&)>* candidate, U&& parm) const
	{
		apply_fun(executed, candidate, std::forward<U>(parm));
	}
};

int main()
{
    fun h;
    h(2, 
    [](int data){
        std::cout << data*2 << std::endl;
    },
    [](int data){
        std::cout << data*4 << std::endl;
    },
    [](int data){
        std::cout << data*8 << std::endl;
    });
	return 0;
}

