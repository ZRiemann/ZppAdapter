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
        static zststc_t ststc;
        zststc_node_t node;
#endif
    public:
        static zerr_t Init(int num, int *size){
#if ZUSE_STATISTIC
            return zststc_init(&ststc, num, size);
#endif
        }
        static zerr_t Fini(){
#if ZUSE_STATISTIC
            return zststc_fini(&ststc);
#endif
        }
        static void DumpOrg(const char *path){
#if ZUSE_STATISTIC
            zststc_dump_org(&ststc, path);
#endif
        }
        static void AddThreadStack(const char *thr_name){
#if ZUSE_STATISTIC
            zststc_add_thrs(&ststc, thr_name);
#endif
        }
        Statistic(int sri, int sfi){
#if ZUSE_STATISTIC
            zststc_push(&node, &ststc, sri, sfi);
#endif
        }
        ~Statistic(){
#if ZUSE_STATISTIC
            zststc_pop(&node);
#endif
        }
    };
}
#endif
