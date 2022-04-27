#include <iostream>
#include <vector>
#include <typeinfo>
#include <string>



namespace gt
{



namespace lambdas
{



namespace detail
{



template<typename ...F>
struct multi_lambda : public F...
{
multi_lambda(F &&...f) : F(std::forward<F>(f))... {}
using F::operator()...;
};




} // namespace detail



template<typename ...F>
auto make(F &&...f) ->decltype(auto)
{
return detail::multi_lambda<F...>(std::forward<F>(f)...);
}



} // namespace lambdas



} // namespace gt

int main(){
    auto multi = gt::lambdas::make (
        [](char) { return "char"; },
        [](const char*) { return "const char*"; },
        [](int) { return "int"; },
        [](float) { return "float"; },
        [](double) { return "double"; },
        [](unsigned int) { return "unsigned int"; },
        [](long int) { return "long int"; },
        [](long long int) { return "long long int"; },
        [](auto) { return "unknown type"; }
    );
    std::cout << multi("hola") << std::endl;
    std::cout << multi(3) << std::endl;
    std::cout << multi(3.2) << std::endl;
    std::cout << multi(3.2f) << std::endl;
}
