#ifndef _ZPP_TYPES_H_
#define _ZPP_TYPES_H_
/**@file zpp/framework/types.h
 * @brief 公共类型定义
 */
#include <zit/base/type.h>

namespace z{
    typedef uint32_t otype_t;
    typedef uint32_t oid_t;
    typedef uint32_t ostat_t;
    
    extern int DEV_STAT_MASK;// 0x00000003
    extern int DEV_STAT_FINI;// 0 // 未初始化状态
    extern int DEV_STAT_INIT;// 1 // 初始化状态
    extern int DEV_STAT_RUN;//  2 // 运行状态
    extern int DEV_STAT_EXCP;// 3 // 异常状态

#define DEV_SET_FINI(state)  (state = (state & (~z::DEV_STAT_MASK)) | z::DEV_STAT_FINI)
#define DEV_SET_INIT(state) (state = (state & (~z::DEV_STAT_MASK)) | z::DEV_STAT_INIT)
#define DEV_SET_RUN(state)  (state = (state & (~z::DEV_STAT_MASK)) | z::DEV_STAT_RUN)
#define DEV_SET_EXCP(state) (state = (state & (~z::DEV_STAT_MASK)) | z::DEV_STAT_EXCP)
  
#define DEV_IS_FINI(state) ((state & z::DEV_STAT_MASK) == z::DEV_STAT_FINI)
#define DEV_IS_INIT(state) ((state & z::DEV_STAT_MASK) == z::DEV_STAT_INIT)
#define DEV_IS_RUN(state) ((state & z::DEV_STAT_MASK) == z::DEV_STAT_RUN)
#define DEV_IS_EXCP(state) ((state & z::DEV_STAT_MASK) == z::DEV_STAT_EXCP)

#define SafeDel(ptr) do{delete (ptr); (ptr)=NULL;}while(0)
}

#endif
