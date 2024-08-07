/*---------------------------------------------------------------------------------
MIT License

Copyright (c) 2024 Helio Nunes Santos

        Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"), to
deal in the Software without restriction, including without limitation the
rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
        copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
        copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
        AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

---------------------------------------------------------------------------------*/

#ifndef _UTILITIES_HPP_
#define _UTILITIES_HPP_

#include <type_traits>

namespace hls
{

    template <typename T>
    constexpr std::remove_reference_t<T> &&move(T &&arg)
    {
        return static_cast<std::remove_reference_t<T> &&>(arg);
    }

    template <class T>
    inline T &&forward(typename std::remove_reference<T>::type &t) noexcept
    {
        return static_cast<T &&>(t);
    }

    template <class T>
    inline T &&forward(typename std::remove_reference<T>::type &&t) noexcept
    {
        static_assert(!std::is_lvalue_reference<T>::value, "Can not forward an rvalue as an lvalue.");
        return static_cast<T &&>(t);
    }

    template <typename T>
    std::add_lvalue_reference_t<T> declval()
    {
        static_assert("Shouldn't be evaluated");
    }

} // namespace hls

#endif
