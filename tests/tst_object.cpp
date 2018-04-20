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
 * @file tst_object.cpp
 * @brief <A brief description of what this file is.>
 * @author Z.Riemann https://github.com/ZRiemann/
 * @date 2018-04-18 Z.Riemann found
 *
 * @zmake.app zpptst;
 */

#include <stdio.h>
#include <string.h>

#include <string>

#include <zsi/base/trace.h>
#include <zsi/app/interactive.h>
#include <zpp/object.hpp>

using namespace z;

class A{
public:
    void Aoo(){
        zerrno(ZENOT_SUPPORT);
    }
};

class B{
public:
    void Boo(){
        zerrno(ZENOT_SUPPORT);
    }
};

class Dev : public Object<Dev>, public B, public A{
public:
    zerr_t ToStringImp(std::string &info){
        char buf[128];
        sprintf(buf, "Dev<type:%x, id:%d, status:%d, ref:%d>",
                type, id, status, ref_cnt);
        info = buf;
        printf("%s\n", buf);
        return ZEOK;
    }
};
zerr_t tu_object(zop_arg){
    printf("#-------------------------------------------------------------------------------\n"
           "# object\n");
    return ZEOK;
}

zerr_t tc_object(zop_arg){
    Dev dev;
    Obj *o;
    std::string dev_info;
    o = dev.Uniform();
    dev.ToString(dev_info);
    dev.ToStringImp(dev_info);
    o->ToString(dev_info);
    dev.Aoo();
    dev.Boo();
    ztrace_org("\nCRTP limit:"
               "\n - Deep inherit is a bad mind, mast design more carefully;"
               "\n - std::atic_cast<parent*>(chile) OK;"
               "\n - An object multi inherit, suggest: C : public Object<C>, public B, public A{};\n");
    ztrace_org("   C is an Object class, B and A is NOT an Object class");
    return ZEOK;
}
