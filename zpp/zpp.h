#ifndef _ZPP_NO_LIB_HPP_
#define _ZPP_NO_LIB_HPP_
/**@file zpp/no_lib.hpp
 * @brief make zpp not need to load lib, and can be custom-made to suit requirements.
 * @note
 *   first include needed files in main file, JUST INCLUDE ONCE;
 *   second include <zpp/no_lib.hpp> in main , and load needed 3d_party lib in makefile;
 *   give priority to  inline or template implements;
 *   actually it's a cpp file;
 * @history
 *   2017-07-18  ZRiemann found
 */
#include <zit/base/trace.h>

#ifdef _ZPP_PB2JSON_H_
#include "src/pb2json.cpp"
#endif

#endif // _ZPP_NO_LIB_HPP_
