#ifndef _ZPP_RWLOCK_H_
#define _ZPP_RWLOCK_H_

#include <zit/thread/rwlock.h> // 读写锁封装
#include <zit/base/time.h>

namespace z{
    // 直接锁，可能被代码死锁
    class Rdlock{
    public:
        Rdlock(zrwlock_t *lock){
            if(ZOK == zrwlock_rdlock(lock)){
                lck = lock;
            }else{
                lck = NULL;
                ZERRC(ZFUN_FAIL);
            }
        }

        ~Rdlock(){
            zrwlock_unlock(lck, 1);
        }

        zrwlock_t *lck;
    };

    class Wrlock{
    public:
        Wrlock(zrwlock_t *lock){
            if(ZOK == zrwlock_wrlock(lock)){
                lck = lock;
            }else{
                lck = NULL;
                ZERRC(ZFUN_FAIL);
            }
        }
        inline ~Wrlock(){
            zrwlock_unlock(lck, 0);
        }

        zrwlock_t *lck;
    };
    /*
    // 超时读写锁，防止死锁
    class TimedRwlock{
    public:
    TimedRwlock(zrwlock_t *lock, bool rd, int ms){
    // timed lock 不能控制时间,用trylock
    lck = NULL;
    if(rd){
	do{
    if(ZOK == zrwlock_tryrdlock(lock)){
    lck = lock;
    break;
    }
    zsleepms(1);
	}while(ms-1>0);
    }else{
	do{
    if(ZOK == zrwlock_trywrlock(lock)){
    lck = lock;
    break;
    }
    zsleepms(1);
	}while(ms-1>0);
    }
    }

    ~TimedRwlock(){
    if(lck){
	zrwlock_unlock(lck);
    }
    }

    zrwlock_t *lck;
    };
    */
}
#endif
