namespace gt
{

namespace lambdas
{

namespace detail
{

template<typename ...F>
struct multi_lambda_impl;

template<typename F0>
struct multi_lambda_impl<F0> : public F0
{
  multi_lambda_impl(F0&& f0) : F0(std::forward<F0>(f0)) {}
  using F0::operator();
};

template<typename F0, typename ...F>
struct multi_lambda_impl<F0, F...> : public F0, public multi_lambda_impl<F...>
{
  multi_lambda_impl(F0&& f0, F &&...f) : F0(std::forward<F0>(f0)), multi_lambda_impl<F...>(std::forward<F>(f)...) {}
  using F0::operator();
  using multi_lambda_impl<F...>::operator();
};

} // namespace detail

template<typename ...F>
struct multi_lambda : public F...//public detail::multi_lambda_impl<F...>
{
  ~multi_lambda() = default;
  multi_lambda() = delete;
  multi_lambda(F &&...f) : detail::multi_lambda_impl<F...>(std::forward<F>(f)...) {}
};

template<typename ...F>
auto make(F &&...f) ->decltype(auto)
{
  return multi_lambda<F...>(std::forward<F>(f)...);
}

} // namespace lambdas



} // namespace gt
