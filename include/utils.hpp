#ifndef OASIS_UTILS_H
#define OASIS_UTILS_H

namespace oasis {

// Overload pattern for using std::visit with std:variant, introducing a way to
// provide a variable number of lambdas that take different parameter types. the
// one that matches the underlying type of the std::variant will be called. there
// will be a compiler error if there is a std::variant type that is not present
// in the Overload
template< class ... Ts >
struct Overload : Ts ... {
    using Ts::operator() ...;
};
template< class... Ts > Overload(Ts...) -> Overload< Ts... >;

}; // namespace oasis

#endif // OASIS_UTILS_H
