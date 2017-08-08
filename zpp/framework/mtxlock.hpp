#ifndef _ZMTX_LOCK_HPP_
#define _ZMTX_LOCK_HPP_
/**@file zpp/framework/mtxlock.hpp
 * @brief mutex lock
 */
#include <zit/thread/mutex.h>

namespace z{
    class MtxLock{
        zmtx_t *mtx;
    public:
        MtxLock(zmtx_t *m){
            mtx = m;
            zmtx_lock(mtx);
        }

        ~MtxLock(){
            zmtx_unlock(mtx);
        }
    };
}

#endif
