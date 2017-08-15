#ifndef _ZPP_STATISTIC_HPP_
#define _ZPP_STATISTIC_HPP_
/**
 * @file zpp/statistic.hpp
 * @brief zit/statistic/statistic.h C++ wrapper
 */
#include <zit/base/error.h>
#include <zit/statistic/statistic.h>

/**
 * @biref statistic class
 */
//#define ZUSE_STATISTIC 1 // 0-disable statistic
namespace z{
    class Statistic{
#if ZUSE_STATISTIC
    public:
        static zstat_hots_t **hots;
        zstat_node_t node;
#endif
    public:
        static zerr_t Init(int num, int *size){
#if ZUSE_STATISTIC
            return zstatistic_init(&hots, num, size);
#endif
        }
        static zerr_t Fini(int num, int *size){
#if ZUSE_STATISTIC
            return zstatistic_fini(&hots, num, size);
#endif
        }
        static void DumpFile(int num, int *size, const char *path){
#if ZUSE_STATISTIC
            zstatistic_dump_file(hots, num, size, path);
#endif
        }

        Statistic(int sri, int sfi){
#if ZUSE_STATISTIC
            zstatistic_push_hots(hots, &node, sri, sfi);
#endif
        }
        ~Statistic(){
#if ZUSE_STATISTIC
            zstatistic_pop_hots(&node);
#endif
        }
    };
}
#endif
