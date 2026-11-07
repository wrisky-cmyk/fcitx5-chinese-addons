/*
 * SPDX-FileCopyrightText: 2026 fcitx5-chinese-addons contributors
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 */
#ifndef _TABLE_GBCHARSET_H_
#define _TABLE_GBCHARSET_H_

#include <cstdint>
#include <string_view>

namespace fcitx {

/**
 * Check whether the CJK ideograph codepoint is within GB2312 charset.
 *
 * Codepoints that are not CJK ideographs (latin, punctuation, symbol, etc.)
 * are always considered within the charset.
 */
bool isGB2312Char(uint32_t codepoint);

/**
 * Check whether all CJK ideographs in the UTF-8 string are within GB2312
 * charset.
 */
bool isGB2312Word(std::string_view word);

} // namespace fcitx

#endif // _TABLE_GBCHARSET_H_
