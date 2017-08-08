#ifndef _ZSPIN_LOCK_HPP_
#define _ZSPIN_LOCK_HPP_
/**@file zpp/framework/mtxlock.hpp
 * @brief mutex lock
 */
#include <zit/thread/spin.h>

namespace z{
    class SpinLock{
        zspinlock_t *spin;
    public:
        SpinLock(zspinlock_t *_spin){
            spin = _spin;
            zspin_lock(spin);
        }

        ~SpinLock(){
            zspin_unlock(spin);
        }
    };
}

#endif
