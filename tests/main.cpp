/**
 * MIT License
 *
 * Copyright (c) 2018 Z.Riemann
 * https://github.com/ZRiemann/
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the Software), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED AS IS, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM
 * , OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
/**
 * @file main.cpp
 * @brief <A brief description of what this file is.>
 * @author Z.Riemann https://github.com/ZRiemann/
 * @date 2018-04-18 Z.Riemann found
 */

/**
 * @file main.c
 * @brief <A brief description of what this file is.>
 * @author Z.Riemann
 * @date 2018-03-27 Z.Riemann found
 *
 * @par zmake tags
 * @zmake.build on;
 * @zmake.install off;
 * @zmake.link -lzsi -pthread -lstdc++;
 * @zmake.app zpptst;
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <zsi/base/error.h>
#include <zsi/base/trace.h>
#include <zsi/app/trace2console.h>
#include <zsi/app/trace2file.h>
#include <zsi/app/interactive.h>

#include "tst_object.h"
//#include "tst_libevent.h"

static void ztrace2zpp(const char *msg, int msg_len, zptr_t hint);
static void zregister_mission(zitac_t itac);

int main(int argc, char **argv){
    zitac_t itac = NULL;
    ztrace_register(ztrace2zpp, NULL);

    if(!(itac = zitac_init())){
        zerrno(ZEMEM_INSUFFICIENT);
        return ZEMEM_INSUFFICIENT;
    }
    zregister_mission(itac);
    zitac_repl(itac);
    zitac_fini(itac);

    zdbg("test zsi exit now.");
    return 0;
}

static void ztrace2zpp(const char *msg, int msg_len, zptr_t hint){
    ztrace2console(msg, msg_len, hint);
    ztrace2file(msg, msg_len, hint);
}

#define ZREG_MIS(key) zitac_reg_mission(itac, #key, strlen(#key), tu_##key, tc_##key)
static void zregister_mission(zitac_t itac){
    ZREG_MIS(object);
}
