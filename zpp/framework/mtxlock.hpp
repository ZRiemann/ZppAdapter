#ifndef _ZMTX_LOCK_HPP_
#define _ZMTX_LOCK_HPP_
/**@file zpp/framework/mtxlock.hpp
 * @brief mutex lock
 */
#include <zit/thread/thread_def.h>

namespace z{
  class MtxLock{
    zmutex_t *mtx;
  public:
    MtxLock(zmutex_t *m){
      mtx = m;
      ZLOCK(mtx);
    }
    
    ~MtxLock(){
      ZUNLOCK(mtx);
    }
  };
  
}

#endif
