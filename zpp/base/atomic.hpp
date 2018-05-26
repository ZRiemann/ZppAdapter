/**
 * MIT License
 *
 * Copyright (c) 2018 Z.Riemann
 * https://github.com/ZRiemann/
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the Software), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED AS IS, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM
 * , OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#ifndef _ZPP_ATOMIC_HPP_
#define _ZPP_ATOMIC_HPP_

/**
 * @file zit atomic object wrapper
 */

#include <zsi/base/atomic.h>

#ifndef NSB_Z
#define NSB_Z namespace z{
#define NSE_Z }
#endif

NSB_Z
class Atomic{
    zatm32_t atm;
public:
    Atomic(int32_t init = 0):atm(init){
    }
    ~Atomic(){
    }
    int32_t Set(int32_t i){
        return zatm_xchg(&atm, i);
    }
    int32_t Inc(){
        return zatm_inc(&atm);
    }
    int32_t Dec(){
        return zatm_dec(&atm);
    }
    int32_t Add(int32_t i){
        return zatm_add(&atm, i);
    }
    int32_t Sub(int32_t i){
        return zatm_sub(&atm, i);
    }
};
NSE_Z
#endif
