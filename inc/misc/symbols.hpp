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

#ifndef __SYMBOLS_HPP_
#define __SYMBOLS_HPP_

#include "misc/types.hpp"

extern "C" byte _load_address;
extern "C" byte _imap;
extern "C" byte _kernel_begin;
extern "C" byte _kload_begin;
extern "C" byte _text_begin;
extern "C" byte _text_end;
extern "C" byte _global_pointer;
extern "C" byte _rodata_begin;
extern "C" byte _driverinfo_begin;
extern "C" byte _driverinfo_end;
extern "C" byte _rodata_end;
extern "C" byte _data_begin;
extern "C" byte _data_end;
extern "C" byte _bss_begin;
extern "C" byte _bss_end;
extern "C" byte _stack_begin;
extern "C" byte _stack_end;
extern "C" byte _kernel_end;
extern "C" byte _kload_end;
extern "C" byte _kphysical_end;

#endif