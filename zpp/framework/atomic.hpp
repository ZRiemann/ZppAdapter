#ifndef _ZPP_ATOMIC_HPP_
#define _ZPP_ATOMIC_HPP_
/**
 * @file zit atomic object wrapper
 */

#include <zit/base/atomic.h>

namespace z{
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
}
#endif
