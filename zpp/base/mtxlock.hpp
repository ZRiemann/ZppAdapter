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

    class Mutex{
        zmtx_t mtx;
    public:
        Mutex(){
            zmtx_init(&mtx);
        }
        ~Mutex(){
            zmtx_fini(&mtx);
        }
        void Lock(){
            zmtx_lock(&mtx);
        }
        void Unlock(){
            zmtx_unlock(&mtx);
        }
    }
    class MutexLock{
        Mutex &mtx;
    public:
        MutexLock(Mutex &_mtx)
            :mtx(_mtx){
            mtx.Lock();
        }
        ~MutexLock(){
            mtx.Unlock();
        }
    };
}

#endif
