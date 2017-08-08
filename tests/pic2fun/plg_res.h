#ifndef _ZTST_PLG_RES_H_
#define _ZTST_PLG_RES_H_
/**@file tests/pic2fun/plg_res.h
 * @brief cross call object member function test
 **/
#include <zit/base/type.h>

ZC_BEGIN

ZAPI int init(ZOP_ARG);
ZAPI int fini(ZOP_ARG);
ZAPI int run(ZOP_ARG);
ZAPI int stop(ZOP_ARG);
ZAPI int operate(ZOP_ARG);
ZAPI int ability(ZOP_ARG);

ZC_END

#endif
