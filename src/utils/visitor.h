#pragma once

template <class... Ts>
struct visitor : Ts...
{
    using Ts::operator()...;
};

// explicit deduction guide (not needed as of C++20)
template <class... Ts>
visitor(Ts...) -> visitor<Ts...>;