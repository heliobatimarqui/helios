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

#ifndef _BIT_HPP_
#define _BIT_HPP_

#include "include/types.h"
#include "sys/mem.hpp"
#include "ulib/expected.hpp"

extern "C" size_t _popcount(size_t data);

namespace hls {

namespace __detail {
template <size_t N> struct CalculateSize {

  static constexpr size_t value =
      N < sizeof(byte) * 8
          ? 1
          : N / (sizeof(byte) * 8) + (N % (sizeof(byte) * 8) ? 1 : 0);
};

} // namespace __detail

template <size_t N> class Bit {
  static constexpr size_t s_data_size = __detail::CalculateSize<N>::value;

  byte data[s_data_size];

  size_t calculate_buffer_index(size_t n) const {
    size_t result = (N - n) / 8;

    return result;
  }

public:
  Bit() { memset(data, 0, s_data_size); }

  ~Bit() = default;

  size_t popcount() const {
    size_t accumulator = 0;

    // TODO: IMPLEMENT MORE EFFICIENT VERSION
    for (size_t i = 0; i < s_data_size; ++i) {
      accumulator += _popcount(data[i]);
    }

    return accumulator;
  }

  Expected<bool> get_bit(size_t n) const {
    if (n >= s_data_size * 8) {
      return error<bool>(Error::OUT_OF_BOUNDS);
    }

    size_t i = calculate_buffer_index(n);
    size_t j = n % 8;
    auto &b = data[i];

    bool result = (b >> j) & 0x1;

    return value(result);
  }

  void set_bit(size_t n, bool val) {
    if (n >= s_data_size * 8)
      return;

    size_t i = calculate_buffer_index(n);
    size_t j = n % 8;

    byte &b = data[i];

    byte temp = 0x1u & val;

    b = b | (temp << j);
  }

  size_t size() const { return N; }

  void flip() {
    for (size_t i = 0; i < s_data_size; ++i) {
      data[i] = ~data[i];
    }
  }
};

} // namespace hls

#endif