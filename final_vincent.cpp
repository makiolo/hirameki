#include <iostream>
#include <vector>
#include <typeinfo>
#include <variant>
#include <string>

namespace gt {

namespace lambdas {

template<typename ... Ts>
struct make : Ts ... { 
    using Ts::operator() ...;
};
template<class... Ts> make(Ts...) -> make<Ts...>;

} }

int main(){
    auto multi = gt::lambdas::make {
        [](char) { return "char"; },
        [](const char*) { return "const char*"; },
        [](int) { return "int"; },
        [](float) { return "float"; },
        [](double) { return "double"; },
        [](unsigned int) { return "unsigned int"; },
        [](long int) { return "long int"; },
        [](long long int) { return "long long int"; },
        [](auto) { return "unknown type"; },
    };
    std::cout << multi("hola") << std::endl;
    std::cout << multi(3) << std::endl;
    std::cout << multi(3.2) << std::endl;
    std::cout << multi(3.2f) << std::endl;
}
