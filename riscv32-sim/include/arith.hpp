/*
 * Copyright (c) 2010-2017, The Regents of the University of California
 * (Regents).  All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Regents nor the
 *    names of its contributors may be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * IN NO EVENT SHALL REGENTS BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT,
 * SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS,
 * ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF
 * REGENTS HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED
 * HEREUNDER IS PROVIDED "AS IS". REGENTS HAS NO OBLIGATION TO PROVIDE
 * MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 */

#ifndef ARITH_HPP
#define ARITH_HPP

#include <cstdint>

inline uint32_t mulhu(uint32_t a, uint32_t b) {
  uint32_t a_0 = static_cast<uint32_t>(static_cast<uint16_t>(a));
  uint32_t a_1 = a >> 16;
  uint32_t b_0 = static_cast<uint32_t>(static_cast<uint16_t>(b));
  uint32_t b_1 = b >> 16;
  uint32_t t   = ((a_1 * b_0) + ((a_0 * b_0) >> 16));
  uint16_t y_1 = static_cast<uint16_t>(t);
  uint16_t y_2 = (static_cast<uint16_t>(t >> 16));
  t            = ((a_0 * b_1) + static_cast<uint32_t>(y_1));
  t            = (((a_1 * b_1) + static_cast<uint32_t>(y_2)) + (t >> 16));
  y_2          = static_cast<uint16_t>(t);
  uint16_t y_3 = (static_cast<uint16_t>(t >> 16));
  return (static_cast<uint32_t>(y_3) << 16) | static_cast<uint32_t>(y_2);
}

inline int32_t mulh(int32_t a, int32_t b) {
  int32_t  negate = static_cast<int32_t>(static_cast<int32_t>((a < 0)) !=
                                        static_cast<int32_t>((b < 0)));
  uint32_t res    = mulhu(static_cast<uint32_t>(a < 0 ? -a : a),
                          static_cast<uint32_t>(b < 0 ? -b : b));
  return static_cast<int32_t>(static_cast<bool>(negate)
                                  ? ~res + static_cast<uint32_t>(((a * b) == 0))
                                  : res);
}

inline int32_t mulhsu(int32_t a, uint32_t b) {
  int32_t  negate = static_cast<int32_t>(a < 0);
  uint32_t res    = mulhu(static_cast<uint32_t>(a < 0 ? -a : a), b);
  return static_cast<int32_t>(
      static_cast<bool>(negate)
          ? ~res + static_cast<uint32_t>(((static_cast<uint32_t>(a) * b) == 0))
          : res);
}

#endif
